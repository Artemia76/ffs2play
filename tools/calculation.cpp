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
#include <QIODevice>
#include <algorithm>
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

namespace
{
///
/// \brief Quaternion with the conventions of SimGear (simgear/math/SGQuat.hxx)
///
struct SGQuat
{
    double w, x, y, z;

    SGQuat operator*(const SGQuat& v) const
    {
        return { w*v.w - x*v.x - y*v.y - z*v.z,
                 w*v.x + x*v.w + y*v.z - z*v.y,
                 w*v.y - x*v.z + y*v.w + z*v.x,
                 w*v.z + x*v.y - y*v.x + z*v.w };
    }
    SGQuat conj() const { return { w, -x, -y, -z }; }

    /// Rotation from the ECEF frame to the local horizontal frame
    static SGQuat fromLonLatRad(double pLon, double pLat)
    {
        const double zd2 = 0.5*pLon;
        const double yd2 = -0.25*SGD_PI - 0.5*pLat;
        const double Szd2 = sin(zd2), Czd2 = cos(zd2);
        const double Syd2 = sin(yd2), Cyd2 = cos(yd2);
        return { Czd2*Cyd2, -Szd2*Syd2, Czd2*Syd2, Szd2*Cyd2 };
    }
    static SGQuat fromYawPitchRollRad(double pZ, double pY, double pX)
    {
        const double Szd2 = sin(0.5*pZ), Czd2 = cos(0.5*pZ);
        const double Syd2 = sin(0.5*pY), Cyd2 = cos(0.5*pY);
        const double Sxd2 = sin(0.5*pX), Cxd2 = cos(0.5*pX);
        const double cxd2czd2 = Cxd2*Czd2, cxd2szd2 = Cxd2*Szd2;
        const double sxd2szd2 = Sxd2*Szd2, sxd2czd2 = Sxd2*Czd2;
        return { cxd2czd2*Cyd2 + sxd2szd2*Syd2,
                 sxd2czd2*Cyd2 - cxd2szd2*Syd2,
                 cxd2czd2*Syd2 + sxd2szd2*Cyd2,
                 cxd2szd2*Cyd2 - sxd2czd2*Syd2 };
    }
    /// \param pAxis rotation axis multiplied by the angle (radians)
    static SGQuat fromAngleAxis(const double pAxis[3])
    {
        const double Angle = sqrt(pAxis[0]*pAxis[0] + pAxis[1]*pAxis[1] + pAxis[2]*pAxis[2]);
        if (Angle <= std::numeric_limits<double>::min()) return { 1, 0, 0, 0 };
        const double S = sin(0.5*Angle)/Angle;
        return { cos(0.5*Angle), S*pAxis[0], S*pAxis[1], S*pAxis[2] };
    }
    void getAngleAxis(double pAxis[3]) const
    {
        const double Nrm = sqrt(w*w + x*x + y*y + z*z);
        if (Nrm <= std::numeric_limits<double>::min())
        {
            pAxis[0] = pAxis[1] = pAxis[2] = 0;
            return;
        }
        const double rNrm = 1/Nrm;
        const double Angle = acos(std::max(-1.0, std::min(1.0, rNrm*w)));
        const double sAng = sin(Angle);
        if (fabs(sAng) <= std::numeric_limits<double>::min())
        {
            pAxis[0] = 2*Angle;
            pAxis[1] = pAxis[2] = 0;
            return;
        }
        const double Coef = 2*Angle*rNrm/sAng;
        pAxis[0] = Coef*x;
        pAxis[1] = Coef*y;
        pAxis[2] = Coef*z;
    }
    void getEulerRad(double& pZ, double& pY, double& pX) const
    {
        const double sqrQW = w*w, sqrQX = x*x, sqrQY = y*y, sqrQZ = z*z;
        double num = 2*(y*z + w*x);
        double den = sqrQW - sqrQX - sqrQY + sqrQZ;
        pX = ((fabs(den) <= std::numeric_limits<double>::min()) && (fabs(num) <= std::numeric_limits<double>::min())) ? 0 : atan2(num, den);
        const double tmp = 2*(x*z - w*y);
        if (tmp <= -1) pY = 0.5*SGD_PI;
        else if (1 <= tmp) pY = -0.5*SGD_PI;
        else pY = -asin(tmp);
        num = 2*(x*y + w*z);
        den = sqrQW + sqrQX - sqrQY - sqrQZ;
        if ((fabs(den) <= std::numeric_limits<double>::min()) && (fabs(num) <= std::numeric_limits<double>::min())) pZ = 0;
        else
        {
            pZ = atan2(num, den);
            if (pZ < 0) pZ += SGD_2PI;
        }
    }
};
}

///
/// \brief FGOrientationToEuler
/// Attitude of a FlightGear multiplayer position message
/// \param pLon , pLat position (degrees)
/// \param pAngleAxis orientation of the body in the ECEF frame (angle axis, radians)
/// \param pHeading true heading (degrees)
/// \param pPitch , pBank ffs2play convention (SimConnect) : positive nose down, left wing down (degrees)
///
void FGOrientationToEuler(double pLon, double pLat, const float pAngleAxis[3], double& pHeading, double& pPitch, double& pBank)
{
    const double Axis[3] = { pAngleAxis[0], pAngleAxis[1], pAngleAxis[2] };
    // FlightGear sends qEc2Hl * hlOr : the orientation in the local horizontal frame is qEc2Hl^-1 * q
    const SGQuat HlOr = SGQuat::fromLonLatRad(pLon*SGD_DEGREES_TO_RADIANS, pLat*SGD_DEGREES_TO_RADIANS).conj() * SGQuat::fromAngleAxis(Axis);
    double Yaw, Pitch, Roll;
    HlOr.getEulerRad(Yaw, Pitch, Roll);
    pHeading = Yaw*SGD_RADIANS_TO_DEGREES;
    pPitch = -Pitch*SGD_RADIANS_TO_DEGREES;
    pBank = -Roll*SGD_RADIANS_TO_DEGREES;
}

///
/// \brief EulerToFGOrientation
/// Orientation of a FlightGear multiplayer position message
/// \param pLon , pLat position (degrees)
/// \param pHeading true heading (degrees)
/// \param pPitch , pBank ffs2play convention (SimConnect) : positive nose down, left wing down (degrees)
/// \param pAngleAxis orientation of the body in the ECEF frame (angle axis, radians)
///
void EulerToFGOrientation(double pLon, double pLat, double pHeading, double pPitch, double pBank, float pAngleAxis[3])
{
    const SGQuat Q = SGQuat::fromLonLatRad(pLon*SGD_DEGREES_TO_RADIANS, pLat*SGD_DEGREES_TO_RADIANS)
                   * SGQuat::fromYawPitchRollRad(pHeading*SGD_DEGREES_TO_RADIANS, -pPitch*SGD_DEGREES_TO_RADIANS, -pBank*SGD_DEGREES_TO_RADIANS);
    double Axis[3];
    Q.getAngleAxis(Axis);
    for (int i = 0; i < 3; i++) pAngleAxis[i] = static_cast<float>(Axis[i]);
}
