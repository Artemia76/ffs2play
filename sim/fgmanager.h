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
* fgmanager.h is part of FF2Play project
*
* This is the Flight Gear Connector
* It use the abastractive SimConnector
* **************************************************************************/

#ifndef FGMANAGER_H
#define FGMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMap>
#include <QVector>

#include "sim/simconnector.h"
#include "sim/fgdatagram.h"
#include "sim/aircraftstate.h"
#include "net/udpserver.h"

///
/// \brief The CFGManager class
///
class CFGManager : public SimConnector
{
    Q_OBJECT
public:
    explicit CFGManager(QObject *parent = nullptr);
    ~CFGManager() override;
private:
    QTimer          m_PingTimer;
    datetime        m_LastPong;
    QHostAddress    m_FGAddress;
    quint16         m_FGPort;
    quint16         m_LocalPort;
    UDPServer*      m_Server;
    /// ICAO type, type and category of the aircraft found in the AI database for m_InfoTitle
    QString         m_InfoTitle;
    QString         m_InfoModel;
    QString         m_InfoType;
    QString         m_InfoCategory;
    bool            m_InfoFound;
    int             m_InfoRetry;

    /// Peer aircraft displayed in FlightGear as a multiplayer aircraft
    struct FGAIObject
    {
        QString         Tail;
        /// Callsign in FlightGear, which identifies the aircraft there (7 characters)
        QString         CallSign;
        QString         Model;
        CAircraftState  State;
        datetime        LastSend;
    };
    QMap<uint, FGAIObject> m_AIObjects;
    uint            m_NextObjectID;
    /// Callsigns of the removed aircraft, still known by FlightGear for 10 s
    QMap<QString, datetime> m_ReleasedCallSigns;

protected:
    void            open() override;
    void            close() override;
    SIM_VERSION     getVersion() override;
    void            sendScrollingText(const QString&) override;
    bool            createAI(AIResol&, const QString&, const CAircraftState&, bool) override;
    bool            removeAI(uint&) override;
    bool            updateAI(uint, const CAircraftState&) override;
    void            freezeAI(uint, bool) override;
    void            sendOptions() override {}
    void            sendCallSignColor() override {}

    void            sendPing();
    void            sendData();
    void            sendDisc();
    void            updateAircraftInfo();
    bool            sendAI(FGAIObject& pObject);
    void            hideAI(FGAIObject& pObject);
    QString         makeCallSign(const QString& pTail);

private slots:
    void            OnReceiveDatagram(QNetworkDatagram datagram);
    void            OnPingTimer();
};

#endif // FGMANAGER_H
