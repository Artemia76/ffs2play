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

namespace ffs2play
{

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
		data.set_timestamp(TimeStamp.time_since_epoch().count());
		data.set_title(Title.toStdString());
		data.set_model(Model.toStdString());
		data.set_type(Type.toStdString());
		data.set_category(Category.toStdString());
		data.set_latitude(Latitude);
		data.set_longitude(Longitude);
		data.set_altitude(Altitude);
		data.set_pitch(Pitch);
		data.set_bank(Bank);
		data.set_heading(Heading);
		data.set_elevatorpos(ElevatorPos);
		data.set_aileronpos(AileronPos);
		data.set_rudderpos(RudderPos);
		data.set_groundaltitude(GroundAltitude);
		data.set_iasspeed(IASSpeed);
		data.set_squawk(Squawk);
		data.set_numengine(NumEngine);
		data.set_door1pos(Door1Pos);
		data.set_door2pos(Door2Pos);
		data.set_door3pos(Door3Pos);
		data.set_door4pos(Door4Pos);
		data.set_stateeng1(StateEng1);
		data.set_stateeng2(StateEng2);
		data.set_stateeng3(StateEng3);
		data.set_stateeng4(StateEng4);
		data.set_throttleeng1(ThrottleEng1);
		data.set_throttleeng2(ThrottleEng2);
		data.set_throttleeng3(ThrottleEng3);
		data.set_throttleeng4(ThrottleEng4);
		data.set_gearpos(GearPos);
		data.set_flapsindex(FlapsIndex);
		data.set_landinglight(LandingLight);
		data.set_strobelight(StrobeLight);
		data.set_beaconlight(BeaconLight);
		data.set_navlight(NavLight);
		data.set_recolight(RecoLight);
		data.set_smoke(Smoke);
		data.set_onground(OnGround);
		data.set_gndspeed(GroundSpeed);
		data.set_tasspeed(TASSpeed);
	}

    ///
    /// \brief CAircraftState::fromAirData
    /// \param data
    ///
	void CAircraftState::fromAirData(const AirData &data)
	{
		TimeStamp = fromTimeStamp(data.timestamp());
		Title = QString::fromStdString(data.title());
		Model = QString::fromStdString(data.model());
		Type = QString::fromStdString(data.type());
		Category = QString::fromStdString(data.category());
		Latitude = data.latitude();
		Longitude= data.longitude();
		Altitude = data.altitude();
		Pitch = data.pitch();
		Bank = data.bank();
		Heading = data.heading();
		ElevatorPos = data.elevatorpos();
		AileronPos = data.aileronpos();
		RudderPos = data.rudderpos();
		GroundAltitude = data.groundaltitude();
		IASSpeed = data.iasspeed();
		Squawk = data.squawk();
		NumEngine = data.numengine();
		Door1Pos = data.door1pos();
		Door2Pos = data.door2pos();
		Door3Pos = data.door3pos();
		Door4Pos = data.door4pos();
		StateEng1 = data.stateeng1();
		StateEng2 = data.stateeng2();
		StateEng3 = data.stateeng3();
		StateEng4 = data.stateeng4();
		ThrottleEng1 = data.throttleeng1();
		ThrottleEng2 = data.throttleeng2();
		ThrottleEng3 = data.throttleeng3();
		ThrottleEng4 = data.throttleeng4();
		GearPos = data.gearpos();
		FlapsIndex = data.flapsindex();
		LandingLight = data.landinglight();
		StrobeLight = data.strobelight();
		BeaconLight = data.beaconlight();
		NavLight = data.navlight();
		RecoLight = data.recolight();
		Smoke = data.smoke();
		OnGround = data.onground();
		TASSpeed = data.tasspeed();
		GroundSpeed = data.gndspeed();
	}
}
