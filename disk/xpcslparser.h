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
** xpcslparser.h is part of FF2Play project
**
** This class provide method to parse X-Plane CSL files
****************************************************************************/

#ifndef XPCSLPARSER_H
#define XPCSLPARSER_H

#include "disk/aibaseparser.h"
#include <QMap>
#include <QVector>

struct CSL
{
    QString AirLine;
    QString ICAO;
    double Offset;
    bool HasGear;
    int Livery;
};

struct xsb_aircraft
{
    QString Export_Name;
    QVector<CSL> ArrCSL;
};

class XPCSLParser : public AIBaseParser
{
public:
    XPCSLParser(QObject *parent = nullptr);
    ~XPCSLParser() override;
    static QStringList findXPlaneRoots();
private:
    void    searchSimPath() override;
    static bool hasCSLPackage(const QString& pPath);
    void    DBFileUpdate(const QString& pKey,const QString& pPath) override;
    void    ReadCSL (const QString& pPath,  xsb_aircraft& pContent);
};

#endif // XPCSLPARSER_H
