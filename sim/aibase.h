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

#ifndef AIBASE_H
#define AIBASE_H

#include "protobuf/ffs2play.pb.h"

using namespace ffs2play;

class CAIBase
{
public:
    CAIBase();
    ~CAIBase();
    virtual void toAirData (AirData& data);
    virtual void fromAirData (const AirData& data);
	double Latitude;
	double Longitude;
	double Altitude;
	double Pitch;
	double Bank;
	double Heading;
	double ElevatorPos;
	double AileronPos;
	double RudderPos;
	int Door1Pos;
	int Door2Pos;
	int Door3Pos;
	int Door4Pos;
	int StateEng1;
	int StateEng2;
	int StateEng3;
	int StateEng4;
	int ThrottleEng1;
	int ThrottleEng2;
	int ThrottleEng3;
	int ThrottleEng4;
	int Squawk;
	int GearPos;
	int FlapsIndex;
	int LandingLight;
	int StrobeLight;
	int BeaconLight;
	int NavLight;
	int RecoLight;
	int Smoke;
};

#endif // AIRCRAFTSTATE_H
