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

#ifndef SIMCONNECTOR_H
#define SIMCONNECTOR_H

#include <QObject>
#include <QMutex>
#include "sim/aircraftstate.h"
#include "singleton/clogger.h"
#include "tools/datetime.h"

namespace ffs2play
{

	enum class SIM_VERSION
	{
		FSX,
		FSX_STEAM,
		P3D_V2,
		P3D_V3,
		P3D_V4,
		XPLANE,
		UNKNOWN
	};

	/// <summary>
	/// Enumération des évenements du simulateur à traiter
	/// </summary>
	enum class SIM_EVENT_ID
	{
		PAUSE,
		PARKING_BRAKE,
		SIMSTART,
		SIMSTOP,
		OVERSPEED,
		STALLING,
		CRASH,
		SLEW,
		PUSHBACK,
		ON_GROUND,
		GEAR_HANDLE_POSITION,
		FLAPS_HANDLE_PERCENT,
		FLAPS_HANDLE_INDEX,
		ALTIMETER_SETTING,
		LANDING_LIGHT,
		BEACON_LIGHT,
		STROBE_LIGHT,
		NAV_LIGHT,
		RECOGNITION_LIGHT,
		SMOKE_ENABLE,
		SLEW_ON,
		UNUSED = 0xFFFFFFF,
	};

	class SimEvent
	{
	public:
		SimEvent()
		{
			Time = Now();
		}

		SimEvent(SIM_EVENT_ID pEvt_ID, int pData )
		{
			Time = Now();
			Data = pData;
			Evt_Id = pEvt_ID;
		}

		SIM_EVENT_ID Evt_Id;
		int Data;
		datetime Time;
	};

	class SimConnector : public QObject
	{
		Q_OBJECT
		friend class	simbasemanager;
	public:
		explicit		SimConnector(QObject *parent = nullptr);
		virtual			~SimConnector();
		bool			isConnected();
	protected:
		CLogger*		m_log;
		bool			m_connected;
		CAircraftState  m_state;
		QMutex			m_Mutex;

virtual	void			open()=0;
virtual void			close()=0;
virtual	void			main_loop()=0;
virtual	SIM_VERSION		getVersion()=0;
virtual	void			sendScrollingText(const QString& pMessage)=0;
virtual	bool			createAI(const QString& pAircraft,const QString& Tail,const CAircraftState& pData,bool pNonAIObject)=0;
virtual	bool			removeAI(uint& pObjectID)=0;
virtual bool			updateAI(uint pObjectID, const CAircraftState& data)=0;
virtual	void			freezeAI(uint pObjectID, bool pState)=0;
virtual CAircraftState	getState();
	signals:
		void			finished();
		void			fireSimVariableEvent(CAircraftState);
		void			fireSimEvent(SimEvent);
		void			fireSimAICreated(QString,uint);
		void			fireSimAIUpdated(CAircraftState, uint);
		void			fireSimAIRemoved(uint);
		void			fireSimEventFrame(float,float);
		void			fireSimConnected();
		void			fireSimDisconnected();
		void			fireSimTimedOut();

	public slots:
	};
}
#endif // SIMCONNECTOR_H
