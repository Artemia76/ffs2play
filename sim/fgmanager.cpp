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
** fgmanager.cpp is part of FF2Play project
**
** This is the Flight Gear Connector
** It use the abastractive SimConnector
****************************************************************************/

#include <QDataStream>
#include <QNetworkDatagram>

#include "fgmanager.h"

#include "tools/datagram.h"
#include "disk/aimapping.h"

using namespace simgear;

///
/// \brief FlightGear Manager Constructor
/// \param parent
///
CFGManager::CFGManager(QObject* parent):
    SimConnector(parent)
{
    m_LocalPort=5000;
    m_FGPort=0;
    m_InfoFound=false;
    m_InfoRetry=0;
    m_NextObjectID=1;
    m_Server = new UDPServer(this);
    connect(m_Server, &UDPServer::fireIncomingData, this, &CFGManager::OnReceiveDatagram);
    connect(&m_PingTimer, &QTimer::timeout, this, &CFGManager::OnPingTimer);
    m_log.log("FGManager : " + tr("FlightGear Module loaded"));
}

///
/// \brief CFGManager::~CFGManager
///
CFGManager::~CFGManager()
{
    close();
    m_Server->stop();
    m_log.log("FGManager : " + tr("FlightGear Module unloaded"));
}

///
/// \brief CFGManager::open
///
void CFGManager::open ()
{
    if (!m_connected)
    {
        m_log.log(tr("Connecting to FlightGear ..."));
        m_Server->setPort(m_LocalPort);
        m_PingTimer.start(500);
        if (m_Server->start())
        {
            m_connected=true;
            m_log.log(tr("Connected to FlightGear."));
            emit fireSimConnected();
        }
    }
}

///
/// \brief CFGManager::close
///
void CFGManager::close ()
{
    m_PingTimer.stop();
    if (m_connected)
    {
        sendDisc();
        // FlightGear removes them 10 s after their last position
        m_AIObjects.clear();
        m_Server->stop();
        m_connected=false;
        m_log.log(tr("Disconnected from FlightGear."));
    }
}


///
/// \brief CFGManager::getVersion
/// \return
///
SimConnector::SIM_VERSION CFGManager::getVersion()
{
    return SIM_VERSION::FLIGHTGEAR;
}

///
/// \brief CFGManager::sendScrollingText
///
void CFGManager::sendScrollingText(const QString& /*pMessage*/)
{

}

///
/// \brief CFGManager::createAI
/// FlightGear creates a multiplayer aircraft when it receives a position message
/// with a new callsign : the object ID is given by the connector
/// \param pAircraft model path of the aircraft to display (AIMapping::SolveTitle)
/// \param pTail callsign of the peer, truncated to 7 characters by FlightGear
/// \param pData
/// \return false while FlightGear has not sent its first packet (the peer retries)
///
bool CFGManager::createAI(AIResol& pAircraft,const QString& pTail,const CAircraftState& pData,bool /*pNonAIObject*/)
{
    if ((!m_connected) || (m_FGPort == 0)) return false;
    const uint ObjectID = m_NextObjectID++;
    FGAIObject& Object = m_AIObjects[ObjectID];
    Object.Tail = pTail;
    Object.Model = pAircraft.title();
    Object.State = pData;
    sendAI(Object);
#ifdef FFS_DEBUG
    m_log.log("FGManager : Send createAI " + pTail + " , Model = " + Object.Model + " , Object ID = " + QString::number(ObjectID), Qt::darkBlue,LEVEL_NORMAL);
#endif
    // Answered after the return, like the X-Plane plugin, when the peer is waiting for it
    QTimer::singleShot(0, this, [this, pTail, ObjectID]()
    {
        if (m_AIObjects.contains(ObjectID)) emit fireSimAICreated(pTail, ObjectID);
    });
    return true;
}

///
/// \brief CFGManager::removeAI
/// There is no remove message : FlightGear removes a multiplayer aircraft
/// 10 s after its last position message
/// \param pObjectID set to 0
/// \return
///
bool CFGManager::removeAI(uint& pObjectID)
{
#ifdef FFS_DEBUG
    m_log.log("FGManager : Send removeAI " + QString::number(pObjectID), Qt::darkBlue,LEVEL_NORMAL);
#endif
    m_AIObjects.remove(pObjectID);
    pObjectID = 0;
    return true;
}

///
/// \brief CFGManager::updateAI
/// \param pObjectID
/// \param pData
/// \return
///
bool CFGManager::updateAI(uint pObjectID, const CAircraftState& pData)
{
    auto it = m_AIObjects.find(pObjectID);
    if (it == m_AIObjects.end()) return false;
#ifdef FFS_DEBUG
    m_log.log("FGManager : Send updateAI " + it->Tail, Qt::darkBlue,LEVEL_VERBOSE);
#endif
    it->State = pData;
    return sendAI(*it);
}

