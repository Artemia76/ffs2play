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

CAircraftState::CAircraftState(const CAircraftState &Object)
{
    TimeStamp = Object.TimeStamp;
    Title = Object.Title;
    Model = Object.Model;
    Type = Object.Type;
    Category = Object.Category;
    PlaneAltitude = Object.PlaneAltitude;
    GndAltitude = Object.GndAltitude;
    PlaneWeight = Object.PlaneWeight;
    Vario = Object.Vario;
    PlaneHeading = Object.PlaneHeading;
    TimeFactor = Object.TimeFactor;
    Longitude = Object.Longitude;
    Latitude = Object.Latitude;
    GndSpeed = Object.GndSpeed;
    TASSpeed = Object.TASSpeed;
    IASSpeed = Object.IASSpeed;
    Fuel = Object.Fuel;
    TotalFuelCapacity = Object.TotalFuelCapacity;
    GForce = Object.GForce;
    PlanePitch = Object.PlanePitch;
    PlaneBank = Object.PlaneBank;
    AmbiantWindVelocity = Object.AmbiantWindDirection;
    AmbiantWindDirection = Object.AmbiantWindDirection;
    AltimeterSetting = Object.AltimeterSetting;
    SeaLevelPressure = Object.SeaLevelPressure;
    XAxis = Object.XAxis;
    YAxis = Object.YAxis;
    Slider = Object.Slider;
    RZAxis = Object.RZAxis;
    HatAxis = Object.HatAxis;
    Squawk = Object.Squawk;
    Realism = Object.Realism;
    AmbiantPrecipState = Object.AmbiantPrecipState;
    ElevatorPos = Object.ElevatorPos;
    AileronPos = Object.AileronPos;
    RudderPos = Object.RudderPos;
    Door1Pos = Object.Door1Pos;
    Door2Pos = Object.Door2Pos;
    Door3Pos = Object.Door3Pos;
    Door4Pos = Object.Door4Pos;
    FlapsIndex = Object.FlapsIndex;
    NumEngine = Object.NumEngine;
    ThrottleEng1 = Object.ThrottleEng1;
    ThrottleEng2 = Object.ThrottleEng2;
    ThrottleEng3 = Object.ThrottleEng3;
    ThrottleEng4 = Object.ThrottleEng4;

    //Etat
    ParkingBrake = Object.ParkingBrake;
    Pause = Object.Pause;
    Crash = Object.Crash;
    RunState = Object.RunState;
    Pushback = Object.Pushback;
    OverSpeed = Object.OverSpeed;
    Stalling = Object.Stalling;
    Slew = Object.Slew;
    OnGround = Object.OnGround;
    GearPos = Object.GearPos;
    LandingLight = Object.LandingLight;
    BeaconLight = Object.BeaconLight;
    StrobeLight = Object.StrobeLight;
    NavLight = Object.NavLight;
    RecoLight = Object.RecoLight;
    StateEng1 = Object.StateEng1;
    StateEng2 = Object.StateEng2;
    StateEng3 = Object.StateEng3;
    StateEng4 = Object.StateEng4;
    Smoke = Object.Smoke;
}

CAircraftState::~CAircraftState()
{

}

