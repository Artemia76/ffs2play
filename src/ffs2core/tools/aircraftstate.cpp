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

#include "tools/aircraftstate.h"

CAircraftState::CAircraftState ()
{
    TimeStamp = QDateTime::currentDateTimeUtc();
    Title = "";
    Model = "";
    Type = "";
    Category = "";
    PlaneAltitude = 0;
    GndAltitude = 0;
    PlaneWeight = 0;
    Vario = 0;
    PlaneHeading = 0;
    TimeFactor = 1;
    Longitude = 0;
    Latitude = 0;
    GndSpeed = 0;
    TASSpeed = 0;
    IASSpeed = 0;
    Fuel = 0;
    TotalFuelCapacity = 0;
    GForce = 0;
    PlanePitch = 0;
    PlaneBank = 0;
    AmbiantWindVelocity = 0;
    AmbiantWindDirection = 0;
    AltimeterSetting = 0;
    SeaLevelPressure = 0;
    XAxis = 0;
    YAxis = 0;
    Slider = 0;
    RZAxis = 0;
    HatAxis = 0;
    Squawk=0;
    Realism = 0;
    AmbiantPrecipState = 0;
    //Etat
    ParkingBrake = false;
    Pause = false;
    Crash = false;
    RunState = true;
    Pushback = false;
    OverSpeed = false;
    Stalling = false;
    Slew = false;
    OnGround = false;
    ElevatorPos = 0;
    AileronPos = 0;
    RudderPos = 0;
    Door1Pos = 0;
    Door2Pos = 0;
    Door3Pos = 0;
    Door4Pos = 0;
    ThrottleEng1 = 0;
    ThrottleEng2 = 0;
    ThrottleEng3 = 0;
    ThrottleEng4 = 0;
    Transponder = 0;
    NumEngine = 0;
    FlapsIndex = 0;
    GearPos = false;
    LandingLight = false;
    StrobeLight = false;
    BeaconLight = false;
    NavLight = false;
    RecoLight = false;
    StateEng1 = false;
    StateEng2 = false;
    StateEng3 = false;
    StateEng4 = false;
    Smoke = false;
}

CAircraftState::~CAircraftState()
{

}

