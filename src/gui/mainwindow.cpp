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

#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "net/chttprequest.h"
#include "gui/dialogprofils.h"
#include "tools/getappversion.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    Log->setOutput(ui->pteLogger);
    Log->setDebugLevel(LEVEL_VERBOSE);
    ui->pteLogger->setPalette(p);
    Log->log("Bienvenue sur FFS2Play",Qt::blue);
    // Initialisation Simconnect Manager
    scManager = CSCManager::instance();
    connect(scManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
    connect(scManager, SIGNAL(closed()),this, SLOT(onSCM_Closed()));
    // Init window position and size
    s.beginGroup("MainWindow");
    resize(s.value("size", QSize(400,400)).toSize());
    move(s.value("pos", QPoint(200,200)).toPoint());
    s.endGroup();
    //Lancement du timer pour mise à jour de l'interface
    connect(&Update,&QTimer::timeout,this,&MainWindow::onUpdate);
    Update.start(5000);
    P2PManager = CP2PManager::instance();
    P2PManager->init(true);

    WebManager = CWebManager::instance();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent*)
{
    s.beginGroup("MainWindow");
    s.setValue("size", size());
    s.setValue("pos", pos());
    s.endGroup();
    P2PManager->init(false);
}

void MainWindow::on_atFSCon_triggered()
{
    scManager->open_close();
}

void MainWindow::onSCM_Opened()
{
    ui->atFSCon->setIcon(QIcon(":/gfx/plane_green.png"));
}

void MainWindow::onSCM_Closed()
{
    ui->atFSCon->setIcon(QIcon(":/gfx/plane_gris.png"));
}

void MainWindow::onUpdate()
{

}

void MainWindow::on_afServeurCon_triggered()
{
    WebManager->Connect();
}

void MainWindow::on_afProfil_triggered()
{
    DialogProfils* dialog = new DialogProfils();
    dialog->show();
}

void MainWindow::on_afAide_triggered()
{
    sVersion Version = GetAppVersion();
    Log->log("Version actuelle : " + Version.FullVersion  );
}
