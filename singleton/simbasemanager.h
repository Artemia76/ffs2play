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

#ifndef SIMBASEMANAGER_H
#define SIMBASEMANAGER_H

#include <QObject>
#include <QThread>
#include "singleton/clogger.h"
#include "sim/simconnector.h"


namespace ffs2play
{
	class simbasemanager : public QObject
	{
		Q_OBJECT
	private:
	static	simbasemanager*		createInstance();
		explicit				simbasemanager(QObject *parent = nullptr);
		CLogger*				m_log;
		bool					m_initialized;
		SimConnector*			m_connector;
		QThread*				m_thread;
		bool					m_bRunning;

	public:
	static simbasemanager*		instance();
								~simbasemanager();
		void					init(SimConnector* p_connector);
		void					open();
		void					close();
		bool					isConnected();
		bool					isInitialized();
		const CAircraftState	getAircraftState();
		void					sendScrollingText(const QString& pMessage);
		SIM_VERSION				getVersion();
		bool					createAI(const QString& pAircraft, const QString& pTail, const CAircraftState& pData);
		bool					removeAI(uint& pObjectID);
		void					freezeAI(uint pObjectID, bool pState);
	private slots:
		void					mainThread();
		void					OnSimVariableEvent(CAircraftState);
		void					OnSimEvent(SimEvent);
		void					OnSimAICreated(QString,uint);
	public slots:
		void					open_close();

	signals:
		void					finished();
		void					opened();
		void					closed();
		void					fireSimVariableEvent(CAircraftState);
		void					fireSimEvent(SimEvent);
		void					fireSimAICreated(QString,uint);
	};
}
#endif // SIMBASEMANAGER_H
