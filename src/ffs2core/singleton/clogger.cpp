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

#include "singleton/clogger.h"
#include "singleton/singleton.h"

///
/// \brief CLogger::CLogger
/// \param parent
///

CLogger::CLogger(QObject* parent):
    QObject(parent)
{
    qRegisterMetaType<CL_DEBUG_LEVEL>("CL_DEBUG_LEVEL");
    s.beginGroup("Log");
    m_DebugLevel = (CL_DEBUG_LEVEL)s.value("DebugLevel",LEVEL_NONE).toLongLong();
    m_logToFile = s.value("LogToFile",false).toBool();
    s.endGroup();
}

///
/// \brief CLogger::createInstance
/// \return
///

CLogger* CLogger::createInstance()
{
    return new CLogger();
}

///
/// \brief CLogger::~CLogger
///

CLogger::~CLogger()
{
    s.beginGroup("Log");
    s.setValue("DebugLevel",m_DebugLevel);
    s.setValue("LogToFile",m_logToFile);
    s.endGroup();
}

///
/// \brief CLogger::instance
/// \return
///

CLogger* CLogger::instance()
{
    return Singleton<CLogger>::instance(CLogger::createInstance);
}

///
/// \brief CLogger::setDebugLevel
/// \param pLevel
///

void CLogger::setDebugLevel(CL_DEBUG_LEVEL pLevel)
{
    if (pLevel < LEVEL_NONE ) m_DebugLevel=LEVEL_NONE;
    else if (pLevel > LEVEL_VERBOSE) m_DebugLevel=LEVEL_VERBOSE;
    else m_DebugLevel = pLevel;
    s.beginGroup("Log");
    s.setValue("DebugLevel",QVariant(m_DebugLevel));
    s.endGroup();
}

///
/// \brief CLogger::log
/// \param Texte
/// \param pColor
/// \param pLevel
///

void CLogger::log (const QString& Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    if (pLevel > m_DebugLevel) return;
    emit(fireLog(Texte,pColor,pLevel));
}
