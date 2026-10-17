/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

/****************************************************************************
** p2pmanager.cpp is part of FF2Play project
**
** This singleton provide peers manager from peer to peer network
****************************************************************************/
#include "net/p2pmanager.h"

#include <QNetworkInterface>
#include <cfloat>

#include "tools/singleton.h"

///
/// \brief CP2PManager::CP2PManager
/// \param parent
///
CP2PManager::CP2PManager(QObject *parent) : QObject(parent)
{
    m_log=CLogger::instance();
    m_analyser = Analyzer::instance();
    m_bUPNP=false;
    m_EnforceTCP=false;
    connect (m_analyser, SIGNAL(fireTouchSpeed(double)),this,SLOT(onLanding(double)));
    m_UDPServer = new UDPServer(this);
    m_TCPServer = new TCPServer(this);
    m_upnp = new CUPnPClient(this);
    connect(m_upnp, &CUPnPClient::failed, this, &CP2PManager::onUPnPFailed);
    connect(m_upnp, &CUPnPClient::externalAddressFound, this, &CP2PManager::onUPnPExternalAddress);
    connect(m_upnp, &CUPnPClient::portMapped, this, &CP2PManager::onUPnPPortMapped);
    connect(m_upnp, &CUPnPClient::portUnmapped, this, &CP2PManager::onUPnPPortUnmapped);
    connect (&m_checkAITimer,SIGNAL(timeout()),this,SLOT(onCheckAI()));
    m_checkAITimer.start(5000);
    m_LastDropRate = Now();
    QList<QHostAddress> InterfaceList = QNetworkInterface::allAddresses();
    for (const QHostAddress& IP : InterfaceList)
    {
        if ((!IP.isLoopback())&&(IP.protocol()==QAbstractSocket::IPv4Protocol))
            m_InternalIP.append(IP.toString());
        // a global IPv6 address : peers' IPv6 addresses are worth probing
        if ((IP.protocol()==QAbstractSocket::IPv6Protocol) && IP.isGlobal())
            m_HasIPv6 = true;
    }
#ifdef FFS_DEBUG
    m_log->log("P2PManager : Local LAN IP address : ",Qt::darkBlue,LEVEL_NORMAL);
    for (const QString& IP : std::as_const(m_InternalIP))
    {
        m_log->log("P2PManager : IP address = " + IP,Qt::darkBlue,LEVEL_NORMAL);
    }
#endif
}

///
/// \brief CP2PManager::createInstance
/// \return
///
CP2PManager* CP2PManager::createInstance()
{
    return new CP2PManager();
}

///
/// \brief CP2PManager::~CP2PManager
///
CP2PManager::~CP2PManager()
{
    init(false);
}

///
/// \brief CP2PManager::instance
/// \return
///
CP2PManager* CP2PManager::instance()
{
    return Singleton<CP2PManager>::instance(CP2PManager::createInstance);
}

///
/// \brief CP2PManager::onCheckAI
///
void CP2PManager::onCheckAI()
{
    int NbInvisible = 0;
    int NbVisible = 0;
    int PosLointain = 0;
    double ScoreLointain = -1;
    int PosPret = 0;
    double ScorePret=DBL_MAX;
    bool TeamMemberShip;

    for (int i=0; i<m_peers.count(); i++)
    {
        TeamMemberShip = false;
        if (((!m_Team.isEmpty()) && (m_peers[i]->m_team == m_Team)) ||
            ((!m_peers[i]->m_team.isEmpty()) && (m_peers[i]->m_team != m_peers[i]->m_callSign)))
        {
                TeamMemberShip =true;
                m_peers[i]->m_visible = false;
        }
        if (m_peers[i]->m_spawnable)
        {
            if (m_peers[i]->m_visible)
            {
                NbVisible++;
                if (m_peers[i]->m_distance > ScoreLointain)
                {
                    ScoreLointain = m_peers[i]->m_distance;
                    PosLointain = i;
                }
            }
            else if (!TeamMemberShip)
            {
                NbInvisible++;
                if (m_peers[i]->m_distance < ScorePret)
                {
                    ScorePret = m_peers[i]->m_distance;
                    PosPret = i;
                }
            }
        }
        //Update list quality signal icon
        emit(firePeerChange(m_peers[i]->CallSign,m_peers[i]->GetSignal()));

    }
    // Si on en a plus de visible que la limite alors on en choisi un pour le rendre invisible (les plus lointains)
    if ((NbVisible > m_AILimit) && (m_AILimit>0))
    {
        //On rend le plus lointain invisible
        m_peers[PosLointain]->m_visible = false;
#ifdef FFS_DEBUG
        m_log->log("P2PManager : Peer " + m_peers[PosLointain]->m_callSign + " become unvisible",Qt::darkBlue);
#endif
    }
    //Sinon , si on a des invisibles et que le nombre de visibles est inférieur à la limite
    //cela signifie qu'on a des invisibles qu'on peut afficher (les plus prêt)
    else if ((NbInvisible > 0) && ((NbVisible < m_AILimit)||(m_AILimit<1)))
    {
        m_peers[PosPret]->m_visible = true;
#ifdef FFS_DEBUG
        m_log->log("P2PManager : Peer " + m_peers[PosPret]->m_callSign + " become visible",Qt::darkBlue);
#endif
    }
}

