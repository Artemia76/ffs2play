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

#include "calculation.h"
#include <QDataStream>

namespace ffs2play
{
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
		if (std::isnan(dist)) dist = 0;
		return (dist);
	}

	double deg2rad(double deg)
	{
		return (deg * M_PI  / 180.0);
	}

	double rad2deg(double rad)
	{
		return (rad / M_PI * 180.0);
	}

	double Average(QVector<double> Array)
	{
		double total = 0;
		for (double Value : Array) total += Value;
		return total / (double)Array.count();
	}

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
			quint8 bcdByte = bytes[i];
			int idHigh = (bcdByte >> 4) & 0X07;
			int idLow = bcdByte & 0x07;
			QString High, Low;
			High.setNum(idHigh,16);
			Low.setNum(idLow,16);
			bcd += High + Low;
		}
		return bcd.toInt();
	}
}
