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

		// Init window position and size
		m_settings.beginGroup("MainWindow");
		restoreGeometry(m_settings.value("Geometry").toByteArray());
		restoreState(m_settings.value("State").toByteArray());
		m_settings.endGroup();
		m_settings.beginGroup("log");
		m_maxLine = m_settings.value("MaxLine",200).toUInt();
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
		connect(m_log, SIGNAL(fireLog(QString,QColor,CL_DEBUG_LEVEL)),this,SLOT(onLog(QString,QColor,CL_DEBUG_LEVEL)));
		ui->pteLogger->setPalette(p);
		ui->pteLogger->setMaximumBlockCount(m_maxLine);
		m_log->log(tr("Welcome to FFS2Play"),Qt::blue);
		// Simconnect Manager Init
		m_simManager = simbasemanager::instance();
		connect(m_simManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
		connect(m_simManager,  SIGNAL(closed()),this, SLOT(onSCM_Closed()));
#ifdef Q_OS_WIN
		m_simManager->init(new CSCManager());
#else
		m_simManager->init(new CXPManager());
#endif
		//Start timer to updating GUI
		connect(&m_updateTimer,SIGNAL(timeout()),this,SLOT(onUpdate()));
		m_updateTimer.start(500);

		m_P2PManager = CP2PManager::instance();
		m_P2PManager->init(true);
		connect(m_P2PManager, SIGNAL(onPeerAdd(QString)), SLOT(OnPeerAdd(QString)));
		connect(m_P2PManager, SIGNAL(onPeerChange(QString)), SLOT(OnPeerChange(QString)));
		connect(m_P2PManager, SIGNAL(onPeerRemove(QString)), SLOT(OnPeerRemove(QString)));

		m_webManager = CWebManager::instance();
		connect(m_webManager, SIGNAL(FireConnected()),this, SLOT(onWM_Connected()));
		connect(m_webManager, SIGNAL(FireDisconnected()),this, SLOT(onWM_Disconnected()));
	}

	///
	/// \brief MainWindow::~MainWindow
	///

	MainWindow::~MainWindow()
	{
		//delete ui;
	}

	///
	/// \brief MainWindow::closeEvent
	///

	void MainWindow::closeEvent(QCloseEvent*)
	{
		m_settings.beginGroup("MainWindow");
		m_settings.setValue("Geometry",saveGeometry());
		m_settings.setValue("State",saveState());
		m_settings.endGroup();
		m_settings.beginGroup("Log");
		m_settings.setValue("MaxLine",m_maxLine);
		m_settings.endGroup();
		m_webManager->disconnect();
		m_P2PManager->init(false);
		m_simManager->close();
	}

	///
	/// \brief MainWindow::onLog
	/// \param Texte
	/// \param pColor
	/// \param pLevel
	///
	/// Redirect Log Text flow to Plain Text Edit on main Window

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
					ui->twSIMData->item(i,0)->setText(QString::number(State.Altitude)+ " ft");
					break;
				}
				case 5:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.GroundAltitude)+ " ft");
					break;
				}
				case 6:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.Vario)+ " ft/min");
					break;
				}
				case 7:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.Heading)+ " °");
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
					ui->twSIMData->item(i,0)->setText(QString::number(State.GroundSpeed)+ " Kt");
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
					ui->twSIMData->item(i,0)->setText(QString::number(State.Pitch)+ " °");
					break;
				}
				case 16:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.Bank)+ " °");
					break;
				}
				case 17:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.GForce));
					break;
				}
				case 18:
				{
					ui->twSIMData->item(i,0)->setText(QString::number(State.Weight)+" lbs");
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
		m_webManager->connect_disconnect();
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
		m_log->log(tr("FFS2Play Version : ") + Version.FullVersion  );
	}

	///
	/// \brief MainWindow::on_afFSCon_triggered
	///

	void MainWindow::on_afFSCon_triggered()
	{
		m_simManager->open_close();
	}

	///
	/// \brief MainWindow::on_afP2P_triggered
	///

	void MainWindow::on_afP2P_triggered()
	{

	}

	///
	/// \brief MainWindow::OnPeerAdd
	/// \param pCallSign
	///

	void MainWindow::OnPeerAdd(QString pCallSign)
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

	void MainWindow::OnPeerChange(QString)
	{

	}

	///
	/// \brief MainWindow::OnPeerRemove
	/// \param pCallSign
	///

	void MainWindow::OnPeerRemove(QString pCallSign)
	{
		QList<QListWidgetItem*> item_list = ui->lwPeer->findItems(pCallSign,Qt::MatchExactly);
		if (item_list.count()>0)
		{
			delete item_list[0];
		}
		else
		{
			m_log->log("Try to remove unexisting peer in the list, ignore it",Qt::darkMagenta);
		}
	}
}

void ffs2play::MainWindow::on_rb_filterNone_toggled(bool checked)
{
	if (checked) m_log->setDebugLevel(LEVEL_NONE);
}

void ffs2play::MainWindow::on_rb_filterNormal_toggled(bool checked)
{
	if (checked) m_log->setDebugLevel(LEVEL_NORMAL);
}

void ffs2play::MainWindow::on_rb_filterVerbose_toggled(bool checked)
{
	if (checked) m_log->setDebugLevel(LEVEL_VERBOSE);
}