///
/// \brief CP2PManager::init
/// \param flag
///
void CP2PManager::init(bool flag)
{
    if (flag)
    {
        m_settings.beginGroup("P2P");
        m_port = static_cast<quint16>(m_settings.value("Port",54857).toInt());
        m_UDPServer->setPort(m_port);
        m_TCPServer->setPort(m_port);
        m_UDPServer->start();
        m_TCPServer->start();
        if (m_settings.value("UPNPEnable", true).toBool())
            initUPNP(true);
        m_EnforceTCP = m_settings.value("EnforceTCP", false).toBool();
        m_Shadow = m_settings.value("ShadowEnable", false).toBool();
        m_AILimit = m_settings.value("AILimit",0).toInt();
        m_AIRadius = m_settings.value("AIRadius",10).toInt();
        m_P2PRate = m_settings.value("TxRate",200).toInt();
        m_DropRateDisplay = m_settings.value("DropRateDisplay",true).toBool();
        m_SendDropRate = m_settings.value("SendDropRate",false).toBool();
        m_BlackList = m_settings.value("BlackList").toStringList();
        m_settings.endGroup();
    }
    else
    {
        clear();
        m_UDPServer->stop();
        m_TCPServer->stop();
        initUPNP(false);
    }
}

///
/// \brief CP2PManager::initUPNP
/// \param flag
///
void CP2PManager::initUPNP(bool flag)
{
    if (flag)
    {
        if (m_bUPNP) return;
        m_upnp->addPortMappings(m_port);
    }
    else
    {
        m_upnp->removePortMappings();
        m_bUPNP = false;
    }
}

///
/// \brief CP2PManager::onUPnPFailed
/// \param pError
///
void CP2PManager::onUPnPFailed(CUPnPClient::Error pError)
{
    if (pError == CUPnPClient::Error::NoDevice)
        m_log->log("P2PManager : " + tr("No IGD UPNP Device found on the network !"),Qt::red);
    else
        m_log->log("P2PManager : " + tr("No valid Internet Gateway Device found."));
}

///
/// \brief CP2PManager::onUPnPExternalAddress
/// \param pAddress
///
void CP2PManager::onUPnPExternalAddress(QHostAddress pAddress)
{
    m_UPnPIPv4 = pAddress;
#ifdef FFS_DEBUG
    m_log->log("P2PManager : UPnP Internet IP address = " + m_UPnPIPv4.toString(),Qt::darkBlue,LEVEL_NORMAL);
#endif
    updateExternalIP();
}

///
/// \brief CP2PManager::setPublicIPv4
/// \param pAddress IPv4 address seen by the portal
///
void CP2PManager::setPublicIPv4(const QHostAddress& pAddress)
{
    m_PortalIPv4 = pAddress;
    updateExternalIP();
}

///
/// \brief CP2PManager::setPublicIPv6
/// \param pAddress IPv6 address seen by the portal
///
void CP2PManager::setPublicIPv6(const QHostAddress& pAddress)
{
    m_PublicIPv6 = pAddress;
    if (!pAddress.isNull()) m_HasIPv6 = true;
}

