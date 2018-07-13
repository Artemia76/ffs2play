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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCodec>
#include <QSettings>
#include <QTimer>
#include <QListWidget>

#include "singleton/clogger.h"
#include "singleton/simbasemanager.h"
#include "net/chttprequest.h"
#include "singleton/p2pmanager.h"
#include "singleton/webmanager.h"
#include "singleton/aimapping.h"

namespace Ui {
class MainWindow;
}

namespace ffs2play
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit		MainWindow(QWidget *pParent = 0);
						~MainWindow();
	private slots:
		void			onLog(const QString& pMessage,QColor pColor, CL_DEBUG_LEVEL pLevel);
		void			onSCM_Opened();
		void			onSCM_Closed();
		void			onSCM_Loaded(SIM_TYPE);
		void			onWM_Connected();
		void			onWM_Disconnected();
		void			onUpdate();
		void			on_afServeurCon_triggered();
		void			on_afProfil_triggered();
		void			on_afAide_triggered();
		void			on_afFSCon_triggered();
		void			on_afP2P_triggered();
		void			OnPeerRemove(QString pCallSign);
		void			OnPeerChange(QString pCallSign,int pSignal);
		void			OnPeerAdd(QString pCallSign);
		void			OnPeerReceiveTChat(QString,QString);
		void			on_rb_filterNone_toggled(bool checked);
		void			on_rb_filterNormal_toggled(bool checked);
		void			on_rb_filterVerbose_toggled(bool checked);
		void			on_cb_filter_currentIndexChanged(const QString &arg1);
		void			on_afFSType_triggered();
		void			on_leTchatSend_returnPressed();
		void			OnPeerShowContextMenu(const QPoint&);
		void			OpenMappingBox();

	private:
		Ui::MainWindow*	ui;
		CLogger*		m_log;
		simbasemanager*	m_simManager;
		CP2PManager*	m_P2PManager;
		CWebManager*	m_webManager;
		AIMapping*		m_AIMapping;
		QSettings		m_settings;
		QTimer			m_updateTimer;
		quint32			m_maxLine;
		void			closeEvent(QCloseEvent* pEvent);
	};
}
#endif // MAINWINDOW_H
