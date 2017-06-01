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

#include <QByteArray>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogprofils.h"
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
    restoreGeometry(s.value("Geometry").toByteArray());
    restoreState(s.value("State").toByteArray());
    s.endGroup();

    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    connect(Log, SIGNAL(fireLog(QString,QColor,CL_DEBUG_LEVEL)),this,SLOT(onLog(QString,QColor,CL_DEBUG_LEVEL)));
    ui->pteLogger->setPalette(p);
    Log->log(tr("Welcome to FFS2Play"),Qt::blue);
    // Simconnect Manager Init
    SimManager = simbasemanager::instance();
    connect(SimManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
    connect(SimManager, SIGNAL(closed()),this, SLOT(onSCM_Closed()));
    SimManager->init(new CSCManager());
    //Start timer to updating GUI
    connect(&Update,SIGNAL(timeout()),this,SLOT(onUpdate()));
    Update.start(500);

    P2PManager = CP2PManager::instance();
    P2PManager->init(true);

    WebManager = CWebManager::instance();
    connect(WebManager, SIGNAL(FireConnected()),this, SLOT(onWM_Connected()));
    connect(WebManager, SIGNAL(FireDisconnected()),this, SLOT(onWM_Disconnected()));
    connect(SimManager, SIGNAL(fireSimEvent(SimEvent)),SLOT(OnSimEvent(SimEvent)));
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
    s.setValue("Geometry",saveGeometry());
    s.setValue("State",saveState());
    s.endGroup();
    P2PManager->init(false);
    SimManager->close();
}


void MainWindow::onLog (const QString& Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(pColor));
    ui->pteLogger->mergeCurrentCharFormat(fmt);
    ui->pteLogger->appendPlainText(Texte);
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
    CAircraftState State = SimManager->getAircraftState();
    for (int i = 0; i < ui->twSIMData->rowCount(); i++)
    {
        switch (i)
        {
            case 0:
            {
                ui->twSIMData->item(i,0)->setText(State.Title);
                break;
            }
            case 1:
            {
                ui->twSIMData->item(i,0)->setText(State.Type);
                break;
            }
            case 2:
            {
                ui->twSIMData->item(i,0)->setText(State.Model);
                break;
            }
            case 3:
            {
                ui->twSIMData->item(i,0)->setText(State.Category);
                break;
            }
            case 4:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.PlaneAltitude)+ " ft");
                break;
            }
            case 5:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.GndAltitude)+ " ft");
                break;
            }
            case 6:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Vario)+ " ft/min");
                break;
            }
            case 7:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.PlaneHeading)+ " °");
                break;
            }
            case 8:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Longitude)+ " °");
                break;
            }
            case 9:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Latitude)+ " °");
                break;
            }
            case 10:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.GndSpeed)+ " Kt");
                break;
            }
            case 11:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.TASSpeed)+ " Kt");
                break;
            }
            case 12:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Fuel)+ " lbs");
                break;
            }
            case 13:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Fuel)+ " lbs");
                break;
            }
            case 14:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.TotalFuelCapacity)+ " lbs");
                break;
            }
            case 15:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.PlanePitch)+ " °");
                break;
            }
            case 16:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.PlaneBank)+ " °");
                break;
            }
            case 17:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.GForce));
                break;
            }
            case 18:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.PlaneWeight)+" lbs");
                break;
            }
            case 19:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.Realism));
                break;
            }
            case 20:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantWindVelocity));
                break;
            }
            case 21:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantWindDirection));
                break;
            }
            case 22:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantPrecipState));
                break;
            }
            case 23:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.SeaLevelPressure));
                break;
            }
            case 24:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.IASSpeed) + " kt");
                break;
            }
            case 25:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.ElevatorPos) + " °");
                break;
            }
            case 26:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.AileronPos) + " °");
                break;
            }
            case 27:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.RudderPos) + " °");
                break;
            }
            case 28:
            {
                ui->twSIMData->item(i,0)->setText(QString::number(State.TimeFactor));
                break;
            }
        case 29:
        {
            ui->twSIMData->item(i,0)->setText(QString::number(State.Door1Pos));
            break;
        }
        case 30:
        {
            ui->twSIMData->item(i,0)->setText(QString::number(State.Door2Pos));
            break;
        }
        case 31:
        {
            ui->twSIMData->item(i,0)->setText(QString::number(State.Door3Pos));
            break;
        }
        case 32:
        {
            ui->twSIMData->item(i,0)->setText(QString::number(State.Door4Pos));
            break;
        }
        case 33:
        {
            ui->twSIMData->item(i,0)->setText(QString::number((int)State.ParkingBrake));
            break;
        }
        }

    }
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
    SimManager->open_close();
}

void MainWindow::on_afP2P_triggered()
{

}

void MainWindow::OnSimEvent(SimEvent Event)
{
    Log->log("Receive Sim Event = " + QString::number((int)Event.Evt_Id) + " Data = " + QString::number(Event.Data));
}