///
/// \brief CP2PManager::updateExternalIP
/// Public IPv4 used to detect peers on the same LAN : the one seen by the portal
/// (compared with the addresses of the Whazzup), else the one given by UPnP
///
void CP2PManager::updateExternalIP()
{
    const QHostAddress ExternalIP = m_PortalIPv4.isNull() ? m_UPnPIPv4 : m_PortalIPv4;
    if (ExternalIP.isEqual(m_ExternalIP)) return;
    m_ExternalIP = ExternalIP;
    // peers created before as remote are now known to be on our LAN : recreated at next Whazzup
    QStringList LocalPeers;
    for (CPeer* Peer : std::as_const(m_peers))
    {
        if (!Peer->m_local && !m_ExternalIP.isNull() && m_ExternalIP.isEqual(Peer->m_externalIP))
            LocalPeers.append(Peer->CallSign);
    }
    for (const QString& CallSign : std::as_const(LocalPeers)) deleteUser(CallSign);
}

///
/// \brief CP2PManager::onUPnPPortMapped
/// \param pProtocol
/// \param pErrorCode
///
void CP2PManager::onUPnPPortMapped(QString pProtocol, int pErrorCode)
{
    if (pErrorCode == 0)
    {
        m_bUPNP = true;
        return;
    }
    if (pProtocol == "UDP")
        m_log->log("P2PManager : " + tr("Adding UDP port mapping failed with code ") + QString::number(pErrorCode),Qt::magenta);
    else
        m_log->log("P2PManager : " + tr("Adding TCP port mappping failed with code ") + QString::number(pErrorCode),Qt::magenta);
}

///
/// \brief CP2PManager::onUPnPPortUnmapped
/// \param pProtocol
/// \param pErrorCode
///
void CP2PManager::onUPnPPortUnmapped(QString pProtocol, int pErrorCode)
{
    if (pErrorCode == 0) return;
    if (pProtocol == "UDP")
        m_log->log("P2PManager : " + tr("Delete UDP port mapping failed with code ") + QString::number(pErrorCode),Qt::magenta);
    else
        m_log->log("P2PManager : " + tr("Delete TCP port mapping failed with code ") + QString::number(pErrorCode),Qt::magenta);
}

///
/// \brief CP2PManager::getPort
/// \return
///
int CP2PManager::getPort()
{
    return m_port;
}

///
/// \brief CP2PManager::getLocalIPs
/// \return
///
QString CP2PManager::getLocalIPs()
{
    StringArray LocalIPs;
    LocalIPs.setValues(m_InternalIP);
    return QString::fromLatin1(ProtobufSerialize(LocalIPs).toBase64());
}

///
/// \brief CP2PManager::setCallsign
/// \param pCallSign
///
void CP2PManager::setCallsign(const QString &pCallSign)
{
    m_CallSign = pCallSign;
}

///
/// \brief CP2PManager::getCallsign
/// \return
///
QString CP2PManager::getCallsign()
{
    return m_CallSign;
}

///
/// \brief CP2PManager::sendTChat
/// \param pMessage
///
void CP2PManager::sendTChat(const QString &pMessage)
{
    emit(fireSendTChat(pMessage));
}

