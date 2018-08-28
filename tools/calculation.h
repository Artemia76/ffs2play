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
 * calculation.h is part of FF2Play project
 *
 * These functions provide mathematicals and binary tools for convertions
 * **************************************************************************/

#ifndef CALCULATION_H
#define CALCULATION_H

#define _USE_MATH_DEFINES
#include<cmath>
#include<QVector>

#define _EQURAD 6378137.0
#define _FLATTENING 298.257223563


/** Feet to Meters */
#define SG_FEET_TO_METER    0.3048

/** Meters to Feet */
#define SG_METER_TO_FEET    3.28083989501312335958

const float SG_PI = 3.1415926535f;
const double SGD_PI = 3.1415926535;
const double SGD_2PI = SGD_PI * 2.0;
const double SGD_PI_4 = 0.78539816339744830961;

const double SGD_DEGREES_TO_RADIANS = SGD_PI / 180.0;
const double SGD_RADIANS_TO_DEGREES = 180.0 / SGD_PI;

const float SG_DEGREES_TO_RADIANS = SG_PI / 180.0f;
const float SG_RADIANS_TO_DEGREES = 180.0f / SG_PI;

// These are derived quantities more useful to the code:
#if 0
    #define _SQUASH (1 - 1/_FLATTENING)
    #define _STRETCH (1/_SQUASH)
    #define _POLRAD (EQURAD * _SQUASH)
#else
    // High-precision versions of the above produced with an arbitrary
    // precision calculator (the compiler might lose a few bits in the FPU
    // operations).  These are specified to 81 bits of mantissa, which is
    // higher than any FPU known to me:
    #define _SQUASH    0.9966471893352525192801545
    #define _STRETCH   1.0033640898209764189003079
    #define _POLRAD    6356752.3142451794975639668
#endif

// additional derived and precomputable ones
// for the geodetic conversion algorithm

const double EQURAD = _EQURAD;
const double iFLATTENING = _FLATTENING;
const double SQUASH = _SQUASH;
const double STRETCH = _STRETCH;
const double POLRAD = _POLRAD;

#define E2 fabs(1 - _SQUASH*_SQUASH)
static double a = _EQURAD;
static double ra2 = 1/(_EQURAD*_EQURAD);
//static double e = sqrt(E2);
static double e2 = E2;
static double e4 = E2*E2;

#undef _EQURAD
#undef _FLATTENING
#undef _SQUASH
#undef _STRETCH
#undef _POLRAD
#undef E2


namespace ffs2play
{
    double distance(double lat1, double lon1, double lat2, double lon2, char unit);
    double deg2rad(double deg);
    double rad2deg(double rad);
    double Average(QVector<double>& Array);
    int ConvertToBinaryCodedDecimal(int Code);
    void CartToGeod(const double pX, const double pY, const double pZ, double& pLon, double& pLat, double& pAlt);
    void GeodToCart(const double pLon, const double pLat, const double pAlt, double& pX, double& pY, double& pZ);
}

#endif
