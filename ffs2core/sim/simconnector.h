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

#ifndef SIMCONNECTOR_H
#define SIMCONNECTOR_H

#include <QObject>
#include "sim/aircraftstate.h"
#include "singleton/clogger.h"
#include "tools/ffs2core_global.h"
#include "tools/datetime.h"

/// <summary>
/// Enumération des évenements du simulateur à traiter
/// </summary>
enum class FFS2CORE_EXPORT SIM_EVENT_ID
{
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
    SLEW_ON,
    UNUSED = 0xFFFFFFF,
};

class FFS2CORE_EXPORT SimEvent
{
public:
    SimEvent()
    {
        Time = Now();
    }

    SimEvent(SIM_EVENT_ID pEvt_ID, int pData )
    {
        Time = Now();
        Data = pData;
        Evt_Id = pEvt_ID;
    }

    SIM_EVENT_ID Evt_Id;
    int Data;
    datetime Time;
};

class FFS2CORE_EXPORT SimConnector : public QObject
{
    Q_OBJECT
    friend class simbasemanager;
public:
    explicit        SimConnector(QObject *parent = nullptr);
    virtual         ~SimConnector();
    bool            isConnected();
protected:
    CLogger*        Log;
    bool            m_bConnected;
    CAircraftState  State;
    virtual bool    open()=0;
    virtual void    close()=0;
    virtual void    main_loop()=0;
signals:
    void            finished();
    void            opened();
    void            closed();
    void            fireSimVariableEvent(CAircraftState);
    void            fireSimEvent(SimEvent);
public slots:
};

#endif // SIMCONNECTOR_H
