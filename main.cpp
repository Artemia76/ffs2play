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
#include <QApplication>
#include <QSettings>
#include <QTranslator>

///
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("FFS2P");
    QCoreApplication::setOrganizationDomain("ffsimulateur2.fr");
    QCoreApplication::setApplicationName("FFS2Play");
    QString locale = QLocale::system().name();
    QApplication app(argc, argv);
    QDir::setCurrent(app.applicationDirPath());
    QTranslator FFS2PlayTS;
#ifdef Q_OS_LINUX
    FFS2PlayTS.load("/usr/share/ffs2play/translations/ffs2play_" + locale +".qm");
#else
    FFS2PlayTS.load("translations/ffs2play_" + locale +".qm");
#endif
    app.installTranslator(&FFS2PlayTS);
    //Load Style Sheet
    QFile styleFile ( "ffs2play.qss");
    styleFile.open(QFile::ReadOnly);
    if (styleFile.isOpen())
    {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
    ffs2play::MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
