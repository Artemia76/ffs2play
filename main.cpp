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
 * main.cpp is part of FF2Play project
 *
 * This is the entry point of ffs2play application
 * **************************************************************************/

#include "gui/mainwindow.h"
#include "gui/theme.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>

#ifdef Q_OS_MACOS
    #include "tools/keepalive.h"
#endif

///
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
#ifdef Q_OS_MACOS
    KeepAlive kalive;
#endif
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("FFS2P");
    QCoreApplication::setOrganizationDomain("ffs2play.fr");
    QCoreApplication::setApplicationName("FFS2Play");
    QApplication app(argc, argv);
    QDir::setCurrent(app.applicationDirPath());
    // Qt own translations (deployed with the application)
    QTranslator QtTS;
    if (QtTS.load(QLocale(), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&QtTS);
    // Application translations and style sheet are embedded in resources
    QTranslator FFS2PlayTS;
    if (FFS2PlayTS.load(QLocale(), "ffs2play", "_", ":/i18n"))
        app.installTranslator(&FFS2PlayTS);
    // Light or dark theme (palette and style sheet)
    CTheme::instance().init();
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
