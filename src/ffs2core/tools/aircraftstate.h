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

#include <QObject>
#include <QDateTime>
#include "tools/ffs2core_global.h"

class FFS2CORE_EXPORT CAircraftState
{
public:
    CAircraftState();
    CAircraftState(const CAircraftState& Object);
    ~CAircraftState();
    QDateTime TimeStamp;
    QString Title;
    QString Model;
    QString Type;
    QString Category;
    double PlaneAltitude;
    double GndAltitude;
    double PlaneWeight;
    double TotalFuelCapacity;
    double Vario;
    double PlaneHeading;
    int TimeFactor;
    double Longitude;
    double Latitude;
    double GndSpeed;
    double TASSpeed;
    double IASSpeed;
    double Fuel;
    double GForce;
    double PlanePitch;
    double PlaneBank;
    double AmbiantWindVelocity;
    double AmbiantWindDirection;
    double SeaLevelPressure;
    qint32  XAxis;
    qint32 YAxis;
    qint32 Slider;
    qint32 RZAxis;
    qint32 HatAxis;
    qint32 Squawk;
    qint32 Realism;
    quint32 AmbiantPrecipState;
    qint32 AltimeterSetting;
    double ElevatorPos;
    double AileronPos;
    double RudderPos;
    qint32 Door1Pos;
    qint32 Door2Pos;
    qint32 Door3Pos;
    qint32 Door4Pos;
    qint32 FlapsIndex;
    qint32 NumEngine;
    qint32 ThrottleEng1;
    qint32 ThrottleEng2;
    qint32 ThrottleEng3;
    qint32 ThrottleEng4;
    qint32 Transponder;
    //Etat
    bool ParkingBrake;
    bool Pause;
    bool Crash;
    bool RunState;
    bool Pushback;
    bool OverSpeed;
    bool Slew;
    bool OnGround;
    bool Stalling;
    bool GearPos;
    bool LandingLight;
    bool BeaconLight;
    bool StrobeLight;
    bool NavLight;
    bool RecoLight;
    bool StateEng1;
    bool StateEng2;
    bool StateEng3;
    bool StateEng4;
    bool Smoke;
};

#endif // AIRCRAFTSTATE_H
