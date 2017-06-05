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

#include "sim/aibase.h"

CAIBase::CAIBase ()
{
    Latitude=0;
    Longitude=0;
    Altitude=0;
    Pitch=0;
    Bank=0;
    Heading=0;
    ElevatorPos=0;
    AileronPos=0;
    RudderPos=0;
    Door1Pos=0;
    Door2Pos=0;
    Door3Pos=0;
    Door4Pos=0;
    StateEng1=0;
    StateEng2=0;
    StateEng3=0;
    StateEng4=0;
    ThrottleEng1=0;
    ThrottleEng2=0;
    ThrottleEng3=0;
    ThrottleEng4=0;
    Squawk=0;
    GearPos=0;
    FlapsIndex=0;
    LandingLight=0;
    StrobeLight=0;
    BeaconLight=0;
    NavLight=0;
    RecoLight=0;
    Smoke=0;
}

CAIBase::~CAIBase()
{

}

void CAIBase::toAirData(AirData &data)
{
    data.set_aileronpos(AileronPos);
    data.set_altitude(Altitude);
    data.set_bank(Bank);
    data.set_beaconlight(BeaconLight);
    data.set_door1pos(Door1Pos);
    data.set_door2pos(Door2Pos);
    data.set_door3pos(Door3Pos);
    data.set_door4pos(Door4Pos);
    data.set_elevatorpos(ElevatorPos);
    data.set_flapsindex(FlapsIndex);
    data.set_gearpos(GearPos);
    data.set_heading(Heading);
    data.set_landinglight(LandingLight);
    data.set_latitude(Latitude);
    data.set_longitude(Longitude);
    data.set_navlight(NavLight);
    data.set_pitch(Pitch);
    data.set_recolight(RecoLight);
    data.set_rudderpos(RudderPos);
    data.set_smoke(Smoke);
    data.set_squawk(Squawk);
    data.set_stateeng1(StateEng1);
    data.set_stateeng2(StateEng2);
    data.set_stateeng3(StateEng3);
    data.set_stateeng4(StateEng4);
    data.set_strobelight(StrobeLight);
    data.set_throttleeng1(ThrottleEng1);
    data.set_throttleeng2(ThrottleEng2);
    data.set_throttleeng3(ThrottleEng3);
    data.set_throttleeng4(ThrottleEng4);
}

void CAIBase::fromAirData(const AirData &data)
{
    AileronPos = data.aileronpos();
    Altitude = data.altitude();
    Bank = data.bank();
    BeaconLight = data.beaconlight();
    Door1Pos = data.door1pos();
    Door2Pos = data.door2pos();
    Door3Pos = data.door3pos();
    Door4Pos = data.door4pos();
    ElevatorPos = data.elevatorpos();
    FlapsIndex = data.flapsindex();
    GearPos = data.gearpos();
    Heading = data.heading();
    LandingLight = data.landinglight();
    Latitude = data.latitude();
    Longitude= data.longitude();
    NavLight = data.navlight();
    Pitch = data.pitch();
    RecoLight = data.recolight();
    RudderPos = data.rudderpos();
    Smoke = data.smoke();
    Squawk = data.squawk();
    StateEng1 = data.stateeng1();
    StateEng2 = data.stateeng2();
    StateEng3 = data.stateeng3();
    StateEng4 = data.stateeng4();
    StrobeLight = data.strobelight();
    ThrottleEng1 = data.throttleeng1();
    ThrottleEng2 = data.throttleeng2();
    ThrottleEng3 = data.throttleeng3();
    ThrottleEng4 = data.throttleeng4();
}
