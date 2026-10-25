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
#include <QHash>
#include <QMap>
#include <QVector>

#include "sim/simconnector.h"
#include "sim/fgdatagram.h"
#include "sim/aircraftstate.h"
#include "net/udpserver.h"

/// FlightGear drops a multiplayer aircraft 10 s after its last message : delay
/// before a removed callsign can be used again with another model (ms)
constexpr int FGReleaseDelay = 15000;
/// Peers only send the gear handle and the flaps index : time of a full travel
/// shown in FlightGear (s)
constexpr double FGGearTravelTime = 6.0;
constexpr double FGFlapsTravelTime = 8.0;

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
    /// Lights and doors in the generic properties of the user aircraft
    simgear::FGGenericMap m_InfoMap;
    /// Lights and doors of the models displayed for the peers, by model path
    QHash<QString, simgear::FGGenericMap> m_ModelMaps;
    QStringList     m_FGRoots;

    /// Peer aircraft displayed in FlightGear as a multiplayer aircraft
    struct FGAIObject
    {
        QString         Tail;
        /// Callsign in FlightGear, which identifies the aircraft there (7 characters)
        QString         CallSign;
        QString         Model;
        simgear::FGGenericMap Map;
        CAircraftState  State;
        datetime        LastSend;
        /// Nothing sent before : FlightGear still knows the previous aircraft with this callsign
        datetime        HoldUntil;
        bool            Sent = false;
        /// Gear and flaps shown (0 to 1), moving to the state of the peer
        double          GearNorm = 0.0;
        double          FlapsNorm = 0.0;
        datetime        RampTime;
        bool            Ramped = false;
    };
    QMap<uint, FGAIObject> m_AIObjects;
    uint            m_NextObjectID;
    /// Removed callsigns, still known by FlightGear for 10 s with their model
    struct FGReleased
    {
        datetime        Time;
        QString         Model;
    };
    QMap<QString, FGReleased> m_ReleasedCallSigns;

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
    void            updateRamps(FGAIObject& pObject);
    simgear::FGGenericMap loadGenericMap(const QString& pFile);
    simgear::FGGenericMap modelMap(const QString& pModel);

private slots:
    void            OnReceiveDatagram(QNetworkDatagram datagram);
    void            OnPingTimer();
};

#endif // FGMANAGER_H