///
/// \brief CFGManager::sendAI
/// Sends the position message of a peer aircraft to FlightGear
/// \param pObject
/// \return
///
bool CFGManager::sendAI(FGAIObject& pObject)
{
    if ((!m_connected) || (m_FGPort == 0)) return false;
    FGDatagram Datagram;
    Datagram.SetIdentity(pObject.Tail, pObject.Model);
    Datagram.SetAirData(pObject.State);
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    Datagram.Write(stream);
    // FlightGear receives on the socket it sends from
    m_Server->send(QNetworkDatagram(Data, m_FGAddress, m_FGPort));
    pObject.LastSend = Now();
    return true;
}

///
/// \brief CFGManager::freezeAI
///
void CFGManager::freezeAI(uint /*pObjectID*/, bool /*pState*/)
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("FGManager : Send freezeAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
}

///
/// \brief CFGManager::sendPing
///
void CFGManager::sendPing()
{
#ifdef FFS_DEBUG
    m_log.log("FGManager : Send PING", Qt::darkBlue,LEVEL_NORMAL);
#endif
    /*QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << (quint8)PING;
    stream << (quint32)m_options;
    QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
    m_Server->send(Output);*/
}

///
/// \brief CFGManager::sendData
///
void CFGManager::sendData()
{
    if (m_connected)
    {
#ifdef FFS_DEBUG
        m_log.log("FGManager : Send DATA", Qt::darkBlue,LEVEL_NORMAL);
#endif
        /*QByteArray Data;
        QDataStream stream(&Data,QIODevice::WriteOnly);
        stream << (quint8)DATA;
        stream << (quint8)pFlag;
        QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
        m_Server->send(Output);*/
    }
}

///
/// \brief CFGManager::sendDisc
///
void CFGManager::sendDisc()
{
    /*QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream << (quint8)DISC;
    QNetworkDatagram Output(Data,m_XPPlugin,m_DestPort);
    m_Server->send(Output);*/
}

///
/// \brief CXPManager::OnReceiveDatagram
///
void CFGManager::OnReceiveDatagram(QNetworkDatagram datagram)
{
    m_FGAddress = datagram.senderAddress();
    m_FGPort = datagram.senderPort();
    QByteArray data(datagram.data());
    QDataStream stream(&data,QIODevice::ReadOnly);
    // Reading Header
    FGDatagram FGData;
    FGData.Read(stream);
    if (!FGData.IsSane()) return;
    if (FGData.GetAirData(m_state))
    {
        // Peers drop a DATA not newer than the previous one
        m_state.TimeStamp = Now();
        updateAircraftInfo();
#ifdef FFS_DEBUG
        m_log.log("FGManager : Received Data from FG : Model = " +
            m_state.Title +
            " , Latitude = " + QString::number(m_state.Latitude) +
            " , Longitude = " + QString::number(m_state.Longitude) +
            " , Altitude = " + QString::number(m_state.Altitude)
            , Qt::darkBlue,LEVEL_VERBOSE);
#endif
        emit fireSimVariableEvent(m_state);
        emit fireSimEventFrame(20,50);
    }
}

///
/// \brief CFGManager::updateAircraftInfo
/// FlightGear only sends the model path : the ICAO type, the type and the category come from
/// the AI database (FGAIParser). Not called from the constructor : AIMapping
/// needs simbasemanager, which is creating this connector.
///
void CFGManager::updateAircraftInfo()
{
    if (m_state.Title != m_InfoTitle)
    {
        m_InfoTitle = m_state.Title;
        m_InfoModel.clear();
        m_InfoType.clear();
        m_InfoCategory.clear();
        m_InfoFound = false;
        m_InfoRetry = 0;
    }
    // While the aircraft is not found (scan running), retry every 100 packets
    if ((!m_InfoFound) && (--m_InfoRetry <= 0))
    {
        m_InfoRetry = 100;
        m_InfoFound = AIMapping::instance().getAircraftInfo(m_InfoTitle, m_InfoModel, m_InfoType, m_InfoCategory);
#ifdef FFS_DEBUG
        if (m_InfoFound)
            m_log.log("FGManager : Aircraft found in AI database : ICAO = " + m_InfoModel + " , Type = " + m_InfoType + " , Category = " + m_InfoCategory, Qt::darkBlue,LEVEL_NORMAL);
#endif
    }
    m_state.Model = m_InfoModel;
    m_state.Type = m_InfoType;
    m_state.Category = m_InfoCategory;
}

///
/// \brief CFGManager::OnPingTimer
///
void CFGManager::OnPingTimer()
{
    if (m_connected)
    {
        // Keeps the peer aircraft alive in FlightGear between two DATA of the peer
        const datetime Time = Now();
        for (FGAIObject& Object : m_AIObjects)
        {
            if ((Time - Object.LastSend) > milliseconds(1000)) sendAI(Object);
        }
        /*if(timespan(Now() - m_LastPong).count()>10000)
        {
            close();
            emit fireSimTimedOut();
        }*/
        //else sendPing();
    }
    else
    {
        m_PingTimer.stop();
        emit fireSimTimedOut();
    }
}
