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
/// \return
///
bool CFGManager::createAI(AIResol& /*pAircraft*/,const QString& /*pTail*/,const CAircraftState& /*pData*/,bool /*pNonAIObject*/)
{
#ifdef FFS_DEBUG
        m_log.log("FGManager : Send createAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
    return false;
}

///
/// \brief CFGManager::removeAI
/// \return
///
bool CFGManager::removeAI(uint& /*pObjectID*/)
{
#ifdef FFS_DEBUG
        m_log.log("FGManager : Send removeAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
    return false;
}

///
/// \brief CFGManager::updateAI
/// \param pObjectID
/// \param pData
/// \return
///
bool CFGManager::updateAI(uint /*pObjectID*/, const CAircraftState& /*pData*/)
{
#ifdef FFS_DEBUG
        m_log.log("FGManager : Send updateAI", Qt::darkBlue,LEVEL_NORMAL);
#endif
    return false;
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
/// \brief CFGManager::OnPingTimer
///
void CFGManager::OnPingTimer()
{
    if (m_connected)
    {
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
