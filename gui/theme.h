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
** theme.h is part of FF2Play project
**
** This singleton applies the light or dark graphic theme (palette and
** style sheet) and follows the system color scheme when asked to
****************************************************************************/

#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QColor>
#include <QSettings>

class QMenu;

class CTheme : public QObject
{
    Q_OBJECT
public:
    enum Mode
    {
        System,
        Light,
        Dark
    };

    static CTheme&  instance();
    void            init();
    Mode            mode() const { return m_mode; }
    void            setMode(Mode pMode);
    bool            isDark() const { return m_dark; }
    QColor          logColor(const QColor& pColor) const;
    QMenu*          createMenu(QWidget* pParent);

signals:
    void            fireChanged(bool pDark);

private:
    explicit        CTheme();
    static CTheme*  createInstance();
    void            apply();
    void            onColorSchemeChanged();

    QSettings       m_settings;
    Mode            m_mode;
    bool            m_dark;
    bool            m_applying;
};

#endif // THEME_H
