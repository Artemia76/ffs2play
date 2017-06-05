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

#ifndef AIRCRAFTSTATE_H
#define AIRCRAFTSTATE_H

#include <QDateTime>
#include "sim/aibase.h"

using namespace ffs2play;

class FFS2CORE_EXPORT CAircraftState : public CAIBase
{
public:
    CAircraftState();
    ~CAircraftState();
    virtual void toAirData (AirData& pAirData);
    virtual void fromAirData (const AirData& pAirData);
    QDateTime   TimeStamp;
    QString     Title;
    QString     Model;
    QString     Type;
    QString     Category;
    double      GroundAltitude;
    double      Weight;
    double      TotalFuelCapacity;
    double      Vario;
    qint32      TimeFactor;
    double      GroundSpeed;
    double      TASSpeed;
    double      IASSpeed;
    double      Fuel;
    double      GForce;
    double      AmbiantWindVelocity;
    double      AmbiantWindDirection;
    double      SeaLevelPressure;
    qint32      XAxis;
    qint32      YAxis;
    qint32      Slider;
    qint32      RZAxis;
    qint32      HatAxis;
    qint32      Squawk;
    qint32      Realism;
    quint32     AmbiantPrecipState;
    qint32      AltimeterSetting;
    qint32      NumEngine;
    //Etat
    bool        ParkingBrake;
    bool        Pause;
    bool        Crash;
    bool        RunState;
    bool        Pushback;
    bool        OverSpeed;
    bool        Slew;
    bool        OnGround;
    bool        Stalling;
};

#endif // AIRCRAFTSTATE_H