void CAircraftState::OnSCReceiveVariable(SCManagerEventSCVariable pData)
{
    TimeStamp = pData.Time;
    Title = pData.Data.Title;
    Model = pData.Data.Model;
    Type = pData.Data.Type;
    Category = pData.Data.Category;
    PlaneAltitude = pData.Data.PlaneAltitude;
    GndAltitude = pData.Data.GndAltitude;
    PlaneWeight = pData.Data.PlaneWeight;
    Vario = pData.Data.Vario;
    PlaneHeading = pData.Data.PlaneHeading;
    TimeFactor = pData.Data.TimeFactor;
    Longitude = pData.Data.Longitude;
    Latitude = pData.Data.Latitude;
    GndSpeed = pData.Data.GndSpeed;
    TASSpeed = pData.Data.TASSpeed;
    IASSpeed = pData.Data.IASSpeed;
    Fuel = pData.Data.FuelQty;
    TotalFuelCapacity = pData.Data.TotalFuelCapacity;
    GForce = pData.Data.GForce;
    PlanePitch = pData.Data.PlanePitch;
    PlaneBank = pData.Data.PlaneBank;
    AmbiantWindVelocity = pData.Data.AmbiantWindDirection;
    AmbiantWindDirection = pData.Data.AmbiantWindDirection;
    SeaLevelPressure = pData.Data.SeaLevelPressure;
    Squawk = pData.Data.Squawk;
    Realism = pData.Data.Realism;
    AmbiantPrecipState = pData.Data.AmbiantPrecipState;
    ElevatorPos = pData.Data.ElevatorPos;
    AileronPos = pData.Data.AileronPos;
    RudderPos = pData.Data.RudderPos;
    Door1Pos = pData.Data.Door1Pos;
    Door2Pos = pData.Data.Door2Pos;
    Door3Pos = pData.Data.Door3Pos;
    Door4Pos = pData.Data.Door4Pos;
    NumEngine = pData.Data.NumEngine;
    ThrottleEng1 = pData.Data.ThrottleEng1;
    ThrottleEng2 = pData.Data.ThrottleEng2;
    ThrottleEng3 = pData.Data.ThrottleEng3;
    ThrottleEng4 = pData.Data.ThrottleEng4;
    StateEng1 = pData.Data.StateEng1;
    StateEng2 = pData.Data.StateEng2;
    StateEng3 = pData.Data.StateEng3;
    StateEng4 = pData.Data.StateEng4;
}

void CAircraftState::OnSCReceiveEvent(SCManagerEventSCEvent pData)
{
    switch (pData.Evt_Id)
    {
        case EVENT_ID::PARKING_BRAKE:
        {
            ParkingBrake = (bool)pData.Data;
            break;
        }
        /*case EVENT_ID::OVERSPEED:
        {
            OverSpeed = (bool)pData.Data;
            break;
        }
        case EVENT_ID::STALLING:
        {
            Stalling = (bool)pData.Data;
            break;
        }
        case EVENT_ID::CRASH:
        {
            Crash = (bool)pData.Data;
            break;
        }
        case EVENT_ID::SLEW:
        {
            Slew = (bool)pData.Data;
            break;
        }
        case EVENT_ID::PUSHBACK:
        {
            Pushback = (bool)pData.Data;
            break;
        }
        case EVENT_ID::ON_GROUND:
        {
            if ((pData.Data >=0) && (pData.Data <=1))
                OnGround = (bool)pData.Data;
            break;
        }
        case EVENT_ID::SIMSTART:
        {
            RunState = true;
            break;
        }
        case EVENT_ID::SIMSTOP:
        {
            RunState = false;
            break;
        }
        case EVENT_ID::PAUSE:
        {
            if ((pData.Data >=0) && (pData.Data <=1))
                Pause = (bool)pData.Data;
            break;
        }
        case EVENT_ID::FLAPS_HANDLE_INDEX:
        {
            FlapsIndex = pData.Data;
            break;
        }
        case EVENT_ID::GEAR_HANDLE_POSITION:
        {
            GearPos = pData.Data;
            break;
        }
        case EVENT_ID::ALTIMETER_SETTING:
        {
            AltimeterSetting = pData.Data;
            break;
        }
        case EVENT_ID::LANDING_LIGHT:
        {
            LandingLight = (bool)pData.Data;
            break;
        }
        case EVENT_ID::STROBE_LIGHT:
        {
            StrobeLight = (bool)pData.Data;
            break;
        }
        case EVENT_ID::BEACON_LIGHT:
        {
            BeaconLight = (bool)pData.Data;
            break;
        }
        case EVENT_ID::NAV_LIGHT:
        {
            NavLight = (bool)pData.Data;
            break;
        }
        case EVENT_ID::RECOGNITION_LIGHT:
        {
            RecoLight = (bool)pData.Data;
            break;
        }
        case EVENT_ID::SMOKE_ENABLE:
        {
            Smoke = (bool)pData.Data;
            break;
        }*/
        default:
        {
            break;
        }
    }
}
