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

#include "sim/aircraftstate.h"

CAircraftState::CAircraftState ()
{
    TimeStamp = Now();
    Title = "";
    Model = "";
    Type = "";
    Category = "";
    GroundAltitude = 0;
    Weight = 0;
    Vario = 0;
    TimeFactor = 1;
    GroundSpeed = 0;
    TASSpeed = 0;
    IASSpeed = 0;
    Fuel = 0;
    TotalFuelCapacity = 0;
    GForce = 0;
    Pitch = 0;
    Bank = 0;
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
    NumEngine = 0;
}

CAircraftState::~CAircraftState()
{

}

void CAircraftState::toAirData(AirData &data)
{
    CAIBase::toAirData(data);
    data.set_groundaltitude(GroundAltitude);
    data.set_category(Category.toStdString());
    data.set_iasspeed(IASSpeed);
    data.set_model(Model.toStdString());
    data.set_numengine(NumEngine);
    data.set_onground(OnGround);
    data.set_timestamp(TimeStamp.time_since_epoch().count());
    data.set_title(Title.toStdString());
    data.set_type(Type.toStdString());

}

void CAircraftState::fromAirData(const AirData &data)
{
    CAIBase::fromAirData(data);
    GroundAltitude = data.groundaltitude();
    Category = QString::fromStdString(data.category());
    IASSpeed = data.iasspeed();
    Model = QString::fromStdString(data.model());
    NumEngine = data.numengine();
    OnGround = data.onground();
    TimeStamp = fromTimeStamp(data.timestamp());
    Title = QString::fromStdString(data.title());
    Type = QString::fromStdString(data.type());
}
