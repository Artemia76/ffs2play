/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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
** xpmanager.cpp is part of FF2Play project
**
** This is the X-Plane Connector
** It use the abastractive SimConnector
****************************************************************************/

#include "xpmanager.h"
#include "tools/singleton.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QNetworkDatagram>
#include <QApplication>

#include "tools/datagram.h"

///
/// \brief CXPManager::CXPManager
/// \param parent
///
CXPManager::CXPManager(QObject* parent):
    SimConnector(parent)
{
    m_settings.beginGroup("XPlane");
    setOptionChange(SimOption::UseAI, m_settings.value("AIMode", false).toBool());
    setOptionChange(SimOption::UseCSL, !m_settings.value("AIMode", false).toBool());
    m_settings.endGroup();
    m_XPPlugin.setAddress("127.0.0.1");
    m_LocalPort=47600;
    m_DestPort=47500;
    m_Server = new UDPServer(this);
    connect(m_Server, &UDPServer::fireIncomingData, this, &CXPManager::OnReceiveDatagram);
    connect(&m_PingTimer, &QTimer::timeout, this, &CXPManager::OnPingTimer);
    m_Server->setPort(m_LocalPort);
    m_Server->start();
    m_log.log("XPManager : " + tr("XPlane Module loaded"));
}

///
/// \brief CXPManager::~CXPManager
///
CXPManager::~CXPManager()
{
    close();
    m_Server->stop();
    m_log.log("XPManager : " + tr("XPlane Module unloaded"));
    //delete Server;
}

///
/// \brief CXPManager::open
///
void CXPManager::open ()
{
    if (!m_connected)
    {
        m_log.log(tr("Connecting to XPlane ..."));
        sendPing();
        m_LastPong=Now();
        m_PingTimer.start(5000);
    }
}

///
/// \brief CXPManager::close
///
void CXPManager::close ()
{
    // Also stops a connection attempt still waiting for the plugin answer
    m_PingTimer.stop();
    if (m_connected)
    {
        sendDisc();
        m_connected=false;
        m_log.log(tr("Disconnected from XPlane."));
    }
}


///
/// \brief CXPManager::getVersion
/// \return
///
SimConnector::SIM_VERSION CXPManager::getVersion()
{
    if (!m_connected) return SimConnector::SIM_VERSION::UNKNOWN;
    if (isCSLOnlyPlugin(m_version)) return SimConnector::SIM_VERSION::XPLANE_CSL;
    if (getOption(UseAI)) return SimConnector::SIM_VERSION::XPLANE;
    if (getOption(UseCSL)) return SimConnector::SIM_VERSION::XPLANE_CSL;
    return SimConnector::SIM_VERSION::UNKNOWN;
}

///
/// \brief CXPManager::sendScrollingText
///
void CXPManager::sendScrollingText(const QString& pMessage)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send scrolling text = " + pMessage, Qt::darkBlue,LEVEL_NORMAL);
#endif
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream << (quint8)TCHAT;
        StringToStream(stream,pMessage);
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
#ifdef FFS_DEBUG
        m_log.log("XPManager : Hexa = " + QString(Data.toHex()), Qt::darkBlue,LEVEL_NORMAL);
#endif
        m_Server->send(Output);
    }
}

///
/// \brief CXPManager::createAI
/// \param pAircraft
/// \param pTail
/// \param pData
/// \return
///
bool CXPManager::createAI(AIResol& pAircraft, const QString& pTail,const CAircraftState& pData,bool /*pNonAIObject*/)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send createAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        pData.toAirData(m_SendData);
        stream << (quint8)ADD_PEER;
        StringToStream(stream,pTail);
        ProtobufToStream(stream,pAircraft);
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);
        return true;
    }
    return false;
}

///
/// \brief CXPManager::removeAI
/// \return
///
bool CXPManager::removeAI(uint& pObjectID)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send removeAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << (quint8)DEL_PEER;
        stream << pObjectID;
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);
        pObjectID = 0;
        return true;
    }
    return false;
}

///
/// \brief CXPManager::updateAI
/// \param pObjectID
/// \param pData
/// \return
///
bool CXPManager::updateAI(uint pObjectID, const CAircraftState& pData)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send updateAI", Qt::darkBlue,LEVEL_VERBOSE);
#endif
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream << (quint8)DATA_SEND;
        stream << pObjectID;
        pData.toAirData(m_SendData);
        ProtobufToStream(stream,m_SendData);
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);
        return true;
    }
    return false;
}

///
/// \brief CXPManager::freezeAI
///
void CXPManager::freezeAI(uint /*pObjectID*/, bool /*pState*/)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send freezeAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
}

///
/// \brief CXPManager::sendPing
///
void CXPManager::sendPing()
{
#ifdef FFS_DEBUG
    m_log.log("XPManager : Send PING Option=" + QString::number(m_options), Qt::darkBlue,LEVEL_VERBOSE);
#endif
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << (quint8)PING;
    stream << (quint32)m_options;
    QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
    m_Server->send(Output);
}

///
/// \brief CXPManager::sendData
/// \param pFlag
///
void CXPManager::sendData(bool pFlag)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("XPManager : Send DATA", Qt::darkBlue,LEVEL_NORMAL);
#endif
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream << (quint8)DATA;
        stream << (quint8)pFlag;
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);
    }
}

///
/// \brief CXPManager::sendDisc
///
void CXPManager::sendDisc()
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream << (quint8)DISC;
    QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
    m_Server->send(Output);
}

