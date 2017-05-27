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

#ifndef SIMCONNECT_H
#define SIMCONNECT_H

#include <QObject>
#include <QThread>
#include <QDateTime>

#include <windows.h>
#include <WinDef.h>

#include "singleton/clogger.h"
#include "SimConnect.h"
#include "tools/ffs2core_global.h"

const int WM_USER_SIMCONNECT = 0x402;

struct FFS2CORE_EXPORT SAI_Data
{
    char    title[256];
    double  kohlsmann;
    double  altitude;
    double  latitude;
    double  longitude;
};


enum class FFS2CORE_EXPORT GROUP_ID
{
    JOYSTICK,
    AI
};

/// <summary>
/// Enumération des évenements du simulateur à traiter
/// </summary>
enum class FFS2CORE_EXPORT EVENT_ID
{
    PERIODIQUE,
    PAUSE,
    PARKING_BRAKE,
    SIMSTART,
    SIMSTOP,
    OVERSPEED,
    STALLING,
    CRASH,
    SLEW,
    PUSHBACK,
    ON_GROUND,
    GEAR_HANDLE_POSITION,
    FLAPS_HANDLE_PERCENT,
    FLAPS_HANDLE_INDEX,
    ALTIMETER_SETTING,
    LANDING_LIGHT,
    BEACON_LIGHT,
    STROBE_LIGHT,
    NAV_LIGHT,
    RECOGNITION_LIGHT,
    SMOKE_ENABLE,
    FREEZE_LATLONG,
    FREEZE_ALTITUDE,
    FREEZE_ATTITUDE,
    OBJECT_REMOVED,
    SLEW_ON,
    ADDED_AIRCRAFT,
    REMOVED_AIRCRAFT,
    FRAME,
    TEXT_SCROLL,
    SLIDER = 100,
    XAXIS,
    YAXIS,
    RZAXIS,
    HAT,
    SLIDER2 = 110,
    XAXIS2,
    YAXIS2,
    RZAXIS2,
    HAT2,
    SLIDER3 = 120,
    XAXIS3,
    YAXIS3,
    RZAXIS3,
    HAT3,
    UNUSED = 0xFFFFFFF,
};

/// <summary>
/// Enumération des requêtes
/// à partir de 100, réservé pour les requetes de création et destruction d'avion
/// </summary>
enum class FFS2CORE_EXPORT REQUESTS_ID
{
    PERIODIQUE,
    PAUSE,
    PARKING_BRAKE,
    SIMSTART,
    SIMSTOP,
    OVERSPEED,
    STALLING,
    CRASH,
    SLEW,
    PUSHBACK,
    ON_GROUND,
    GEAR_HANDLE_POSITION,
    FLAPS_HANDLE_PERCENT,
    FLAPS_HANDLE_INDEX,
    ALTIMETER_SETTING,
    LANDING_LIGHT,
    BEACON_LIGHT,
    STROBE_LIGHT,
    NAV_LIGHT,
    RECOGNITION_LIGHT,
    SMOKE_ENABLE,
    SYSTEM,
    JOY_INFO,
    AI_RELEASE,
    AI_CREATE = 0x100000,
    AI_UPDATE = 0x200000,
};

enum class FFS2CORE_EXPORT DEFINITIONS_ID
{
    PERIODIQUE,
    PARKING_BRAKE,
    OVERSPEED,
    STALLING,
    CRASH,
    SLEW,
    PUSHBACK,
    ON_GROUND,
    GEAR_HANDLE_POSITION,
    FLAPS_HANDLE_PERCENT,
    FLAPS_HANDLE_INDEX,
    ALTIMETER_SETTING,
    LANDING_LIGHT,
    BEACON_LIGHT,
    STROBE_LIGHT,
    NAV_LIGHT,
    RECOGNITION_LIGHT,
    SMOKE_ENABLE,
    IS_SLEW_ACTIVE,
    AI_MOVE,
    AI_INIT
};

