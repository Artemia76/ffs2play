/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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
** iniparser.h is part of FF2Play project
**
** This clas provide ini based configuration file parser
****************************************************************************/

#ifndef INIPARSER_H
#define INIPARSER_H

#include <QObject>
#include <QFile>
#include <QVariant>
#include <QMap>
#include "sim/simbasemanager.h"

class IniParserException : public std::exception
{
public:
    QString Message;
};

class IniParser : public QObject
{
    Q_OBJECT
public:
                                IniParser(const QString& pFilename, const SIM_TYPE pSimType = SIM_TYPE::FSX);
virtual                         ~IniParser();
    QVariant                    value(const QString& pGroup, const QString& pParameter, const QVariant& pDefaultValue = QVariant()) const;
private:
    SIM_TYPE                    m_simType;
    QString                     m_Selected_Group;
    QMap<QString,QStringList>   m_GroupMap;
};

#endif // INIPARSER_H
