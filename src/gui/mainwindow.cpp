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

///
/// \brief MainWindow::MainWindow
/// \param parent
///

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Init window position and size
    s.beginGroup("MainWindow");
    resize(s.value("size", QSize(400,400)).toSize());
    move(s.value("pos", QPoint(200,200)).toPoint());
    s.endGroup();

    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    Log->setOutput(ui->pteLogger);
    ui->pteLogger->setPalette(p);
    Log->log(tr("Welcome to FFS2Play"),Qt::blue);
    // Simconnect Manager Init
    scManager = CSCManager::instance();
    connect(scManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
    connect(scManager, SIGNAL(closed()),this, SLOT(onSCM_Closed()));
    //Start timer to updating GUI
    connect(&Update,SIGNAL(timeout()),this,SLOT(onUpdate()));
    Update.start(5000);

    P2PManager = CP2PManager::instance();
    P2PManager->init(true);

    WebManager = CWebManager::instance();
    connect(WebManager, SIGNAL(FireConnected()),this, SLOT(onWM_Connected()));
    connect(WebManager, SIGNAL(FireDisconnected()),this, SLOT(onWM_Disconnected()));
}

///
/// \brief MainWindow::~MainWindow
///

MainWindow::~MainWindow()
{
    delete ui;
}

///
/// \brief MainWindow::closeEvent
///

void MainWindow::closeEvent(QCloseEvent*)
{
    s.beginGroup("MainWindow");
    s.setValue("size", size());
    s.setValue("pos", pos());
    s.endGroup();
    P2PManager->init(false);
    scManager->close();
}

///
/// \brief MainWindow::onSCM_Opened
///

void MainWindow::onSCM_Opened()
{
    ui->afFSCon->setIcon(QIcon(":/gfx/plane_green.png"));
}

///
/// \brief MainWindow::onSCM_Closed
///

void MainWindow::onSCM_Closed()
{
    ui->afFSCon->setIcon(QIcon(":/gfx/plane_gris.png"));
}

///
/// \brief MainWindow::onWM_Connected
///

void MainWindow::onWM_Connected()
{
    ui->afServeurCon->setIcon(QIcon(":/gfx/connect2_V.png"));
}

///
/// \brief MainWindow::onWM_Disconnected
///

void MainWindow::onWM_Disconnected()
{
    ui->afServeurCon->setIcon(QIcon(":/gfx/connect2_R.png"));
}

///
/// \brief MainWindow::onUpdate
///

void MainWindow::onUpdate()
{

}

///
/// \brief MainWindow::on_afServeurCon_triggered
///

void MainWindow::on_afServeurCon_triggered()
{
    WebManager->connect_disconnect();
}

///
/// \brief MainWindow::on_afProfil_triggered
///

void MainWindow::on_afProfil_triggered()
{
    DialogProfils* dialog = new DialogProfils();
    dialog->show();
}

///
/// \brief MainWindow::on_afAide_triggered
///

void MainWindow::on_afAide_triggered()
{
    sVersion Version = GetAppVersion();
    Log->log(tr("FFS2Play Version : ") + Version.FullVersion  );
}

void MainWindow::on_afFSCon_triggered()
{
    scManager->open_close();
}
