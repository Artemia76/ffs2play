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
 * mainwindow.cpp is part of FF2Play project
 *
 * This class define the main window of the application
 * **************************************************************************/

#include <QMessageBox>
#include <QByteArray>
#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace ffs2play
{
    ///
    /// \brief MainWindow::MainWindow
    /// \param parent
    ///
    MainWindow::MainWindow(QWidget *pParent) :
        QMainWindow(pParent),
        ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
        // Specify custom handler for contextual menu
        ui->pteLogger->setContextMenuPolicy(Qt::CustomContextMenu);

        // Init window position and size
        m_settings.beginGroup("MainWindow");
        restoreGeometry(m_settings.value("Geometry").toByteArray());
        restoreState(m_settings.value("State").toByteArray());
        m_settings.endGroup();
        m_settings.beginGroup("log");
        m_maxLine = m_settings.value("MaxLine",200).toInt();
        m_settings.endGroup();
        QPalette p = ui->pteLogger->palette();
        p.setColor(QPalette::Text,Qt::darkBlue);
        m_log = CLogger::instance();
        switch (m_log->DebugLevel)
        {
            case LEVEL_NONE:
            {
                ui->rb_filterNone->setChecked(true);
                break;
            }
            case LEVEL_NORMAL:
            {
                ui->rb_filterNormal->setChecked(true);
                break;
            }
            case LEVEL_VERBOSE:
            {
                ui->rb_filterVerbose->setChecked(true);
                break;
            }
        }
        int id=ui->cb_filter->findText(m_log->getFilter(),Qt::MatchExactly);
        if (id >=0)
        {
            ui->cb_filter->setCurrentIndex(id);
        }
        else
        {
            ui->cb_filter->addItem(m_log->getFilter());
            ui->cb_filter->setCurrentIndex(ui->cb_filter->findText(m_log->getFilter(),Qt::MatchExactly));
        }
        connect(m_log, SIGNAL(fireLog(QString,QColor,CL_DEBUG_LEVEL)),this,SLOT(onLog(QString,QColor,CL_DEBUG_LEVEL)));
        ui->pteLogger->setPalette(p);
        ui->pteLogger->setMaximumBlockCount(m_maxLine);
        m_log->log(tr("Welcome to FFS2Play"),Qt::blue);
        // Simconnect Manager Init
        m_simManager = simbasemanager::instance();
        connect(m_simManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
        connect(m_simManager,  SIGNAL(closed()),this, SLOT(onSCM_Closed()));
        connect(m_simManager, SIGNAL(loaded(SIM_TYPE)),this, SLOT(onSCM_Loaded(SIM_TYPE)));
        onSCM_Loaded(m_simManager->getType());

        m_P2PManager = CP2PManager::instance();
        m_P2PManager->init(true);
        connect(m_P2PManager, SIGNAL(firePeerAdd(QString)), SLOT(onPeerAdd(QString)));
        connect(m_P2PManager, SIGNAL(firePeerChange(QString,int)), SLOT(onPeerChange(QString,int)));
        connect(m_P2PManager, SIGNAL(firePeerRemove(QString)), SLOT(onPeerRemove(QString)));
        connect(m_P2PManager, SIGNAL(fireReceiveTChat(QString,QString)), SLOT(onPeerReceiveTChat(QString,QString)));

        m_webManager = CWebManager::instance();

        connect(m_webManager, SIGNAL(FireConnected()),this, SLOT(onWM_Connected()));
        connect(m_webManager, SIGNAL(FireDisconnected()),this, SLOT(onWM_Disconnected()));
        connect(m_webManager, SIGNAL(FireExit()),this, SLOT(onWM_Exit()));

        m_AIMapping = AIMapping::instance();
        connect(m_AIMapping, SIGNAL(fireScanFinished(int,int,int)),this, SLOT(onAIScanFinished(int,int,int)));

        m_Analyzer = Analyzer::instance();

        ui->lwPeer->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->lwPeer, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onPeerShowContextMenu(QPoint)));
#ifdef FFS_DEBUG
        ui->dwDebugOpt->setEnabled(true);
        setWindowTitle("FFS2Play(debug)");
        //Start timer to updating GUI
        connect(&m_updateTimer,SIGNAL(timeout()),this,SLOT(onUpdate()));
        m_updateTimer.start(500);
#else
        delete ui->dwDebugOpt;
        delete ui->dwSimVariable;
