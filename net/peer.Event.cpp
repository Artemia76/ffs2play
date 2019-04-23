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
** Peer.Event.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "net/peer.h"
#include "tools/calculation.h"

void CPeer::OnNewTCPConnection(TCPClient* pTCPClient)
{
#ifdef FFS_DEBUG
    m_log->log("Peer [" + m_callSign + "] " + " OnNewTCPCOnnection", Qt::darkBlue,LEVEL_NORMAL);
#endif
    if (pTCPClient!=nullptr)
    {
        if(!m_EP.isNull() && (m_EP.isEqual(pTCPClient->getPeerAddress())))
        {
            if (m_TCPClient!=nullptr)
            {
                delete(m_TCPClient);
            }
            m_TCPClient = pTCPClient;
            m_TCPServer->accept(m_TCPClient);
            connect(m_TCPClient, SIGNAL(fireIncomingData(QNetworkDatagram)), this, SLOT(OnReceiveMessage(QNetworkDatagram)));
            connect(m_TCPClient, SIGNAL(fireDisconnected(TCPClient*)), this, SLOT(OnTCPClientDisconnected(TCPClient*)));
            m_PeerMode=TCP_SERVER;
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " received incoming TCP connection, switch to TCPServer mode",Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
        else if((!m_EP.isEqual(pTCPClient->getPeerAddress())) || (m_port != pTCPClient->getPeerPort())) return;
    }
}

void CPeer::OnTCPClientDisconnected(TCPClient* pTCPClient)
{
#ifdef FFS_DEBUG
    m_log->log("Peer [" + m_callSign + "] " + " OnTCPClientDisconnected", Qt::darkBlue,LEVEL_NORMAL);
#endif
    if (pTCPClient!=nullptr)
    {
        if (m_TCPClient == pTCPClient)
        {
            disconnect(m_TCPClient, SIGNAL(fireIncomingData(QNetworkDatagram)), this, SLOT(OnReceiveMessage(QNetworkDatagram)));
            disconnect(m_TCPClient, SIGNAL(fireDisconnected(TCPClient*)), this, SLOT(OnTCPClientDisconnected(TCPClient*)));
            m_TCPClient=nullptr;
            m_PeerMode=UDP;
            m_counter = 0;
        }
    }
}

///
/// \brief CPeer::OnReceiveMessage
/// \param pDatagram
///
void CPeer::OnReceiveMessage(QNetworkDatagram pDatagram)
{
    if (m_PeerMode==UDP)
    {
        if(m_EP.isNull())
        {
          if(m_internalIP.contains(pDatagram.senderAddress()))
          {
              m_EP=pDatagram.senderAddress();
          }
          else return;
        }
        else if((!m_EP.isEqual(pDatagram.senderAddress())) || (m_port != pDatagram.senderPort())) return;
    }
    else
    {
        if (m_TCPClient != nullptr)
        {
            if ((!m_EP.isEqual(pDatagram.senderAddress())) || (m_TCPClient->getPeerPort() != pDatagram.senderPort())) return;
        }
    }

    datetime curr_time=Now();
    QByteArray data(pDatagram.data());
    QDataStream stream(&data,QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    quint8 Code;
    stream>>Code;
    switch (static_cast<Protocol>(Code))
    {
        case Protocol::PING:
        {
            sendPong(curr_time);
            break;
        }
        case Protocol::PONG:
        {
            m_latence = duration_cast<milliseconds> (curr_time - m_lastPing)/2;
            m_counter = 0;
            quint64 Time;
            stream >> Time;
            datetime TimePong = fromTimeStamp(Time);
            if (m_miniPing > m_latence)
            {
                m_decalage = duration_cast<milliseconds> (curr_time - m_latence - TimePong);
                m_miniPing = m_latence;
            }
            if (!m_onLine)
            {
#ifdef FFS_DEBUG
                m_log->log("Peer [" + m_callSign + "] " + " go online",Qt::darkBlue,LEVEL_NORMAL);
#endif
                m_onLine = true;
                m_distance = -1;
                m_counter_In = 0;
                m_counter_Out = 0;
            }
            if (m_version == 0) requestVersion();
            break;
        }
        case Protocol::CHAT:
        {
            if(m_blockData) return;
            QString Message;
            quint8 Len;
            stream >> Len;
            char* Buff = new char[Len];
            stream.readRawData(Buff,Len);
            Message = QByteArray::fromRawData(Buff,Len);
            delete[] Buff;
            m_simManager->sendScrollingText(CallSign + " : " + Message);
            emit(fireReceiveTChat(m_callSign,Message));
            break;
        }
        case Protocol::DATA:
        {
            if (m_version == PROTO_VERSION)
            {
                if(m_blockData) return;
                m_mutex.lock();
                quint8 CounterIn;
                stream >> CounterIn;
                m_oldData=m_data;
                data.remove(0,2);
                AirData ReceivedData;
                std::istringstream in(data.toStdString());
                ReceivedData.ParseFromIstream(&in);
                m_data.fromAirData(ReceivedData);
                m_data.TimeStamp += m_decalage;
                if (m_data.TimeStamp<= m_oldData.TimeStamp)
                {
#ifdef FFS_DEBUG
                    m_log->log("Peer [" + m_callSign + "] " + " UDP Datagram dropped : too late" +
                                " Shift mSec=" + QString::number(m_decalage.count()) +
                               " OldTime = " + QString::number(toTimeStamp(m_oldData.TimeStamp)) +
                               " NewTime = " + QString::number(toTimeStamp(m_data.TimeStamp))
                               ,Qt::darkBlue,LEVEL_NORMAL);
#endif
                }
                else
                {
                    if (m_spawned==4) m_spawned++;
                    m_refreshRate = curr_time - m_lastData;
                    m_lastData = curr_time;
                    m_remoteRefreshRate = m_data.TimeStamp - m_oldData.TimeStamp;
                    m_distance = distance(m_data.Latitude,m_data.Longitude,m_sendData.latitude(),m_sendData.longitude(), 'N');
#ifdef FFS_DEBUG
                    if ((CounterIn - m_counter_In) > 1) m_log->log("Peer [" + m_callSign + "] " + " UDP Datagram missing quantity = " + QString::number(CounterIn - m_counter_In - 1),Qt::darkBlue,LEVEL_NORMAL);
#endif
                    m_counter = 0;
                    if (m_spawned >=5) refreshData();
                }
                m_counter_In = CounterIn;
                m_mutex.unlock();
            }
            break;
        }
        case Protocol::VERSION:
        {
            stream >> m_version;
            if (m_version == PROTO_VERSION)
            {
                m_data.Title=StringFromStream(stream);
                if (m_spawned >=3)
                {
                    spawnAI(false);
                }
                m_data.Type=StringFromStream(stream);
                m_data.Model=StringFromStream(stream);
                m_data.Category=StringFromStream(stream);
            }
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " received version number = " + QString::number(m_version) +
                       " Title = " + m_data.Title,Qt::darkBlue,LEVEL_NORMAL);
#endif
            break;
        }
        case Protocol::REQ_VERSION:
        {
            sendVersion();
            break;
        }
        case Protocol::EVENT:
        {
            break;
        }
    }
}

///
/// \brief CPeer::OnHeartBeat
///
void CPeer::OnHeartBeat()
{
    sendPing();
}

///
/// \brief CPeer::OnSimEventFrame
/// \param pFrameRate
///
void CPeer::OnSimEventFrame(float pFrameRate, float)
{
    if ((m_spawned >=6)&& (!m_disableCalculation))
    {
        updateAI(Now(),pFrameRate);
    }
}

///
/// \brief CPeer::OnSimAICreated
/// \param pTail
/// \param pObjectID
///
void CPeer::OnSimAICreated(QString pTail,uint pObjectID)
{
    if ((pTail == m_tail) &&(m_spawned==2))
    {
        if ((pObjectID != 0)&&(m_objectID==0))
        {
            m_objectID = pObjectID;
            m_spawned = 4;
            m_simManager->freezeAI(m_objectID, m_data.OnGround);
            m_lastAIUpdate = Now();
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Receive Object_ID = " + QString::number(m_objectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
        else
        {
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Receive an unwanted object" + QString::number(m_objectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
    }
}

///
/// \brief CPeer::OnSimAIRemoved
/// \param pObjectID
///
void CPeer::OnSimAIRemoved(uint pObjectID)
{
    if (pObjectID == m_objectID)
    {
        m_objectID = 0;
        m_spawned = 0;
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Receive a removed Object from sim ID = " + QString::number(pObjectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
}

///
/// \brief CPeer::OnSimEvent
///
void CPeer::OnSimEvent(SimEvent)
{

}

void CPeer::OnSimClosed()
{
    m_objectID = 0;
    m_spawned = 0;
#ifdef FFS_DEBUG
    m_log->log("Peer [" + m_callSign + "] " + " Receive a SimClosed Event",Qt::darkBlue,LEVEL_NORMAL);
#endif
}

///
/// \brief CPeer::OnSimAIUpdated
/// \param pAIData
/// \param pObjectID
///
void CPeer::OnSimAIUpdated(CAircraftState pAIData, uint pObjectID)
{
    if (m_objectID != pObjectID) return;
    m_lastAIUpdate = Now();
    m_AISimData = pAIData;
    m_ecart = distance(m_data.Latitude,m_data.Longitude,m_AISimData.Latitude,m_AISimData.Longitude, 'N');
}

///
/// \brief CPeer::OnSimStateChange
/// \param pState
///
void CPeer::OnSimStateChange(CAircraftState pState)
{
    bool Title_Changed = false;
    if (m_sendData.title() != pState.Title.toStdString())
    {
        Title_Changed = true;
    }
    pState.toAirData(m_sendData);
    if ((pState.TimeStamp - m_lastStateEvent) < m_sendInterval) return;
    if ((!m_onLine) || (m_EP.isNull()) || (m_version < 1)) return;
    sendData();
    if (Title_Changed) sendVersion();
    m_lastStateEvent = pState.TimeStamp;
    // AI Display Management
    m_spawnable = (m_distance < P2PRadius) && (m_distance >=0);
    if (m_spawnable)
    {
        m_sendInterval = milliseconds(P2PRate);
    }
    else
    {
        m_sendInterval =  milliseconds(5000);
    }
    if (m_spawnable && m_visible && (m_spawned == 0))
    {
        spawnAI(true);
#ifdef FFS_DEBUG
        m_log->log("Peer [" + m_callSign + "] " + " Build AI Object" ,Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
    if (((!m_spawnable) || (!m_visible)) && (m_spawned > 0))
    {
        spawnAI(false);
#ifdef FFS_DEBUG
        m_log->log("Peer [" + m_callSign + "] " + " Destruct AI Object Visible=" + QString::number(m_visible) + " Spawnable = " + QString::number(m_spawnable),Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
}

///
/// \brief CPeer::OnTimerCreateAI
///
void CPeer::OnTimerCreateAI()
{
    m_timerCreateAI.stop();
    if ((m_spawned >= 2) && (m_objectID == 0))
    {
        m_spawned = 2;
        if (!m_trySimpleAI)
        {
            m_trySimpleAI = true;
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Simulator have not created the requested AI, Let's try with simple object" ,Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
        else
        {
            if (m_simManager->getType()==SIM_TYPE::FSX)
            {
                m_AIResolution.set_title("Mooney Bravo");
                m_AIResolution.set_cgheight(3.7);
                m_AIResolution.set_pitch(2.9);
            }
            else if (m_simManager->getVersion()==SimConnector::SIM_VERSION::XPLANE)
            {
                m_AIResolution.set_title("Beech Baron 58");
                m_AIResolution.set_cgheight(0.9);
                m_AIResolution.set_pitch(1.93);
            }
            else if (m_simManager->getVersion()==SimConnector::SIM_VERSION::XPLANE_CSL)
            {
                m_AIResolution.set_title("C172:C172:0");
                m_AIResolution.set_cgheight(0.0);
                m_AIResolution.set_pitch(0.0);
            }
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Simulator have not created the requested AI, we are trying with default simulator aircraft" ,Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
        createAI();
    }
    else
    {
#ifdef FFS_DEBUG
            m_log->log("Peer [" + m_callSign + "] " + " Simulator has been created the requested AI" ,Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
}

///
/// \brief CPeer::OnSendTChat
/// \param pMessage
///
void CPeer::OnSendTChat(QString pMessage)
{
    if (m_onLine)
    {
        sendChat(pMessage);
    }
}
