/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#include "fgmanager.h"

namespace ffs2play
{
	///
	/// \brief FlightGear Manager Constructor
	/// \param parent
	///
	CFGManager::CFGManager(QObject* parent):
		SimConnector(parent)
	{
		m_log->log("FGManager : " + tr("FlightGear Module loaded"));
		connect (&m_PingTimer,SIGNAL(timeout()), this, SLOT(OnPingTimer()));
	}

	///
	/// \brief CFGManager::~CFGManager
	///
	CFGManager::~CFGManager()
	{
		close();
		m_log->log("FGManager : " + tr("FlightGear Module unloaded"));
	}

	///
	/// \brief CFGManager::open
	///
	void CFGManager::open ()
	{
		if (!m_connected)
		{
			m_log->log(tr("Connecting to FlightGear ..."));
			m_PingTimer.start(5000);
		}
	}

	///
	/// \brief CFGManager::close
	///
	void CFGManager::close ()
	{
		if (m_connected)
		{
			m_connected=false;
			m_log->log(tr("Disconnected from FlightGear."));
		}
	}


	///
	/// \brief CFGManager::main_loop()
	///
	void CFGManager::main_loop()
	{

	}

	///
	/// \brief CFGManager::getVersion
	/// \return
	///
	SIM_VERSION CFGManager::getVersion()
	{
		return SIM_VERSION::FLIGHTGEAR;
	}

	///
	/// \brief CFGManager::sendScrollingText
	///
	void CFGManager::sendScrollingText(const QString& /*pMessage*/)
	{

	}

	///
	/// \brief CFGManager::createAI
	/// \return
	///
	bool CFGManager::createAI(const QString& /*pAircraft*/,const QString& /*pTail*/,const CAircraftState& /*pData*/,bool /*pNonAIObject*/)
	{
		return false;
	}

	///
	/// \brief CFGManager::removeAI
	/// \return
	///
	bool CFGManager::removeAI(uint& /*pObjectID*/)
	{
		return false;
	}

	///
	/// \brief CFGManager::updateAI
	/// \param pObjectID
	/// \param pData
	/// \return
	///
	bool CFGManager::updateAI(uint /*pObjectID*/, const CAircraftState& /*pData*/)
	{
		return false;
	}

	///
	/// \brief CFGManager::freezeAI
	///
	void CFGManager::freezeAI(uint /*pObjectID*/, bool /*pState*/)
	{
		if (m_connected)
		{
#ifdef FFS_DEBUG
			m_log->log("FGManager : " + tr("Send freezeAI"), Qt::darkBlue,LEVEL_NORMAL);
#endif
		}
	}

	///
	/// \brief CFGManager::OnPingTimer
	///
	void CFGManager::OnPingTimer()
	{
		if (m_connected)
		{
			if(timespan(Now() - m_LastPong).count()>10000)
			{
				close();
				emit(fireSimTimedOut());
			}
			//else sendPing();
		}
		else
		{
			m_PingTimer.stop();
			emit(fireSimTimedOut());
		}
	}
}