///
/// \brief CP2PManager::whazzup_Update
/// \param pWhazzup
///
void CP2PManager::whazzup_Update(QVector<PeerData>& pWhazzup)
{
    QString UserName = m_CallSign.replace('[','_');
    UserName = UserName.replace(']','_');
    UserName = UserName.toLower();
    m_Team="";
    for (PeerData data : pWhazzup)
    {
        bool Shadow = false;
        bool Disabled = isDisabled(data.CallSign);
        // Shadow Detection
        if (UserName == data.CallSign.toLower())
        {
            m_Team=data.Team;
#ifdef FFS_DEBUG
            if (m_Shadow) Shadow = true;
            else
#endif
                continue;
        }
        QVector<CPeer*>::iterator it;
        QHostAddress IP;
        // the peer's IPv6 is only worth probing if we have IPv6 too
        const QHostAddress IP6 = (Shadow || !m_HasIPv6) ? QHostAddress() : data.externalIP6;
        //Find any existing peer in list
        for (it = m_peers.begin();it != m_peers.end(); ++it )
        {
            //If we found existing peer in list
            if ((*it)->CallSign == data.CallSign)
            {
                break;
            }
        }
        // if iteraotr reach end of vector, that mean peer does not yet exist
        // so we create it...
        if (it==m_peers.end())
        {
            if (Shadow) IP = QHostAddress("127.0.0.1");
            else
            {
                IP = QHostAddress(data.externalIP);
                if (IP.isNull() && IP6.isNull())
                {
                    m_log->log("P2PManager : " + tr("Bad IP Address"),Qt::darkMagenta);
                }
            }
            if (((!IP.isNull()) || (!IP6.isNull())) && (data.Port>0))
            {
                m_peers.push_back(new CPeer(
                                      m_UDPServer,
                                      m_TCPServer,
                                      IP,
                                      IP6,
                                      data.Port,
                                      data.CallSign,
                                      Disabled,
                                      !IP.isNull() && m_ExternalIP.isEqual(IP),
                                      &data.localIPs,
                                      this,
                                      m_P2PRate,
                                      m_AIRadius,
                                      data.Team,
                                      m_EnforceTCP));
                connect(this, SIGNAL(fireSendTChat(QString)),m_peers.last(),SLOT(OnSendTChat(QString)));
                connect(m_peers.last(), SIGNAL(fireReceiveTChat(QString,QString)),this,SLOT(onReceiveTChat(QString,QString)));
                emit(firePeerAdd(data.CallSign));
#ifdef FFS_DEBUG
                m_log->log("P2PManager : Adding P2P user callsign = "+ data.CallSign + ", IP=" + IP.toString() + ", IPv6=" + IP6.toString() + ", Port=" + QString::number(data.Port),Qt::darkBlue,LEVEL_NORMAL);
#endif
            }
            else
            {
                m_log->log("P2PManager : " + tr("User ") + data.CallSign + " discarded", Qt::darkMagenta);
            }
        }
        // if user already exist, we update it
        else
        {
            CPeer* Peer = (*it);
            bool user_change=false;
            if (Shadow) IP = QHostAddress("127.0.0.1");
            else
            {
                IP = QHostAddress(data.externalIP);
                if (IP.isNull() && IP6.isNull())
                {
                    m_log->log("P2PManager : " + tr("Bad IP Address"),Qt::darkMagenta);
                    user_change=true;
                }
            }
            if ((!Peer->m_externalIP.isEqual(IP)) || (!Peer->m_externalIP6.isEqual(IP6)))
            {
#ifdef FFS_DEBUG
                m_log->log("P2PManager : Change IP for user " + Peer->CallSign + ", IP=" + Peer->m_externalIP.toString(),Qt::blue, LEVEL_NORMAL);
#endif
                user_change=true;
            }
            if (Peer->m_port != data.Port)
            {
#ifdef FFS_DEBUG
                m_log->log("P2PManager : Change port for user " + Peer->CallSign + ", Port=" + QString::number(data.Port) ,Qt::blue, LEVEL_NORMAL);
#endif
                user_change=true;
            }
            if (Peer->m_team != data.Team)
            {
                Peer->m_team = data.Team;
#ifdef FFS_DEBUG
                m_log->log("P2PManager : Change team for user " + Peer->CallSign + ", Team=" + data.Team ,Qt::blue, LEVEL_NORMAL);
#endif
            }
            if (user_change) deleteUser(Peer->CallSign);
        }
    }
    //Make invert research to purge disconnected peers
    for (QVector<CPeer*>::iterator it = m_peers.begin() ; it != m_peers.end();)
    {
        bool find=false;
        for (PeerData data : pWhazzup)
        {
            if (data.CallSign==(*it)->CallSign)
            {
                if ((!m_Shadow) && (data.CallSign == m_CallSign))
                {
                    find = false;
                }
                else find = true;
            }
        }
        if (!find)
        {
            deleteUser((*it)->CallSign);
        }
        else it++;
    }
}