#endif
        m_simManager->open_close();
        m_webManager->CheckNewVersion();
        m_webManager->Connect();
        QApplication::setPalette(this->palette());
        if(!ui->mainToolBar->isVisible()) ui->mainToolBar->setVisible(true);
        m_P2PDialogVisible=false;
    }

    ///
    /// \brief MainWindow::~MainWindow
    ///
    MainWindow::~MainWindow()
    {
        //delete ui;
    }

    ///
    /// \brief MainWindow::onLog
    /// \param Texte
    /// \param pColor
    /// \param pLevel
    ///
    /// Redirect Log Text flow to Plain Text Edit on main Window
    ///
    void MainWindow::onLog (const QString& pMessage, QColor pColor, CL_DEBUG_LEVEL)
    {
        QTextCharFormat fmt;
        fmt.setForeground(QBrush(pColor));
        ui->pteLogger->mergeCurrentCharFormat(fmt);
        ui->pteLogger->appendPlainText(pMessage);
    }

    ///
    /// \brief MainWindow::onSCM_Opened
    ///
    void MainWindow::onSCM_Opened()
    {
        ui->afFSCon->setIcon(QIcon(":/gfx/plane_green.png"));
        if (m_simManager->getType()==SIM_TYPE::XPLANE) m_webManager->CheckXPPluginVersion();
    }

    ///
    /// \brief MainWindow::onSCM_Closed
    ///
    void MainWindow::onSCM_Closed()
    {
        ui->afFSCon->setIcon(QIcon(":/gfx/plane_gris.png"));
    }

    ///
    /// \brief MainWindow::onSCM_Loaded
    /// \param pSimType
    ///
    void MainWindow::onSCM_Loaded(SIM_TYPE pSimType)
    {
        switch (pSimType)
        {
            case SIM_TYPE::FSX:
            {
                ui->afFSType->setIcon(QIcon(":/gfx/p3d.png"));
                break;
            }
            case SIM_TYPE::XPLANE:
            {
                ui->afFSType->setIcon(QIcon(":/gfx/xplane.png"));
                break;
            }
#ifdef FFS_DEBUG
            case SIM_TYPE::FLIGHTGEAR:
            {
                ui->afFSType->setIcon(QIcon(":/gfx/flightgear.png"));
                break;
            }
#endif
        }
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
    /// \brief MainWindow::onWM_Exit
    ///
    void MainWindow::onWM_Exit()
    {
        this->close();
    }

    ///
    /// \brief MainWindow::onAIScanFinished
    ///
    void MainWindow::onAIScanFinished(int pNbAdded, int pNbUpdated, int pNBDeleted)
    {
        if (m_simManager->isConnected()&& (m_AIMapping->getPathList().count()==0) && (!m_P2PDialogVisible))
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("Simulator Aircraft Folder"),tr("at least one folder is not configured for your simulator. Do you want add the path location now?"),QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                on_afP2P_triggered();
            }
        }
        if ((pNbAdded!=0)||(pNbUpdated!=0)||(pNBDeleted!=0))
        {
            if (m_webManager->isConnected())
            {
                m_webManager->Disconnect();
                m_webManager->Connect();
            }
        }
    }

    ///
    /// \brief MainWindow::OnPeerAdd
    /// \param pCallSign
    ///
    void MainWindow::onPeerAdd(QString pCallSign)
    {
        QList<QListWidgetItem*> item_list = ui->lwPeer->findItems(pCallSign,Qt::MatchExactly);
        if (item_list.count()==0)
        {
            QListWidgetItem* item = new QListWidgetItem	(QIcon(":/gfx/signal_0.png"),pCallSign);
            ui->lwPeer->addItem(item);
        }
        else
        {
            m_log->log("Try to add existing peer in the list, ignore it",Qt::darkMagenta);
        }
    }

    ///
    /// \brief MainWindow::OnPeerChange
    /// \param pCallSign
    ///
    void MainWindow::onPeerChange(QString pCallSign, int pSignal)
    {
        // Update Peer List
        QList<QListWidgetItem*> item_list = ui->lwPeer->findItems(pCallSign,Qt::MatchExactly);
        if (item_list.count()>0)
        {
            QString icon;
            switch (pSignal)
            {
                case 0:
                    icon=":/gfx/signal_0.png";
                    break;
                case 1:
                    icon=":/gfx/signal_1.png";
                    break;
                case 2:
                    icon=":/gfx/signal_2.png";
                    break;
                case 3:
                    icon=":/gfx/signal_3.png";
                    break;
                case 4:
                    icon=":/gfx/close.png";
                    break;
                default:
                    icon=":/gfx/close.png";
                    break;
            }
            item_list[0]->setIcon(QIcon(icon));
        }
    }

    ///
    /// \brief MainWindow::OnPeerRemove
    /// \param pCallSign
    ///
    void MainWindow::onPeerRemove(QString pCallSign)
    {
        //QList<QListWidgetItem*> item_list = ui->lwPeer->findItems(pCallSign,Qt::MatchExactly);
        //if (item_list.count()>0)
        for (int i=0; ui->lwPeer->count();i++)
        {
            if (ui->lwPeer->item(i)->text()==pCallSign)
            {
                QListWidgetItem* item = ui->lwPeer->takeItem(i);
                delete item;
                return;
            }
        }
        m_log->log("Try to remove unexisting peer in the list, ignore it",Qt::darkMagenta);
    }

    ///
    /// \brief MainWindow::onPeerReceiveTChat
    /// \param pCallSign
    /// \param pMessage
    ///
    void MainWindow::onPeerReceiveTChat(QString pCallSign,QString pMessage)
    {
        ui->teTChatZone->append(pCallSign + " : " + pMessage);
    }

    ///
    /// \brief MainWindow::onUpdate
    ///
    void MainWindow::onUpdate()
    {
#ifdef FFS_DEBUG
        CAircraftState State = m_simManager->getAircraftState();
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
                    ui->twSIMData->item(i,0)->setText(m_Analyzer->getPhaseName());
                    break;
                }
                case 5:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Altitude)+ " ft");
                    break;
                }
                case 6:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.GroundAltitude)+ " ft");
                    break;
                }
                case 7:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Vario)+ " ft/min");
                    break;
                }
                case 8:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Heading)+ " °");
                    break;
                }
                case 9:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Longitude)+ " °");
                    break;
                }
                case 10:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Latitude)+ " °");
                    break;
                }
                case 11:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.GroundSpeed)+ " Kt");
                    break;
                }
                case 12:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.TASSpeed)+ " Kt");
                    break;
                }
                case 13:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Fuel)+ " lbs");
                    break;
                }
                case 14:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Fuel)+ " lbs");
                    break;
                }
                case 15:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.TotalFuelCapacity)+ " lbs");
                    break;
                }
                case 16:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Pitch)+ " °");
                    break;
                }
                case 17:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Bank)+ " °");
                    break;
                }
                case 18:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.GForce));
                    break;
                }
                case 19:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Weight)+" lbs");
                    break;
                }
                case 20:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Realism));
                    break;
                }
                case 21:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantWindVelocity));
                    break;
                }
                case 22:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantWindDirection));
                    break;
                }
                case 23:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.AmbiantPrecipState));
                    break;
                }
                case 24:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.SeaLevelPressure));
                    break;
                }
                case 25:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.IASSpeed) + " kt");
                    break;
                }
                case 26:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ElevatorPos) + " °");
                    break;
                }
                case 27:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.AileronPos) + " °");
                    break;
                }
                case 28:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.RudderPos) + " °");
                    break;
                }
                case 29:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.TimeFactor));
                    break;
                }
                case 30:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Door1Pos));
                    break;
                }
                case 31:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Door2Pos));
                    break;
                }
                case 32:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Door3Pos));
                    break;
                }
                case 33:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.Door4Pos));
                    break;
                }
                case 34:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ParkingBrake));
                    break;
                }
                case 35:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ThrottleEng1));
                    break;
                }
                case 36:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ThrottleEng2));
                    break;
                }
                case 37:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ThrottleEng3));
                    break;
                }
                case 38:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.ThrottleEng4));
                    break;
                }
                case 39:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.BeaconLight));
                    break;
                }
                case 40:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.LandingLight));
                    break;
                }
                case 41:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.StrobeLight));
                    break;
                }
                case 42:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.NavLight));
                    break;
                }
                case 43:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.RecoLight));
                    break;
                }
                case 44:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.StateEng1));
                    break;
                }
                case 45:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.StateEng2));
                    break;
                }
                case 46:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.StateEng3));
                    break;
                }
                case 47:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.StateEng4));
                    break;
                }
                case 48:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.FlapsIndex));
                    break;
                }
                case 49:
                {
                    ui->twSIMData->item(i,0)->setText(QString::number(State.SquawkMode));
                    break;
                }
                case 50:
                {
                    int Transponder;
                    if (m_simManager->getType()==SIM_TYPE::FSX)
                    {
                        Transponder=ConvertToBinaryCodedDecimal(State.Squawk);
                    }
                    else
                    {
                        Transponder=State.Squawk;
                    }
                    ui->twSIMData->item(i,0)->setText(QString::number(Transponder));
                    break;
                }
            }
        }
