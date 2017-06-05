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

#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include <QSettings>
#include <QColor>
#include "tools/ffs2core_global.h"

enum FFS2CORE_EXPORT CL_DEBUG_LEVEL
{
    LEVEL_NONE,
    LEVEL_NORMAL,
    LEVEL_VERBOSE
};

class FFS2CORE_EXPORT CLogger : public QObject
{
    Q_OBJECT

private:
                    CLogger(QObject* parent = 0);
    static CLogger* createInstance();
    CL_DEBUG_LEVEL  m_DebugLevel;
    QSettings       s;
    bool            m_logToFile;

public:
                    ~CLogger();
    static CLogger* instance();
    void            setDebugLevel (CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
    void            log (const QString& Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);

signals:
    void            fireLog (QString Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
};

#endif // CLOGGER_H