///
/// \brief CP2PManager::deleteUser
/// \param pCallsign
///
void CP2PManager::deleteUser(const QString pCallsign)
{
    for (QVector<CPeer*>::iterator it = m_peers.begin() ; it != m_peers.end();)
    {
        if ((*it)->CallSign == pCallsign)
        {
#ifdef FFS_DEBUG
            m_log->log("P2PManager : Remove user " + (*it)->CallSign, Qt::blue, LEVEL_NORMAL);
#endif
            disconnect(this, SIGNAL(fireSendTChat(QString)),(*it),SLOT(OnSendTChat(QString)));
            disconnect((*it), SIGNAL(fireReceiveTChat(QString,QString)),this,SLOT(onReceiveTChat(QString,QString)));
            emit(firePeerRemove((*it)->CallSign));
            delete (*it);
            it=m_peers.erase(it);
        }
        else it++;
    }
}

///
/// \brief CP2PManager::isDisabled
/// \param pCallsign
/// \return
///
bool CP2PManager::isDisabled(const QString &pCallsign)
{
    return m_BlackList.contains(pCallsign);
}

void CP2PManager::setDisable(const QString &pCallsign, bool pFlag)
{
    CPeer* Peer=nullptr;
    for (CPeer* pPeer : m_peers)
    {
        if (pPeer!=nullptr)
        {
            if (pPeer->CallSign == pCallsign)
            {
                Peer=pPeer;
            }
        }
    }
    if (Peer == nullptr) return;
    bool Changed=false;
    if ((!m_BlackList.contains(pCallsign)) && pFlag)
    {
        m_BlackList.append(pCallsign);
        Changed=true;
    }
    if (m_BlackList.contains(pCallsign) && (!pFlag))
    {
        m_BlackList.removeOne(pCallsign);
        Changed=true;
    }
    if (Changed)
    {
        m_settings.beginGroup("P2P");
        m_settings.setValue("BlackList",m_BlackList);
        m_settings.endGroup();
        if (m_BlackList.contains(pCallsign))
            Peer->Stop();
        else
            Peer->Start();
    }
}

///
/// \brief CP2PManager::clear
///
void CP2PManager::clear ()
{
    for (QVector<CPeer*>::iterator it = m_peers.begin(); it != m_peers.end() ; ++it)
    {
        QString CallSign = (*it)->CallSign;
#ifdef FFS_DEBUG
        m_log->log("P2PManager : Remove user " + CallSign, Qt::blue, LEVEL_NORMAL);
#endif
        disconnect(this, SIGNAL(fireSendTChat(QString)),*it,SLOT(OnSendTChat(QString)));
        disconnect(*it, SIGNAL(fireReceiveTChat(QString,QString)),this,SLOT(onReceiveTChat(QString,QString)));
        delete (*it);
        emit(firePeerRemove(CallSign));
    }
    m_peers.clear();
}

///
/// \brief CP2PManager::onReceiveTChat
/// \param pCallSign
/// \param pMessage
///
void CP2PManager::onReceiveTChat(QString pCallSign, QString pMessage)
{
    emit (fireReceiveTChat(pCallSign,pMessage));
}

///
/// \brief CP2PManager::getTxRate
/// \return
///
int	CP2PManager::getTxRate()
{
    return m_P2PRate;
}

///
/// \brief CP2PManager::setTxRate
/// \param pTxRate
///
void CP2PManager::setTxRate(int pTxRate)
{
    if ((pTxRate<50) || (pTxRate>5000)) return;
    m_settings.setValue("P2P/TxRate",pTxRate);
    m_P2PRate = pTxRate;
    for (CPeer* peer : m_peers)
    {
        peer->SetTxRate(pTxRate);
    }
}

///
/// \brief CP2PManager::getAILimit
/// \return
///
int	CP2PManager::getAILimit()
{
    return m_AILimit;
}

///
/// \brief CP2PManager::setAILimit
/// \param pAILimit
///
void CP2PManager::setAILimit(int pAILimit)
{
    if ((pAILimit<0) || (pAILimit>30)) return;
    m_settings.setValue("P2P/AILimit",pAILimit);
    m_AILimit=pAILimit;
}

///
/// \brief CP2PManager::getAIRadius
/// \return
///
int	CP2PManager::getAIRadius()
{
    return m_AIRadius;
}

