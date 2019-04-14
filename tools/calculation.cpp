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
** calculation.cpp is part of FF2Play project
**
** These functions provide mathematicals and binary tools for convertions
****************************************************************************/

#include "calculation.h"
#include <QDataStream>
#include <limits>

using namespace std;

///
/// \brief distance
/// \param lat1
/// \param lon1
/// \param lat2
/// \param lon2
/// \param unit
/// \return
///
double distance(double lat1, double lon1, double lat2, double lon2, char unit)
{
    double theta = lon1 - lon2;
    double dist = std::sin(deg2rad(lat1)) * std::sin(deg2rad(lat2)) + std::cos(deg2rad(lat1)) * std::cos(deg2rad(lat2)) * std::cos(deg2rad(theta));
    dist = std::acos(dist);
    dist = rad2deg(dist);
    dist = dist * 60 * 1.1515;
    if (unit == 'K')
    {
        dist = dist * 1.609344;
    }
    else if (unit == 'N')
    {
        dist = dist * 0.8684;
    }
    if (isnan(dist)) dist = 0;
    return (dist);
}

///
/// \brief deg2rad
/// \param deg
/// \return
///
double deg2rad(double deg)
{
    return (deg * M_PI  / 180.0);
}

///
/// \brief rad2deg
/// \param rad
/// \return
///
double rad2deg(double rad)
{
    return (rad / M_PI * 180.0);
}

///
/// \brief Average
/// \param Array
/// \return
///
double Average(QVector<double> Array)
{
    double total = 0;
    for (double Value : Array) total += Value;
    return total / static_cast<double>(Array.count());
}

///
/// \brief ConvertToBinaryCodedDecimal
/// \param Code
/// \return
///
int ConvertToBinaryCodedDecimal(int Code)
{
    QByteArray bytes;
    QDataStream stream(&bytes,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << Code;
    int Length = bytes.length();
    if (Length > 2) Length = 2;
    QString bcd;
    for (int i = Length - 1; i >= 0; i--)
    {
        quint8 bcdByte = static_cast<quint8>(bytes[i]);
        int idHigh = (bcdByte >> 4) & 0X07;
        int idLow = bcdByte & 0x07;
        QString High, Low;
        High.setNum(idHigh,16);
        Low.setNum(idLow,16);
        bcd += High + Low;
    }
    return bcd.toInt();
}

void CartToGeod(const double pX, const double pY, const double pZ, double& pLon, double& pLat, double& pAlt)
{
  double XXpYY = pX*pX+pY*pY;
  if( XXpYY + pZ*pZ < 25 ) {
    pLon= 0.0;
    pLat= 0.0;
    pAlt= ( -EQURAD );
    return;
  }

  double sqrtXXpYY = sqrt(XXpYY);
  double p = XXpYY*ra2;
  double q = pZ*pZ*(1-e2)*ra2;
  double r = 1/6.0*(p+q-e4);
  double s = e4*p*q/(4*r*r*r);

  if( s >= -2.0 && s <= 0.0 )
    s = 0.0;
  double t = pow(1+s+sqrt(s*(2+s)), 1/3.0);
  double u = r*(1+t+1/t);
  double v = sqrt(u*u+e4*q);
  double w = e2*(u+v-q)/(2*v);
  double k = sqrt(u+v+w*w)-w;
  double D = k*sqrtXXpYY/(k+e2);
  pLon = 2*atan2(pY, pX+sqrtXXpYY)*SGD_RADIANS_TO_DEGREES;
  double sqrtDDpZZ = sqrt(D*D+pZ*pZ);
  pLat = 2*atan2(pZ, D+sqrtDDpZZ)*SGD_RADIANS_TO_DEGREES;
  pAlt =(k+e2-1)*sqrtDDpZZ/k* SG_METER_TO_FEET;
}

void GeodToCart(const double pLon, const double pLat, const double pAlt, double& pX, double& pY, double& pZ)
{
    double lambda = pLon * SGD_DEGREES_TO_RADIANS;
    double phi = pLat * SGD_DEGREES_TO_RADIANS;
    double h = pAlt * SG_FEET_TO_METER;
    double sphi = sin(phi);
    double n = a/sqrt(1-e2*sphi*sphi);
    double cphi = cos(phi);
    double slambda = sin(lambda);
    double clambda = cos(lambda);
    pX = (h+n)*cphi*clambda;
    pY = (h+n)*cphi*slambda;
    pZ = (h+n-e2*n)*sphi;
}

void EulerToAngle(const float pX,  const float pY, const float /*pZ*/, float& pYaw, float& pPitch, float& pRoll )
{
    pPitch = asin(pX / cos(asin(pY)));
    pYaw = asin(pY);
    pRoll = 0;
}