void CXPManager::sendCallSignColor()
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream << (quint8)CALLSIGN_COLOR;
    stream << (quint8)m_CallSignColor.red();
    stream << (quint8)m_CallSignColor.green();
    stream << (quint8)m_CallSignColor.blue();
    QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
    m_Server->send(Output);
}

///
/// \brief CXPManager::setAIPaths
/// The folders are known after the connection (AIMapping) and change with the P2P settings:
/// the plugin gets the whole list each time (docs/contracts/xplane-link.md in ffs2-suite)
/// \param pPaths CSL packages folders
///
void CXPManager::setAIPaths(const QStringList& pPaths)
{
    SimConnector::setAIPaths(pPaths);
    if (getVersion() == SIM_VERSION::XPLANE_CSL) sendAIPaths();
}

///
void CXPManager::sendAIPaths()
{
    if (m_connected)
    {
        StringArray Paths;
        Paths.setValues(m_AIPaths);
        QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream << static_cast<quint8>(CSL_PATH);
        ProtobufToStream(stream,Paths);
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);
    }
}

///
/// \brief CXPManager::OnReceiveDatagram
///
void CXPManager::OnReceiveDatagram(QNetworkDatagram datagram)
{
    QByteArray data(datagram.data());
    QDataStream stream(&data,QIODevice::ReadOnly);
    quint8 Code;
    stream >> Code;
    switch ((FFS2Proto)Code)
    {
        case PONG:
        {
            if (!m_connected)
            {
                m_connected=true;
                m_PingTimer.start(5000);
                m_log.log(tr("Connected to XPlane."));
                sendData(true);
                quint8 Proto,Major, Minor, Build;
                stream >> Proto;
                stream >> Major;
                stream >> Minor;
                stream >> Build;
                m_version.Major = static_cast<int>(Major);
                m_version.Minor = static_cast<int>(Minor);
                m_version.Build = static_cast<int>(Build);
                m_version.Name = QString::asprintf("xffs2play_%i_%i_%i", m_version.Major,m_version.Minor,m_version.Build);
#ifdef FFS_DEBUG
                m_log.log("XPManager : Plugin Version Major = " + QString::number(m_version.Major) +
                   " Minor = " + QString::number(m_version.Minor) +
                   " Build = " + QString::number(m_version.Build)
                   ,Qt::darkBlue,LEVEL_NORMAL);
#endif
                sendCallSignColor();
                if (getVersion() == SIM_VERSION::XPLANE_CSL) sendAIPaths();
                emit fireSimConnected();
            }
            m_LastPong=Now();
            break;
        }
        case DATA_SEND:
        {
            try
            {
                data.remove(0,1);
                if (!ProtobufParse(m_PlayerData, data))
                {
                    m_log.log(tr("DataSend Error : bad AirData datagram"),Qt::magenta);
                    break;
                }
                m_state.fromAirData(m_PlayerData);
#ifdef FFS_DEBUG
                m_log.log("XPManager : Receive DATA_SEND event", Qt::darkBlue,LEVEL_VERBOSE);
#endif
                emit fireSimVariableEvent(m_state);
                emit fireSimEventFrame(20,50);
            }
            catch (std::exception& e)
            {
                m_log.log(tr("DataSend Error : ") + QString(e.what()),Qt::magenta);
            }
            break;
        }
        case ADD_PEER:
        {
#ifdef FFS_DEBUG
                m_log.log("XPManager : Receive ADD_PEER " +
                    QString::fromStdString(datagram.data().toHex(':').toStdString())
                    , Qt::darkBlue,LEVEL_NORMAL);
#endif
            try
            {
                stream.setByteOrder(QDataStream::LittleEndian);
                quint32 Object_ID;
                QString CallSign;
                CallSign=StringFromStream(stream);
                stream >> Object_ID;
                if (Object_ID==0)
                {
                    m_log.log("XPManager : " + tr("Receive ADD_PEER event, failed to create object"),Qt::darkMagenta);
                }
                else
                {
#ifdef FFS_DEBUG
                    m_log.log("XPManager : Receive ADD_PEER event, object ID = " +
                    QString::number(Object_ID) + "callsign = " +
                    CallSign
                    , Qt::darkBlue,LEVEL_NORMAL);
#endif
                }
                emit fireSimAICreated(CallSign,Object_ID);
            }
            catch (std::exception& e)
            {
                m_log.log(tr("ADD_PEER Error : ") + QString(e.what()),Qt::magenta);
            }
            break;
        }
        case DEL_PEER:
        {
            try
            {
                stream.setByteOrder(QDataStream::LittleEndian);
                quint32 Object_ID;
                stream >> Object_ID;
#ifdef FFS_DEBUG
                m_log.log("XPManager : Receive DEL_PEER event, object ID = " +
                    QString::number(Object_ID)
                    , Qt::darkBlue,LEVEL_NORMAL);
#endif
                emit fireSimAIRemoved(Object_ID);
            }
            catch (std::exception& e)
            {
                m_log.log(tr("DEL_PEER Error : ") + QString(e.what()),Qt::magenta);
            }
            break;
        }
        case DISC:
        {
            close();
            emit fireSimDisconnected();
            break;
        }
        default:
        {
#ifdef FFS_DEBUG
        m_log.log("XPManager : unknown code = " +
                QString::number(Code),
                Qt::darkBlue,LEVEL_VERBOSE);
#endif
        }
    }
}

///
/// \brief CXPManager::OnPingTimer
///
void CXPManager::OnPingTimer()
{
    if (m_connected)
    {
        if(timespan(Now() - m_LastPong).count()>10000)
        {
            close();
            emit fireSimTimedOut();
        }
        else sendPing();
    }
    else
    {
        m_PingTimer.stop();
        emit fireSimTimedOut();
    }
}