///
/// \brief CP2PManager::setAIRadius
/// \param pAIRadius
///
void CP2PManager::setAIRadius(int pAIRadius)
{
    if ((pAIRadius<0) || (pAIRadius>99)) return;
    m_settings.setValue("P2P/AIRadius",pAIRadius);
    m_AIRadius=pAIRadius;
    for (CPeer* Peer: m_peers)
    {
        Peer->SetAIRadius(m_AIRadius);
    }
}

///
/// \brief CP2PManager::getShadowMode
/// \return
///
bool CP2PManager::getShadowMode()
{
    return m_Shadow;
}

///
/// \brief CP2PManager::setShadowMode
/// \param pFlag
///
void CP2PManager::setShadowMode(bool pFlag)
{
    m_settings.setValue("P2P/ShadowEnable",pFlag);
    m_Shadow=pFlag;
}

///
/// \brief CP2PManager::getTitle
/// \param pPeerName
/// \return
///
QString CP2PManager::getTitle(const QString &pPeerName)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            return Peer->m_data.Title;
        }
    }
    return "";
}

QString CP2PManager::getModel(const QString& pPeerName)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            return Peer->m_data.Model;
        }
    }
    return "";
}

QString CP2PManager::getType(const QString& pPeerName)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            return Peer->m_data.Type;
        }
    }
    return "";
}

///
/// \brief CP2PManager::ResetAI
/// \param pPeerName
///
void CP2PManager::ResetAI(const QString &pPeerName)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            return Peer->resetAI();
        }
    }
}

///
/// \brief CP2PManager::getBlockedData
/// \param pPeerName
/// \return
///
bool CP2PManager::getBlockedData(const QString &pPeerName)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            return Peer->m_blockData;
        }
    }
    return false;
}

///
/// \brief CP2PManager::setBlockedData
/// \param pPeerName
/// \param pFlag
///
void CP2PManager::setBlockedData(const QString &pPeerName, bool pFlag)
{
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            Peer->m_blockData=pFlag;
        }
    }
}

///
/// \brief CP2PManager::getDropRateDisplay
/// \return
///
bool CP2PManager::getDropRateDisplay()
{
    return m_DropRateDisplay;
}

///
/// \brief CP2PManager::setDropRateDisplay
/// \param pFlag
///
void CP2PManager::setDropRateDisplay(bool pFlag)
{
    m_settings.setValue("P2P/DropRateDisplay",pFlag);
    m_DropRateDisplay=pFlag;
}

///
/// \brief CP2PManager::getSendDropRate
/// \return
///
bool CP2PManager::getSendDropRate()
{
    return m_SendDropRate;
}

///
/// \brief CP2PManager::setSendDropRate
/// \param pFlag
///
void CP2PManager::setSendDropRate(bool pFlag)
{
    m_settings.setValue("P2P/SendDropRate",pFlag);
    m_SendDropRate=pFlag;
}

///
/// \brief CP2PManager::onLanding
/// \param pVSpeed
///
void CP2PManager::onLanding(double pVSpeed)
{
    if (m_DropRateDisplay)
    {
        if ( duration_cast<milliseconds> (Now() - m_LastDropRate).count() < 2000) return;
        QString Message = tr("Drop rate : ") + QString::number(static_cast<int>(pVSpeed)) + tr(" ft/min");
        m_log->log(Message);
        if (m_SendDropRate)
        {
            sendTChat(Message);
        }
        m_LastDropRate = Now();
    }
}

///
/// \brief CP2PManager::getPeerData
/// \param pPeerName
/// \return The current peer State datas
///
CAircraftState CP2PManager::getPeerData (const QString &pPeerName)
{
    CAircraftState State;
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            State = Peer->m_data;
        }
    }
    return State;
}

///
/// \brief CP2PManager::getPeerDistance
/// \param pPeerName
/// \return The peer distance
///
double CP2PManager::getPeerDistance (const QString &pPeerName)
{
    double Distance = 0.0;
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            Distance = Peer->m_distance;
        }
    }
    return Distance;
}

PeerConnexionState CP2PManager::getPeerConnexionState(const QString &pPeerName)
{
    PeerConnexionState State;
    for (CPeer* Peer : m_peers)
    {
        if (Peer->CallSign == pPeerName)
        {
            State = Peer->GetConnexionState();
        }
    }
    return State;
}