/// <summary>
/// Structure d'importation des données envoyées par
/// la librairie simconnect
/// </summary>
struct FFS2CORE_EXPORT DonneesAvion
{
    char Title[256];
    char Type[256];
    char Model[256];
    char Category[256];
    double PlaneAltitude;
    double GndAltitude;
    double Vario;
    double PlaneHeading;
    qint32 TimeFactor;
    double Longitude;
    double Latitude;
    double GndSpeed;
    double TASSpeed;
    double FuelQty;
    double TotalFuelCapacity;
    double FuelWeightGallon;
    double PlanePitch;
    double PlaneBank;
    double GForce;
    double PlaneWeight;
    double Realism;
    double AmbiantWindVelocity;
    double AmbiantWindDirection;
    quint32 AmbiantPrecipState;
    double SeaLevelPressure;
    double IASSpeed;
    double ElevatorPos;
    double AileronPos;
    double RudderPos;
    qint32 Door1Pos;
    qint32 Door2Pos;
    qint32 Door3Pos;
    qint32 Door4Pos;
    qint32 NumEngine;
    qint32 StateEng1;
    qint32 StateEng2;
    qint32 StateEng3;
    qint32 StateEng4;
    qint32 ThrottleEng1;
    qint32 ThrottleEng2;
    qint32 ThrottleEng3;
    qint32 ThrottleEng4;
    qint32 Squawk;
};

class FFS2CORE_EXPORT SCManagerEventSCEvent
{
public:
    SCManagerEventSCEvent()
    {
        Time = QDateTime::currentDateTimeUtc();
    }

    SCManagerEventSCEvent(EVENT_ID pEvt_ID, int pData )
    {
        Time = QDateTime::currentDateTimeUtc();
        Data = pData;
        Evt_Id = pEvt_ID;
    }

    EVENT_ID Evt_Id;
    int Data;
    QDateTime Time;
};

class FFS2CORE_EXPORT SCManagerEventSCVariable
{
public:
    SCManagerEventSCVariable ()
    {
        Time = QDateTime::currentDateTimeUtc();
    }

    SCManagerEventSCVariable (DonneesAvion* pData)
    {
        Data = *pData;
        Time = QDateTime::currentDateTimeUtc();
    }
    DonneesAvion Data;
    QDateTime Time;
};

class FFS2CORE_EXPORT CSCManager : public QObject
{
    Q_OBJECT
private:
    static CSCManager* createInstance();
    explicit CSCManager(QObject *parent = 0);
    CLogger*    m_logger;
    bool        m_bConnected;
    bool        m_bRunning;
    HANDLE      m_hSimConnect;
    QThread*    m_thread;
    qint64      m_SimRate;
    qint64      m_VariableRate;
    QDateTime   m_LastVariable;
    QDateTime   m_LastSim;

    void        InitSimConnect();
    void        OnRecvOpen (SIMCONNECT_RECV_OPEN* data);
    void        OnRecvQuit (SIMCONNECT_RECV* data);
    void        OnRecvSystemState(SIMCONNECT_RECV_SYSTEM_STATE* data);
    void        OnRecvException (SIMCONNECT_RECV_EXCEPTION* data);
    void        OnRecvEvent (SIMCONNECT_RECV_EVENT* data);
    void        OnRecvSimObjectData (SIMCONNECT_RECV_SIMOBJECT_DATA* data);
    void        OnRecvEventFrame(SIMCONNECT_RECV_EVENT_FRAME* data);
    void        OnRecvEventObjectAddremove(SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* data);
    void        OnRecvAssignedObjectId(SIMCONNECT_RECV_ASSIGNED_OBJECT_ID* data);

static void CALLBACK DispatchCallback(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
    void        process(SIMCONNECT_RECV *pData, DWORD cbData);
private slots:
    void        mainThread();
public slots:
    void        open_close();
signals:
    void        finished();
    void        opened();
    void        closed();
    void        fireSCReceiveVariable(SCManagerEventSCVariable);
    void        fireSCReceiveEvent(SCManagerEventSCEvent);
public:
    ~CSCManager ();
    static CSCManager* instance();
    void        open();
    void        close();
};

#endif // SIMCONNECT_H
