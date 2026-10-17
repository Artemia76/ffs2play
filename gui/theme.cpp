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
** theme.cpp is part of FF2Play project
**
** This singleton applies the light or dark graphic theme (palette and
** style sheet) and follows the system color scheme when asked to
****************************************************************************/

#include "gui/theme.h"
#include "tools/singleton.h"

#include <QActionGroup>
#include <QApplication>
#include <QFile>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyleFactory>
#include <QStyleHints>

namespace
{
    /// Perceived brightness (0..255)
    int luma(const QColor& pColor)
    {
        return (299 * pColor.red() + 587 * pColor.green() + 114 * pColor.blue()) / 1000;
    }

    QPalette lightPalette()
    {
        QPalette p;
        p.setColor(QPalette::Window, QColor("#BDE3F8"));
        p.setColor(QPalette::WindowText, QColor("#13293D"));
        p.setColor(QPalette::Base, Qt::white);
        p.setColor(QPalette::AlternateBase, QColor("#EEF6FC"));
        p.setColor(QPalette::Text, QColor("#13293D"));
        p.setColor(QPalette::Button, QColor("#E6EEF4"));
        p.setColor(QPalette::ButtonText, QColor("#13293D"));
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Highlight, QColor("#2F7FC1"));
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::ToolTipBase, QColor("#FFFBE6"));
        p.setColor(QPalette::ToolTipText, QColor("#13293D"));
        p.setColor(QPalette::PlaceholderText, QColor("#7A8A96"));
        p.setColor(QPalette::Link, QColor("#1A5FA0"));
        p.setColor(QPalette::Light, Qt::white);
        p.setColor(QPalette::Midlight, QColor("#E3EDF4"));
        p.setColor(QPalette::Mid, QColor("#A9C3D6"));
        p.setColor(QPalette::Dark, QColor("#7F98AB"));
        p.setColor(QPalette::Shadow, QColor("#4A5B69"));
        for (QPalette::ColorRole role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
            p.setColor(QPalette::Disabled, role, QColor("#8A9AA6"));
        return p;
    }

    QPalette darkPalette()
    {
        QPalette p;
        p.setColor(QPalette::Window, QColor("#1F2A33"));
        p.setColor(QPalette::WindowText, QColor("#E3EBF2"));
        p.setColor(QPalette::Base, QColor("#141C22"));
        p.setColor(QPalette::AlternateBase, QColor("#1A242C"));
        p.setColor(QPalette::Text, QColor("#E3EBF2"));
        p.setColor(QPalette::Button, QColor("#2C3A46"));
        p.setColor(QPalette::ButtonText, QColor("#E3EBF2"));
        p.setColor(QPalette::BrightText, QColor("#FF6B6B"));
        p.setColor(QPalette::Highlight, QColor("#3D8FD1"));
        p.setColor(QPalette::HighlightedText, Qt::white);
        p.setColor(QPalette::ToolTipBase, QColor("#2C3A46"));
        p.setColor(QPalette::ToolTipText, QColor("#E3EBF2"));
        p.setColor(QPalette::PlaceholderText, QColor("#7F8F9B"));
        p.setColor(QPalette::Link, QColor("#6FB6EE"));
        p.setColor(QPalette::Light, QColor("#3C4D5B"));
        p.setColor(QPalette::Midlight, QColor("#33434F"));
        p.setColor(QPalette::Mid, QColor("#26333D"));
        p.setColor(QPalette::Dark, QColor("#121A20"));
        p.setColor(QPalette::Shadow, Qt::black);
        for (QPalette::ColorRole role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText})
            p.setColor(QPalette::Disabled, role, QColor("#6B7A86"));
        return p;
    }
}

///
/// \brief CTheme::CTheme
///
CTheme::CTheme()
    : QObject(nullptr), m_mode(System), m_dark(false), m_applying(false)
{
    m_settings.beginGroup("Display");
    int mode = m_settings.value("Theme", System).toInt();
    m_settings.endGroup();
    if (mode >= System && mode <= Dark) m_mode = static_cast<Mode>(mode);
}

///
/// \brief CTheme::createInstance
/// \return
///
CTheme* CTheme::createInstance()
{
    return new CTheme();
}

///
/// \brief CTheme::instance
/// \return
///
CTheme* CTheme::instance()
{
    return Singleton<CTheme>::instance(CTheme::createInstance);
}

