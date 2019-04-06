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
 * xpmanager.h is part of FF2Play project
 *
 * This is the X-Plane Connector
 * It use the abastractive SimConnector
 * **************************************************************************/

#ifndef XPMANAGER_H
#define XPMANAGER_H

#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QTimer>

#include "sim/simconnector.h"
#include "net/udpserver.h"
#include "protobuf/ffs2play.pb.h"

namespace ffs2play { namespace sim {

using namespace net;

enum FFS2Proto
{
    PING = 0x00, // Ping receive
    PONG = 0x01, // Pong With protocol Version
    DATA = 0x02, // Ask start or stop receive data
    DATA_SEND = 0x03, // Contain data of user plane
    DISC = 0x04, // Disconnect client
    ADD_PEER = 0x05, // Add peer
    DEL_PEER = 0x06, // Remove Peer
    CALLSIGN_COLOR = 0x07, // Send Callsign color
    TCHAT = 0x08, //Send Chat Text
    CSL_PATH = 0x09 //Send CSL Package Paths
};

///
/// \brief The CXPManager class
///

class CXPManager : public SimConnector
{
    Q_OBJECT
public:
    explicit CXPManager(QObject *parent = nullptr);
    ~CXPManager();
private:
    qint64          m_SimRate;
    qint64          m_VariableRate;
    quint16         m_DestPort;
    quint16         m_LocalPort;
    datetime        m_LastPong;
    UDPServer*      m_Server;
    AirData         m_PlayerData;
    QTimer          m_PingTimer;
    QHostAddress    m_XPPlugin;
    AirData         m_SendData;

protected:
    void            open();
    void            close();
    void            main_loop();
    SIM_VERSION     getVersion();
    void            sendScrollingText(const QString&);
    bool            createAI(AIResol&, const QString&, const CAircraftState&, bool);
    bool            removeAI(uint&);
    bool            updateAI(uint, const CAircraftState&);
    void            freezeAI(uint, bool);
    void            sendOptions(){}
    void            sendCallSignColor();
    void            sendAIPaths();

    void            sendPing();
    void            sendData(bool pFlag);
    void            sendDisc();
private slots:
    void            OnReceiveDatagram(QNetworkDatagram datagram);
    void            OnPingTimer();
};

}} // namespace ffs2play::sim

#endif // XPMANAGER_H
