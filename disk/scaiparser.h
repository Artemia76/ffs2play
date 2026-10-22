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
** scaiparser.h is part of FF2Play project
**
** This class provide method to parse FSX like AI files
****************************************************************************/

#ifndef SCAIPARSER_H
#define SCAIPARSER_H

#include "disk/aibaseparser.h"

class SCAIParser : public AIBaseParser
{
public:
    SCAIParser( QObject *parent = nullptr);
    ~SCAIParser() override;

protected:
    void    searchSimPath() override;
    QString findInstallationPath (const QString& pKey,QSettings::Format pFormat = QSettings::NativeFormat) override;
    void    DBFileUpdate(const QString& pKey,const QString& pPath) override;
    QString findMSFS2020Packages();
};

#endif // SCAIPARSER_H