#endif
        // Update Peer detail if it available
        QListWidgetItem* Peer = ui->lwPeer->currentItem();
        CAircraftState PeerState;
        double distance = 0.0;
        if (Peer != nullptr)
        {
            PeerState = m_P2PManager->getPeerData(Peer->text());
            distance = m_P2PManager->getPeerDistance(Peer->text());
        }
        QString Value;
        for (int i = 0; i < ui->twPeerDetail->rowCount(); i++)
        {
            switch (i)
            {
                case 0:
                {
                    ui->twPeerDetail->item(i,0)->setText(PeerState.Title);
                    break;
                }
                case 1:
                {
                    ui->twPeerDetail->item(i,0)->setText(Value.sprintf("%.0f ft", PeerState.Altitude));
                    break;
                }
                case 2:
                {
                    ui->twPeerDetail->item(i,0)->setText(Value.sprintf("%.0f °", PeerState.Heading));
                    break;
                }
                case 3:
                {
                    ui->twPeerDetail->item(i,0)->setText(Value.sprintf("%.0f Nm", distance));
                    break;
                }
                case 4:
                {
                    ui->twPeerDetail->item(i,0)->setText(Value.sprintf("%.0f Kts", PeerState.IASSpeed));
                    break;
                }
                case 5:
                {
                    ui->twPeerDetail->item(i,0)->setText(Value.sprintf("%.0f Kts",PeerState.GroundSpeed));
                    break;
                }
            }
        }
    }
}