///
/// \brief CTheme::init
///
/// Fusion draws every widget from the palette and the style sheet : the
/// rendering no longer depends on the native style nor on its own colors
///
void CTheme::init()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &CTheme::onColorSchemeChanged);
    apply();
}

///
/// \brief CTheme::setMode
/// \param pMode
///
void CTheme::setMode(Mode pMode)
{
    m_mode = pMode;
    m_settings.beginGroup("Display");
    m_settings.setValue("Theme", static_cast<int>(pMode));
    m_settings.endGroup();
    apply();
}

///
/// \brief CTheme::apply
///
void CTheme::apply()
{
    m_applying = true;
    QStyleHints* hints = QGuiApplication::styleHints();
    // Back to the system scheme to read it, then force the chosen one
    // (also gives the right color to the window title bar)
    hints->unsetColorScheme();
    bool dark = (m_mode == Dark) || (m_mode == System && hints->colorScheme() == Qt::ColorScheme::Dark);
    if (m_mode != System)
        hints->setColorScheme(dark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
    m_dark = dark;
    QApplication::setPalette(dark ? darkPalette() : lightPalette());
    QFile styleFile(dark ? ":/style/ffs2play-dark.qss" : ":/style/ffs2play.qss");
    if (styleFile.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    m_applying = false;
    emit fireChanged(dark);
}

///
/// \brief CTheme::onColorSchemeChanged
///
void CTheme::onColorSchemeChanged()
{
    if (!m_applying && m_mode == System)
        apply();
}

///
/// \brief CTheme::logColor
/// \param pColor
/// \return
///
/// Log colors are chosen for a white background : on the dark theme they
/// are lightened, and too bright ones are darkened on the light theme
///
QColor CTheme::logColor(const QColor& pColor) const
{
    if (pColor.saturation() < 30)
        return QApplication::palette().color(QPalette::Text);
    QColor color = pColor.toHsl();
    int h = color.hslHue(), s = color.hslSaturation(), l = color.lightness();
    if (m_dark)
    {
        while (luma(color) < 150 && l < 235)
            color.setHsl(h, s, l += 5);
    }
    else
    {
        while (luma(color) > 130 && l > 20)
            color.setHsl(h, s, l -= 5);
    }
    return color;
}

///
/// \brief CTheme::icon
/// \return
///
/// Toolbar icon : a sphere half day, half night
///
QIcon CTheme::icon() const
{
    QPixmap pixmap(72, 72);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF ball(6, 6, 60, 60);
    QRadialGradient light(QPointF(26, 24), 46);
    light.setColorAt(0, QColor("#FFFFFF"));
    light.setColorAt(1, QColor("#9CC9EC"));
    QRadialGradient night(QPointF(46, 48), 46);
    night.setColorAt(0, QColor("#2D5C8A"));
    night.setColorAt(1, QColor("#0B2238"));
    painter.setPen(Qt::NoPen);
    painter.setBrush(night);
    painter.drawEllipse(ball);
    QPainterPath day;
    day.moveTo(ball.center());
    day.arcTo(ball, 45, 180);
    day.closeSubpath();
    painter.setBrush(light);
    painter.drawPath(day);
    // Sun on the day side, crescent moon on the night side
    painter.setBrush(QColor("#F5B027"));
    painter.drawEllipse(QPointF(25, 27), 8, 8);
    QPainterPath moon;
    moon.addEllipse(QPointF(46, 45), 9, 9);
    QPainterPath bite;
    bite.addEllipse(QPointF(50, 41), 8, 8);
    painter.setBrush(QColor("#F2F6FA"));
    painter.drawPath(moon.subtracted(bite));
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor("#0D3C74"), 3));
    painter.drawEllipse(ball);
    return QIcon(pixmap);
}

///
/// \brief CTheme::createMenu
/// \param pParent
/// \return
///
QMenu* CTheme::createMenu(QWidget* pParent)
{
    QMenu* menu = new QMenu(pParent);
    QActionGroup* group = new QActionGroup(menu);
    const QList<QPair<Mode, QString>> entries = {
        {System, tr("Follow system")},
        {Light, tr("Light")},
        {Dark, tr("Dark")}};
    for (const auto& entry : entries)
    {
        QAction* action = menu->addAction(entry.second);
        action->setCheckable(true);
        action->setChecked(entry.first == m_mode);
        group->addAction(action);
        const Mode mode = entry.first;
        connect(action, &QAction::triggered, this, [this, mode] { setMode(mode); });
    }
    return menu;
}
