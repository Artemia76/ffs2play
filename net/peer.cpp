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

#include "peer.h"

#ifdef Q_OS_WIN
	//#include <windows.h>
#endif

namespace ffs2play
{
	CPeer::CPeer(UDPServer* pServer,
			QHostAddress pExternalIP,
			int pPort,
			QString pCallSign,
			bool pDisabled,
			bool pLocal,
			const QVector<QHostAddress>* pInternalIP,
			QObject *pParent,
			int pP2PRate,
			int pP2PRadius) : QObject(pParent),
				CallSign(m_callSign),
				Disabled(m_disabled)
	{
		m_log = CLogger::instance();
		m_AIMapping = AIMapping::instance();
		m_simManager = simbasemanager::instance();
		m_callSign = pCallSign;
		m_server = pServer;
		m_externalIP = pExternalIP;
		if (pInternalIP != NULL)
		{
			m_internalIP = *pInternalIP;
		}
		m_port = pPort;
		m_local = pLocal;
		if (!m_local) m_EP = m_externalIP;
		else m_EP.clear();
		m_disabled = true;
		m_sel_iplocal = 0;
		m_trySimpleAI = false;
		Disable(pDisabled);
		m_visible = false;
		m_spawnable = false;
		m_blockData = false;
		P2PRadius = pP2PRadius;
		P2PRate = pP2PRate;
		connect(&m_heartBeat,SIGNAL(timeout()),SLOT(OnHeartBeat()));
	}

	CPeer::~CPeer()
	{
		Stop();
	}
	void CPeer::Disable(bool pFlag)
	{
		if (m_disabled == pFlag) return;
		else
		{
			if (pFlag) Stop();
			else Start();
		}
	}
	void CPeer::Start()
	{
		if (!m_disabled) return;
		connect(m_server, SIGNAL(fireIncomingData(const QNetworkDatagram*)), this, SLOT(OnReceiveMessage(const QNetworkDatagram*)));
		connect(m_simManager, SIGNAL(fireSimVariableEvent(CAircraftState)), this, SLOT(OnStateChange(CAircraftState)));
		m_heartBeat.start(5000);
		m_lastData = Now();
		m_lastStateEvent = m_lastData;
		m_counter = 0;
		m_counter_In = 0;
		m_counter_Out = 0;
		m_distance = -1;
		m_onLine = false;
		m_sendInterval = milliseconds(5000);
		m_ecart = 0;
		m_objectID = 0;
		m_version = 0;
		m_spawned = 0;
		//m_Refresh = false;
		m_miniPing = milliseconds(1000);
		//m_PredictiveTime = 1000;
		m_settings.beginGroup("P2P");
		if (m_settings.value("P2PInfoEnable", true).toBool())
		{
			if (m_simManager->getVersion()!=SIM_VERSION::XPLANE)
				m_simManager->sendScrollingText(CallSign + tr(" is connected on P2P Network"));
		}
		m_settings.endGroup();
		m_disabled = false;
	}

	void CPeer::Stop()
	{
		if (m_disabled) return;
		//Spawn_AI(false);
		m_heartBeat.stop();
		disconnect(m_server, SIGNAL(fireIncomingData(const QNetworkDatagram*)), this,SLOT(OnReceiveMessage(const QNetworkDatagram*)));
		disconnect(m_simManager, SIGNAL(fireSimVariableEvent(ffs2play::CAircraftState)),this, SLOT(OnStateChange(ffs2play::CAircraftState)));
		m_timerCreateAI.stop();
		//m_SC.OnSCReceiveEvent -= OnSCReceiveEvent;
		//m_SC.OnSCReceiveEventFrame -= OnSCReceiveEventFrame;
		//m_SC.OnAICreated -= OnAICreated;
		//m_SC.OnAIRemoved -= OnAIRemoved;
		//Server.OnReceiveMessage -= OnReceiveMessage;
		//m_SC.OnSCReceiveAIUpdate -= OnSCReceiveAIUpdate;
		m_settings.beginGroup("P2P");
		if (m_settings.value("P2PInfoEnable", true).toBool())
		{
			if (m_simManager->getVersion()!=SIM_VERSION::XPLANE)
				m_simManager->sendScrollingText(CallSign + tr(" leave from P2P Network"));
		}
		m_settings.endGroup();
		m_disabled = true;
		m_spawned = 0;
	}

