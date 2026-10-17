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

/****************************************************************************
** aircraftstate.cpp is part of FF2Play project
**
** This class is used to embed aircraft data used for players states
** It can be used for both player and peer data
****************************************************************************/

#include "sim/aircraftstate.h"

///
/// \brief CAircraftState::CAircraftState
///
CAircraftState::CAircraftState ()
{
    TimeStamp = Now();
    Title = "";
    Model = "";
    Type = "";
    Category = "";
    Latitude=0;
    Longitude=0;
    Altitude=0;
    Pitch=0;
    Bank=0;
    Heading=0;
    ElevatorPos=0;
    AileronPos=0;
    RudderPos=0;
    SpoilerPos=0;
    ParkingBrakePos=0;
    GroundAltitude = 0;
    Weight = 0;
    TotalFuelCapacity = 0;
    Vario = 0;
    GroundSpeed = 0;
    TASSpeed = 0;
    IASSpeed = 0;
    Fuel = 0;
    GForce = 0;
    AmbiantWindVelocity = 0;
    AmbiantWindDirection = 0;
    SeaLevelPressure = 0;
    TimeFactor = 1;
    XAxis = 0;
    YAxis = 0;
    Slider = 0;
    RZAxis = 0;
    HatAxis = 0;
    Squawk=0;
    Realism = 0;
    AmbiantPrecipState = 0;
    AltimeterSetting = 0;
    NumEngine = 0;
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
    GearPos=0;
    FlapsIndex=0;
    LandingLight=0;
    StrobeLight=0;
    BeaconLight=0;
    NavLight=0;
    RecoLight=0;
    Smoke=0;
    SquawkMode=0;
    //Etat
    OnGround = false;
    ParkingBrake = false;
    Pause = false;
    Crash = false;
    RunState = true;
    Pushback = false;
    OverSpeed = false;
    Slew = false;
    Stalling = false;
}

///
/// \brief CAircraftState::~CAircraftState
///
CAircraftState::~CAircraftState()
{

}

///
/// \brief CAircraftState::toAirData
/// \param data
///
void CAircraftState::toAirData(AirData &data) const
{
    data.setTimestamp(TimeStamp.time_since_epoch().count());
    data.setTitle(Title);
    data.setModel(Model);
    data.setType(Type);
    data.setCategory(Category);
    data.setLatitude(Latitude);
    data.setLongitude(Longitude);
    data.setAltitude(Altitude);
    data.setPitch(Pitch);
    data.setBank(Bank);
    data.setHeading(Heading);
    data.setElevatorPos(ElevatorPos);
    data.setAileronPos(AileronPos);
    data.setRudderPos(RudderPos);
    data.setSpoilerPos(SpoilerPos);
    data.setGroundAltitude(GroundAltitude);
    data.setIasSpeed(IASSpeed);
    data.setSquawk(Squawk);
    data.setNumEngine(NumEngine);
    data.setDoor1Pos(Door1Pos);
    data.setDoor2Pos(Door2Pos);
    data.setDoor3Pos(Door3Pos);
    data.setDoor4Pos(Door4Pos);
    data.setStateEng1(StateEng1);
    data.setStateEng2(StateEng2);
    data.setStateEng3(StateEng3);
    data.setStateEng4(StateEng4);
    data.setThrottleEng1(ThrottleEng1);
    data.setThrottleEng2(ThrottleEng2);
    data.setThrottleEng3(ThrottleEng3);
    data.setThrottleEng4(ThrottleEng4);
    data.setGearPos(GearPos);
    data.setFlapsIndex(FlapsIndex);
    data.setLandingLight(LandingLight);
    data.setStrobeLight(StrobeLight);
    data.setBeaconLight(BeaconLight);
    data.setNavLight(NavLight);
    data.setRecoLight(RecoLight);
    data.setSmoke(Smoke);
    data.setOnGround(OnGround);
    data.setGndSpeed(GroundSpeed);
    data.setTasSpeed(TASSpeed);
    data.setParkingBrake(ParkingBrake);
    data.setVSpeed(Vario);
    data.setSquawkMode(SquawkMode);
}

///
/// \brief CAircraftState::fromAirData
/// \param data
///
void CAircraftState::fromAirData(const AirData &data)
{
    TimeStamp = fromTimeStamp(static_cast<uint64_t>(data.timestamp()));
    Title = data.title();
    Model = data.model();
    Type = data.type();
    Category = data.category();
    Latitude = data.latitude();
    Longitude= data.longitude();
    Altitude = data.altitude();
    Pitch = data.pitch();
    Bank = data.bank();
    Heading = data.heading();
    ElevatorPos = data.elevatorPos();
    AileronPos = data.aileronPos();
    RudderPos = data.rudderPos();
    SpoilerPos = data.spoilerPos();
    GroundAltitude = data.groundAltitude();
    IASSpeed = data.iasSpeed();
    Squawk = data.squawk();
    NumEngine = data.numEngine();
    Door1Pos = data.door1Pos();
    Door2Pos = data.door2Pos();
    Door3Pos = data.door3Pos();
    Door4Pos = data.door4Pos();
    StateEng1 = data.stateEng1();
    StateEng2 = data.stateEng2();
    StateEng3 = data.stateEng3();
    StateEng4 = data.stateEng4();
    ThrottleEng1 = data.throttleEng1();
    ThrottleEng2 = data.throttleEng2();
    ThrottleEng3 = data.throttleEng3();
    ThrottleEng4 = data.throttleEng4();
    GearPos = data.gearPos();
    FlapsIndex = data.flapsIndex();
    LandingLight = data.landingLight();
    StrobeLight = data.strobeLight();
    BeaconLight = data.beaconLight();
    NavLight = data.navLight();
    RecoLight = data.recoLight();
    Smoke = data.smoke();
    OnGround = data.onGround();
    TASSpeed = data.tasSpeed();
    GroundSpeed = data.gndSpeed();
    ParkingBrake = data.parkingBrake();
    Vario = data.vSpeed();
    SquawkMode = data.squawkMode();
}
