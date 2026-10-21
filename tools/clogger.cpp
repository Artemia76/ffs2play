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
** clogger.cpp is part of FF2Play project
**
** This singleton provide log messaging manager
****************************************************************************/

#include "tools/clogger.h"
#include "tools/singleton.h"

///
/// \brief CLogger::CLogger
/// \param parent
///
CLogger::CLogger(QObject* parent):
    QObject(parent)
{
    qRegisterMetaType<CL_DEBUG_LEVEL>("CL_DEBUG_LEVEL");
    m_settings.beginGroup("Log");
    m_DebugLevel = static_cast<CL_DEBUG_LEVEL>(m_settings.value("DebugLevel",LEVEL_NONE).toLongLong());
    m_logToFile = m_settings.value("LogToFile",false).toBool();
    m_filtreLog = m_settings.value("FiltreLog","*").toString();
    m_settings.endGroup();
    updatePattern();
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
    m_settings.beginGroup("Log");
    m_settings.setValue("DebugLevel",m_DebugLevel);
    m_settings.setValue("LogToFile",m_logToFile);
    m_settings.endGroup();
}

///
/// \brief CLogger::instance
/// \return
///
CLogger& CLogger::instance()
{
    return Singleton<CLogger>::instance(CLogger::createInstance);
}

///
/// \brief CLogger::setDebugLevel
/// \param pLevel
///
void CLogger::setDebugLevel(CL_DEBUG_LEVEL pLevel)
{
    QMutexLocker locker(&m_mutex);
    if (pLevel < LEVEL_NONE ) m_DebugLevel=LEVEL_NONE;
    else if (pLevel > LEVEL_VERBOSE) m_DebugLevel=LEVEL_VERBOSE;
    else m_DebugLevel = pLevel;
    m_settings.beginGroup("Log");
    m_settings.setValue("DebugLevel",QVariant(m_DebugLevel));
    m_settings.endGroup();
}

///
/// \brief CLogger::log
/// \param Texte
/// \param pColor
/// \param pLevel
///
void CLogger::log (const QString& Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    {
        QMutexLocker locker(&m_mutex);
        if ((pLevel!=LEVEL_NONE) && ((pLevel > m_DebugLevel) || !m_rx.match(Texte).hasMatch())) return;
    }
    // Emitted without the lock : a slot may log again
    emit fireLog(Texte,pColor,pLevel);
}

///
/// \brief CLogger::updatePattern
///
void CLogger::updatePattern()
{
    m_rx.setPattern(QRegularExpression::wildcardToRegularExpression(
        m_filtreLog, QRegularExpression::NonPathWildcardConversion));
}

///
/// \brief CLogger::setFilter
/// \param pFilter
///
void CLogger::setFilter(const QString& pFilter)
{
    QMutexLocker locker(&m_mutex);
    m_filtreLog=pFilter;
    updatePattern();
    m_settings.beginGroup("Log");
    m_settings.setValue("FiltreLog",m_filtreLog);
    m_settings.endGroup();
}

///
/// \brief CLogger::getFilter
/// \return
///
QString CLogger::getFilter()
{
    QMutexLocker locker(&m_mutex);
    return m_filtreLog;
}

///
/// \brief CLogger::debugLevel
/// \return current debug level
///
CL_DEBUG_LEVEL CLogger::debugLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_DebugLevel;
}