	void CPeer::spawnAI(bool pFlag)
	{
		if (pFlag && (m_objectID == 0) && (m_spawned == 0))
		{
			m_trySimpleAI = false;
#ifdef FFS_DEBUG
			m_log->log("Peer [" + m_callSign + "] " +
					tr(" Local request for Title solve = ") + QString::fromStdString(m_data.title()) +
					tr (" , Model = ") + QString::fromStdString(m_data.model()) ,
					Qt::darkBlue,LEVEL_NORMAL);
#endif
			m_AIResolution = m_AIMapping->SolveTitle(QString::fromStdString(m_data.title()), QString::fromStdString(m_data.model()), QString::fromStdString(m_data.type()));
			if (m_AIResolution.Titre != "Mooney Bravo")
			{
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" the local request has find a solution"), Qt::darkBlue,LEVEL_NORMAL);
#endif
			}
			m_spawned = 2;
			createAI();
		}
		if ((!pFlag) && (m_spawned > 0))
		{
			m_spawned = 0;
			m_timerCreateAI.stop();
			if (m_objectID != 0)
			{
				if (m_simManager->removeAI(m_objectID))
				{
#ifdef FFS_DEBUG
					m_log->log("Peer [" + m_callSign + "] " + tr(" Remove aircraft"), Qt::darkBlue,LEVEL_NORMAL);
#endif
				}
				else m_log->log("Peer [" + m_callSign + "] " + tr(" Error during removing of the aircraft"), Qt::darkMagenta);
			}
		}
	}

	void CPeer::createAI()
	{
		if (m_spawned == 2)
		{
			if (m_objectID != 0)
			{
				m_simManager->removeAI(m_objectID);
			}
		}
	}

	void CPeer::refreshData()
	{
		if ((m_objectID == 0) || (m_version != PROTO_VERSION)) return;
		// if it is the first update, we initialize datas with raw position
		if (m_spawned == 5)
		{
			m_AIData.Altitude = m_data.altitude();
			m_AIData.Latitude = m_data.latitude();
			m_AIData.Longitude = m_data.longitude();
			m_AIData.Heading = m_data.heading();
			m_AIData.Pitch = m_data.pitch();
			m_AIData.Bank = m_data.bank();
			m_simManager->freezeAI(m_objectID, m_data.onground());
			m_lastRender = Now();
			if (m_spawned==5) m_spawned=6;
		}
		// We have fresh data and are ready to compute extrapolation
		if (m_spawned >=6)
		{
			double CoefHoriz = 0;
			m_predictiveTime = m_remoteRefreshRate*3;
			// Extrapolation compute
			timespan Retard = m_lastData - fromTimeStamp(m_data.timestamp()); //Calcul du retard absolu de la donnée
			if (Retard.count() < 0) Retard = milliseconds(0);
			//double Periode = m_RemoteRefreshRate.TotalMilliseconds; //Calcul de la période entre deux données
			if (m_remoteRefreshRate.count() > 0)
			{
				// On mémorise la position actuelle de l'extrapolation
				m_actualPos.set_timestamp(toTimeStamp(m_lastRender));
				m_actualPos.set_altitude(m_AIData.Altitude);
				m_actualPos.set_latitude(m_AIData.Latitude);
				m_actualPos.set_longitude(m_AIData.Longitude);
				m_actualPos.set_heading(m_AIData.Heading);
				m_actualPos.set_bank(m_AIData.Bank);
				m_actualPos.set_pitch(m_AIData.Pitch);

				m_futurData.set_timestamp( toTimeStamp(m_lastRender + m_predictiveTime));
				CoefHoriz = (m_futurData.timestamp() - m_data.timestamp()) / m_remoteRefreshRate.count();
				m_futurData.set_altitude(m_data.altitude() + ((m_data.altitude() - m_oldData.altitude()) * CoefHoriz));
				m_futurData.set_longitude(m_data.longitude() + ((m_data.longitude() - m_oldData.longitude()) * CoefHoriz));
				m_futurData.set_latitude(m_data.latitude() + ((m_data.latitude() - m_oldData.latitude()) * CoefHoriz));
				double Gam_Heading = m_data.heading() - m_oldData.heading();
				if (Gam_Heading < -180) Gam_Heading += 360;
				if (Gam_Heading > 180) Gam_Heading -= 360;
				m_futurData.set_heading(m_data.heading() + (Gam_Heading * CoefHoriz));
				if (m_futurData.heading() < 0) m_futurData.set_heading(m_futurData.heading() + 360);
				if (m_futurData.heading() >= 360) m_futurData.set_heading(m_futurData.heading() - 360);
				double Gam_Pitch = m_data.pitch() - m_oldData.pitch();
				if (Gam_Pitch < -180) Gam_Pitch += 360;
				if (Gam_Pitch > 180) Gam_Pitch -= 360;
				m_futurData.set_pitch(m_data.pitch() + (Gam_Pitch * CoefHoriz));
				if (m_futurData.pitch() < -180) m_futurData.set_pitch(m_futurData.pitch()+ 360);
				if (m_futurData.pitch() >= 180) m_futurData.set_pitch(m_futurData.pitch()- 360);
				double Gam_Bank = m_data.bank() - m_oldData.bank();
				if (Gam_Bank < -180) Gam_Bank += 360;
				if (Gam_Bank > 180) Gam_Bank -= 360;
				m_futurData.set_bank(m_data.bank() + (Gam_Bank * CoefHoriz));
				if (m_futurData.bank() < -180) m_futurData.set_bank(m_futurData.bank() + 360);
				if (m_futurData.bank() >= 180) m_futurData.set_bank(m_futurData.bank() - 360);
				//Calcul des deltas de correction
				m_deltaAltitude = m_futurData.altitude() - m_actualPos.altitude();
				m_deltaLatitude = m_futurData.latitude() - m_actualPos.latitude();
				m_deltaLongitude = m_futurData.longitude() - m_actualPos.longitude();
				m_deltaHeading = m_futurData.heading() - m_actualPos.heading();
				if (m_deltaHeading < -180) m_deltaHeading += 360;
				if (m_deltaHeading >= 180) m_deltaHeading -= 360;
				m_deltaPitch = m_futurData.pitch() - m_actualPos.pitch();
				if (m_deltaPitch < -180) m_deltaPitch += 360;
				if (m_deltaPitch >= 180) m_deltaPitch -= 360;
				m_deltaBank = m_futurData.bank() - m_actualPos.bank();
				if (m_deltaBank < -180) m_deltaBank += 360;
				if (m_deltaBank >= 180) m_deltaBank -= 360;
			}
			m_spawned = 7;
			m_actualPos.set_pitch(m_AIData.Pitch);
			if (m_data.onground() != m_oldData.onground()) m_simManager->freezeAI(m_objectID,m_data.onground());
		}
	}
}
