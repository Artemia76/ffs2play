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

#include "simbasemanager.h"
#include "singleton/singleton.h"

namespace ffs2play
{
	///
	/// \brief CSCManager::CSCManager
	/// \param parent
	///

	simbasemanager::simbasemanager(QObject* parent):
		QObject(parent)
	{
		qRegisterMetaType<SimEvent>("SimEvent");
		qRegisterMetaType<CAircraftState>("CAircraftState");
		m_log = CLogger::instance();
		m_connector =nullptr;
		m_initialized = false;
		m_bRunning = false;
		m_thread = new QThread();
		this->moveToThread(m_thread);
		connect(m_thread, SIGNAL(started()), this, SLOT(mainThread()));
		connect(this, SIGNAL(finished()), m_thread, SLOT(quit()), Qt::DirectConnection );
	}

	///
	/// \brief CSCManager::createInstance
	/// \return
	///

	simbasemanager* simbasemanager::createInstance()
	{
		return new simbasemanager();
	}

	///
	/// \brief CSCManager::~CSCManager
	///

	simbasemanager::~simbasemanager()
	{
		close();
	}

	///
	/// \brief CSCManager::instance
	/// \return
	///

	simbasemanager* simbasemanager::instance()
	{
		return Singleton<simbasemanager>::instance(simbasemanager::createInstance);
	}

	void simbasemanager::init(SimConnector *p_connector)
	{
		if ((p_connector==NULL)||(m_initialized)) return;
		m_connector=p_connector;
		m_initialized=true;
		connect(m_connector,SIGNAL(fireSimEvent(SimEvent)),this,SLOT(OnSimEvent(SimEvent)));
		connect(m_connector,SIGNAL(fireSimVariableEvent(CAircraftState)),this,SLOT(OnSimVariableEvent(CAircraftState)));
		CAircraftState state;
		emit m_connector->fireSimVariableEvent(state);
		connect(m_connector,SIGNAL(fireSimEventFrame(float,float)),this,SLOT(OnSimEventFrame(float,float)));
		connect(m_connector,SIGNAL(fireSimAICreated(QString,uint)),this,SLOT(OnSimAICreated(QString,uint)));
		connect(m_connector,SIGNAL(fireSimAIUpdated(CAircraftState,uint)),this,SLOT(OnSimAIUpdated(CAircraftState,uint)));
		connect(m_connector,SIGNAL(fireSimAIRemoved(uint)),this,SLOT(OnSimAIRemoved(uint)));
	}

	///
	/// \brief CSCManager::open_close
	///

	void simbasemanager::open_close()
	{
		if (!isConnected()) open();
		else close();
	}

	///
	/// \brief CSCManager::open
	///

	void simbasemanager::open ()
	{
		if (!isInitialized()) return;
		if (!m_connector->isConnected())
		{
			if (m_connector->open())
			{
				// Thread starting
				m_thread->start();
				emit(opened());
			}
			else
			{
				m_log->log(tr("Connexion to simulator failed."),Qt::darkMagenta);
			}
		}
	}

	///
	/// \brief CSCManager::close
	///

	void simbasemanager::close ()
	{
		if (!isInitialized()) return;
		if (m_connector->isConnected())
		{
			m_connector->close();
			m_thread->wait(0);
			m_log->log(tr("Disconnected from simulator."));
			emit(closed());
		}
	}

	bool simbasemanager::isInitialized()
	{
		if (m_connector==NULL) return false;
		return m_initialized;
	}

	bool simbasemanager::isConnected()
	{
		if (m_connector==NULL) return false;
		return m_connector->isConnected();
	}

	///
	/// \brief CSCManager::mainThread
	///

	void simbasemanager::mainThread()
	{
		while (m_connector->isConnected())
		{
			m_connector->main_loop();
			QApplication::processEvents();
			m_thread->msleep(1);
		}
		emit(finished());
	}

	///
	/// \brief simbasemanager::getAircraftState
	/// \return
	///

	const CAircraftState simbasemanager::getAircraftState()
	{
		CAircraftState State;
		if (m_initialized && (m_connector!=NULL))
			State = m_connector->getState();
		return State;
	}

	///
	/// \brief simbasemanager::sendScrollingText
	/// \param pMessage
	///

	void simbasemanager::sendScrollingText(const QString &pMessage)
	{
		if (m_initialized && (m_connector!=nullptr))
		{
			m_connector->sendScrollingText(pMessage);
		}
	}

	SIM_VERSION simbasemanager::getVersion()
	{
		SIM_VERSION Version = SIM_VERSION::UNKNOWN;
		if (m_initialized && (m_connector!=nullptr))
		{
			Version = m_connector->getVersion();
		}
		return Version;
	}

	bool simbasemanager::createAI(const QString& pAircraft, const QString& pTail, const CAircraftState& pData,bool pNonAIObject)
	{
		if (m_initialized && (m_connector!=nullptr))
		{
			return m_connector->createAI(pAircraft,pTail,pData,pNonAIObject);
		}
		return false;
	}

	bool simbasemanager::updateAI(uint pObjectID, const CAircraftState &pData)
	{
		if (m_initialized && (m_connector!=nullptr))
		{
			return m_connector->updateAI(pObjectID,pData);
		}
		return false;
	}

	bool simbasemanager::removeAI(uint& pObjectID)
	{
		if (m_initialized && (m_connector!=nullptr))
		{
			return m_connector->removeAI(pObjectID);
		}
		return false;
	}

	void simbasemanager::freezeAI(uint pObjectID, bool pState)
	{
		if (m_initialized && (m_connector!=nullptr))
		{
			m_connector->freezeAI(pObjectID,pState);
		}
	}

	void simbasemanager::OnSimVariableEvent(CAircraftState pState)
	{
		emit(fireSimVariableEvent(pState));
	}

	void simbasemanager::OnSimEvent(SimEvent pEvent)
	{
		emit(fireSimEvent(pEvent));
	}

	void simbasemanager::OnSimAICreated(QString pTail, uint pObjectID)
	{
		emit(fireSimAICreated(pTail,pObjectID));
	}

	void simbasemanager::OnSimAIUpdated(CAircraftState pAIData, uint pObjectID)
	{
		emit(fireSimAIUpdated(pAIData,pObjectID));
	}

	void simbasemanager::OnSimAIRemoved(uint pObjectID)
	{
		emit(fireSimAIRemoved(pObjectID));
	}

	void simbasemanager::OnSimEventFrame(float pFramRate, float SimSpeed)
	{
		emit(fireSimEventFrame(pFramRate, SimSpeed));
	}
}
