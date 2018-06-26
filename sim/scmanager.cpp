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

#include "scmanager.h"
#include "singleton/singleton.h"

#include <QCoreApplication>

namespace ffs2play
{

	///
	/// \brief CSCManager::CSCManager
	/// \param parent
	///
	CSCManager::CSCManager(QObject* parent):
		SimConnector(parent)
	{
		// Initialisation de l'état de connexion avec le simulateur
		m_hSimConnect = NULL;
		m_log->log("SCManager : " + tr("FSX/Prepar3D Module loaded"));
	}

	///
	/// \brief CSCManager::~CSCManager
	///
	CSCManager::~CSCManager()
	{
		close();
		m_log->log("SCManager : " + tr("FSX/Prepar3D Module unloaded"));
	}

	///
	/// \brief CSCManager::open
	///
	void CSCManager::open ()
	{
		if (!m_connected)
		{
			HRESULT hr;
			if (SUCCEEDED (hr = SimConnect_Open (&m_hSimConnect,QCoreApplication::applicationName().toStdString().c_str(),NULL,NULL, (HANDLE)WM_USER_SIMCONNECT,0)))
			{
			}
			else
			{
				m_log->log(tr("Connexion to FS failed."),Qt::darkMagenta);
				emit(fireSimTimedOut());
			}
		}
	}

	///
	/// \brief CSCManager::close
	///
	void CSCManager::close ()
	{
		if (m_connected)
		{
			m_connected = false;
			SimConnect_Close(m_hSimConnect);
			m_log->log(tr("Disconnected from FS."));
			emit(fireSimDisconnected());
		}
	}

	///
	/// \brief CSCManager::DispatchCallback
	/// \param pData
	/// \param cbData
	/// \param pContext
	///
	void CALLBACK CSCManager::DispatchCallback(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
	{
		CSCManager *pThis = reinterpret_cast<CSCManager*>(pContext);
		pThis->process(pData, cbData);
	}

	///
	/// \brief CSCManager::main_loop()
	///
	void CSCManager::main_loop()
	{
		HRESULT hr;
		if (m_hSimConnect!=nullptr)
		{
			hr=SimConnect_CallDispatch(m_hSimConnect,CSCManager::DispatchCallback,this);
		}
	}

	///
	/// \brief CSCManager::getVersion
	/// \return
	///
	SimConnector::SIM_VERSION CSCManager::getVersion()
	{
		if (!m_connected) return SimConnector::SIM_VERSION::UNKNOWN;
		switch (m_appVersionMajor)
		{
			case 2:
				return SimConnector::SIM_VERSION::P3D_V2;
			case 3:
				return SimConnector::SIM_VERSION::P3D_V3;
			case 4:
				return SimConnector::SIM_VERSION::P3D_V4;
			case 10:
				return SimConnector::SIM_VERSION::FSX;
			default:
				return SimConnector::SIM_VERSION::UNKNOWN;
		}
	}

	///
	/// \brief CSCManager::process
	/// \param pData
	/// \param cbData
	///
	///
	void CSCManager::process(SIMCONNECT_RECV *pData, DWORD)
	{
		switch (pData->dwID)
		{
			case SIMCONNECT_RECV_ID_OPEN:
			{
				OnRecvOpen ((SIMCONNECT_RECV_OPEN*) pData);
				break;
			}
			case SIMCONNECT_RECV_ID_QUIT:
			{
				OnRecvQuit(pData);
				break;
			}
			case SIMCONNECT_RECV_ID_SYSTEM_STATE:
			{
				OnRecvSystemState((SIMCONNECT_RECV_SYSTEM_STATE*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_EXCEPTION:
			{
				OnRecvException((SIMCONNECT_RECV_EXCEPTION*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_EVENT:
			{
				OnRecvEvent ((SIMCONNECT_RECV_EVENT*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
			{
				OnRecvSimObjectData((SIMCONNECT_RECV_SIMOBJECT_DATA*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_EVENT_FRAME:
			{
				OnRecvEventFrame((SIMCONNECT_RECV_EVENT_FRAME*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_EVENT_OBJECT_ADDREMOVE:
			{
				OnRecvEventObjectAddremove((SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE*)pData);
				break;
			}
			case SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID:
			{
				OnRecvAssignedObjectId((SIMCONNECT_RECV_ASSIGNED_OBJECT_ID*)pData);
				break;
			}
			default:
			{
	#ifdef FFS_DEBUG
				m_log->log("SCManager : " + tr("Get an unknown Event ID"), Qt::darkMagenta,LEVEL_VERBOSE);
	#endif
				break;
			}
		}
	}

	///
	/// \brief CSCManager::InitSimConnect
	///
	void CSCManager::InitSimConnect()
	{
		HRESULT hr;
		// Create event id for joyst
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::SLIDER,"");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::XAXIS,"");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::YAXIS,"");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::RZAXIS,"");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::HAT,"");
		// Create notification for freeze action to AI
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::FREEZE_LATLONG,"FREEZE_LATITUDE_LONGITUDE_SET");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::FREEZE_ALTITUDE,"FREEZE_ALTITUDE_SET");
		hr = SimConnect_MapClientEventToSimEvent(m_hSimConnect,(SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::FREEZE_ATTITUDE,"FREEZE_ATTITUDE_SET");

		// Create notification for joystick events

		// Données périodiques
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "TITLE", NULL, SIMCONNECT_DATATYPE_STRING256);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "ATC TYPE", NULL, SIMCONNECT_DATATYPE_STRING256);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "ATC MODEL", NULL, SIMCONNECT_DATATYPE_STRING256);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "CATEGORY", NULL, SIMCONNECT_DATATYPE_STRING256);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "PLANE ALT ABOVE GROUND", "feet",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "VERTICAL SPEED", "feet per second",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "PLANE HEADING DEGREES TRUE", "degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "Plane Longitude", "degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "Plane Latitude", "degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GROUND VELOCITY", "Knots",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AIRSPEED TRUE", "Knots",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "FUEL TOTAL QUANTITY WEIGHT", "Pounds",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "FUEL TOTAL CAPACITY", "Gallons",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "FUEL WEIGHT PER GALLON", "Pounds",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "PLANE PITCH DEGREES", "degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "PLANE BANK DEGREES", "degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "G FORCE", "Gforce",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "TOTAL WEIGHT", "Pounds",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "REALISM", "Number",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AMBIENT WIND VELOCITY", "Knots",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AMBIENT WIND DIRECTION", "Degrees",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AMBIENT PRECIP STATE", "Mask",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "SEA LEVEL PRESSURE", "Millibars",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AIRSPEED INDICATED", "Knots",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "ELEVATOR POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "AILERON POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "RUDDER POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "SPOILERS HANDLE POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "BRAKE PARKING POSITION", "Position",SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "SIMULATION RATE", "Number",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "EXIT OPEN:0", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "EXIT OPEN:1", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "EXIT OPEN:2", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "EXIT OPEN:3", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "NUMBER OF ENGINES", "Number",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:1", "Bool",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:2", "Bool",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:3", "Bool",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:4", "Bool",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:1", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:2", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:3", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:4", "Percent",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PERIODIQUE, "TRANSPONDER CODE:1", "BCO16",SIMCONNECT_DATATYPE_INT32);

		// Définition de la méthode à apeller pour la récéption des données
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::PERIODIQUE, DEFINITION_ID::PERIODIQUE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME, SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT, 0, 0, 0);

		// Structure pour déplacer un AI
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE LATITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE LONGITUDE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE PITCH DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE BANK DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "PLANE HEADING DEGREES TRUE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "ELEVATOR POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "AILERON POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "RUDDER POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "SPOILERS HANDLE POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "BRAKE PARKING POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "EXIT OPEN:0", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "EXIT OPEN:1", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "EXIT OPEN:2", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "EXIT OPEN:3", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG COMBUSTION:1", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG COMBUSTION:2", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG COMBUSTION:3", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG COMBUSTION:4", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG THROTTLE LEVER POSITION:1", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG THROTTLE LEVER POSITION:2", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG THROTTLE LEVER POSITION:3", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GENERAL ENG THROTTLE LEVER POSITION:4", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "TRANSPONDER CODE:1", "BCO16", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "GEAR HANDLE POSITION", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "FLAPS HANDLE INDEX", "Number", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "LIGHT LANDING", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "LIGHT STROBE", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "LIGHT BEACON", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "LIGHT NAV", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "LIGHT RECOGNITION", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_MOVE, "SMOKE ENABLE", "Bool", SIMCONNECT_DATATYPE_INT32);

		// AI Update définition

		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_UPDATE, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_UPDATE, "GROUND ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_UPDATE, "Plane Longitude", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::AI_UPDATE, "Plane Latitude", "degrees", SIMCONNECT_DATATYPE_FLOAT64);


		// Data events on change

		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PARKING_BRAKE,"BRAKE PARKING INDICATOR", "Bool",SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::OVERSPEED, "OVERSPEED WARNING", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::STALLING, "STALL WARNING", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::CRASH, "CRASH SEQUENCE", "enum", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::SLEW, "IS SLEW ACTIVE", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::PUSHBACK, "PUSHBACK STATE", "enum", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::ON_GROUND, "SIM ON GROUND", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::GEAR_HANDLE_POSITION, "GEAR HANDLE POSITION", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::FLAPS_HANDLE_PERCENT, "FLAPS HANDLE PERCENT", "Percent", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::FLAPS_HANDLE_INDEX, "FLAPS HANDLE INDEX", "Number", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::ALTIMETER_SETTING, "KOHLSMAN SETTING MB", "Millibars", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::LANDING_LIGHT, "LIGHT LANDING", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::STROBE_LIGHT, "LIGHT STROBE", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::BEACON_LIGHT, "LIGHT BEACON", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::NAV_LIGHT, "LIGHT NAV", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::RECOGNITION_LIGHT, "LIGHT RECOGNITION", "Bool", SIMCONNECT_DATATYPE_INT32);
		hr = SimConnect_AddToDataDefinition(m_hSimConnect,DEFINITION_ID::SMOKE_ENABLE, "SMOKE ENABLE", "Bool", SIMCONNECT_DATATYPE_INT32);
		// Request data on change event
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::PARKING_BRAKE,DEFINITION_ID::PARKING_BRAKE,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::OVERSPEED,DEFINITION_ID::OVERSPEED,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::STALLING,DEFINITION_ID::STALLING,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::CRASH,DEFINITION_ID::CRASH,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::SLEW,DEFINITION_ID::SLEW,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::PUSHBACK,DEFINITION_ID::PUSHBACK,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::ON_GROUND,DEFINITION_ID::ON_GROUND,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::GEAR_HANDLE_POSITION,DEFINITION_ID::GEAR_HANDLE_POSITION,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::FLAPS_HANDLE_PERCENT,DEFINITION_ID::FLAPS_HANDLE_PERCENT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::FLAPS_HANDLE_INDEX,DEFINITION_ID::FLAPS_HANDLE_INDEX,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::ALTIMETER_SETTING,DEFINITION_ID::ALTIMETER_SETTING,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::LANDING_LIGHT,DEFINITION_ID::LANDING_LIGHT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::STROBE_LIGHT,DEFINITION_ID::STROBE_LIGHT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::BEACON_LIGHT,DEFINITION_ID::BEACON_LIGHT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::NAV_LIGHT,DEFINITION_ID::NAV_LIGHT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::RECOGNITION_LIGHT,DEFINITION_ID::RECOGNITION_LIGHT,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
		hr = SimConnect_RequestDataOnSimObject(m_hSimConnect,(SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::SMOKE_ENABLE,DEFINITION_ID::SMOKE_ENABLE,SIMCONNECT_OBJECT_ID_USER,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);

		// Subscribe to system events
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::PAUSE, "Pause");
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::SIMSTART, "SimStart");
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::SIMSTOP, "simstop");
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::FRAME, "Frame");
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::ADDED_AIRCRAFT, "ObjectAdded");
		hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::REMOVED_AIRCRAFT, "ObjectRemoved");
		hr = SimConnect_RequestSystemState(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::SYSTEM, "sim");
	}

	///
	/// \brief CSCManager::OnRecvOpen
	/// \param data
	///
	void CSCManager::OnRecvOpen (SIMCONNECT_RECV_OPEN* data)
	{
		m_log->log(tr("Connected to simulator."));
		m_appVersionMajor = (int)data->dwApplicationVersionMajor;
		InitSimConnect();
		// Keep connexion status
		m_connected = true;
		m_log->log(tr("Connected to FS."));
		emit(fireSimConnected());
	}

	///
	/// \brief CSCManager::OnRecvQuit
	/// \param data
	///
	void CSCManager::OnRecvQuit(SIMCONNECT_RECV*)
	{
	#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV event"), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		close();
		emit(fireSimDisconnected());
	}

	///
	/// \brief CSCManager::OnRecvSystemState
	/// \param data
	///
	void CSCManager::OnRecvSystemState(SIMCONNECT_RECV_SYSTEM_STATE* data)
	{
	#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_SYSTEM_STATE event"), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		SimEvent Event;
		switch (data->dwRequestID)
		{
			case REQUEST_ID::PAUSE:
			{
				Event.Evt_Id = SIM_EVENT_ID::PAUSE;
				m_state.Pause=(bool)Event.Data;
				break;
			}
			case REQUEST_ID::SIMSTART:
			{
				Event.Evt_Id = SIM_EVENT_ID::SIMSTART;
				m_state.RunState=true;
				break;
			}
			case REQUEST_ID::SIMSTOP:
			{
				Event.Evt_Id = SIM_EVENT_ID::SIMSTOP;
				m_state.RunState=false;
				break;
			}
			default:
			{
				Event.Evt_Id = SIM_EVENT_ID::UNUSED;
				break;
			}
		}
		emit(fireSimEvent(Event));
	}

	///
	/// \brief CSCManager::OnRecvException
	/// \param data
	///
	void CSCManager::OnRecvException (SIMCONNECT_RECV_EXCEPTION* data)
	{
		m_log->log("SCManager : "+ tr("Get a SimConnect exception = ") + QString::number(data->dwException) +
				   tr(" Related method invoked = ") + m_record.value(data->dwSendID),
				   Qt::red,LEVEL_VERBOSE);
	}

	///
	/// \brief CSCManager::OnRecvEvent
	/// \param data
	///
	void CSCManager::OnRecvEvent(SIMCONNECT_RECV_EVENT* data)
	{
	#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_EVENT event"), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		SimEvent Event;
		Event.Data = data->dwData;
		switch (data->uEventID)
		{
			case EVENT_ID::PAUSE:
			{
				Event.Evt_Id = SIM_EVENT_ID::PAUSE;
				m_state.Pause=(bool)Event.Data;
				break;
			}
			case EVENT_ID::SIMSTART:
			{
				Event.Evt_Id = SIM_EVENT_ID::SIMSTART;
				m_state.RunState=true;
				break;
			}
			case EVENT_ID::SIMSTOP:
			{
				Event.Evt_Id = SIM_EVENT_ID::SIMSTOP;
				m_state.RunState=false;
				break;
			}
			case EVENT_ID::TEXT_SCROLL:
			{
				if((SIMCONNECT_TEXT_RESULT)data->dwData == SIMCONNECT_TEXT_RESULT_TIMEOUT)
				{
					m_messageBuffer.removeAt(0);
					if (m_messageBuffer.count()>0) sendText(m_messageBuffer[0]);
				}
				break;
			}
			default:
			{
				Event.Evt_Id = SIM_EVENT_ID::UNUSED;
				break;
			}
		}
		emit(fireSimEvent(Event));
	}

	///
	/// \brief CSCManager::OnRecvSimObjectData
	/// \param data
	///
	void CSCManager::OnRecvSimObjectData (SIMCONNECT_RECV_SIMOBJECT_DATA* data)
	{
		switch (data->dwRequestID)
		{
			case REQUEST_ID::PERIODIQUE:
			{
#ifdef FFS_DEBUG
				m_log->log("SCManager : " + tr("OnRecvSimObjectData Periodic"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
				UserState* Data =  (UserState*)(&data->dwData);
				m_state.TimeStamp = Now();
				m_state.AileronPos = Data->AileronPos;
				m_state.AmbiantPrecipState = Data->AmbiantPrecipState;
				m_state.AmbiantWindDirection = Data->AmbiantWindDirection;
				m_state.AmbiantWindVelocity = Data->AmbiantWindVelocity;
				m_state.Category = Data->Category;
				m_state.Door1Pos = Data->Door1Pos;
				m_state.Door2Pos = Data->Door2Pos;
				m_state.Door3Pos = Data->Door3Pos;
				m_state.Door4Pos = Data->Door4Pos;
				m_state.ElevatorPos = Data->ElevatorPos;
				m_state.Fuel = Data->FuelQty;
				m_state.GForce = Data->GForce;
				m_state.GroundAltitude = Data->GndAltitude;
				m_state.GroundSpeed = Data->GndSpeed;
				m_state.IASSpeed = Data->IASSpeed;
				m_state.Latitude = Data->Latitude;
				m_state.Longitude = Data->Longitude;
				m_state.Model = Data->Model;
				m_state.NumEngine = Data->NumEngine;
				m_state.Altitude = Data->PlaneAltitude;
				m_state.Bank = Data->PlaneBank;
				m_state.Heading = Data->PlaneHeading;
				m_state.Pitch = Data->PlanePitch;
				m_state.Weight = Data->PlaneWeight;
				m_state.Realism = Data->Realism;
				m_state.RudderPos = Data->RudderPos;
				m_state.SeaLevelPressure = Data->SeaLevelPressure;
				m_state.TASSpeed = Data->TASSpeed;
				m_state.ThrottleEng1 = Data->ThrottleEng1;
				m_state.ThrottleEng2 = Data->ThrottleEng2;
				m_state.ThrottleEng3 = Data->ThrottleEng3;
				m_state.ThrottleEng4 = Data->ThrottleEng4;
				m_state.TimeFactor = Data->TimeFactor;
				m_state.Title = Data->Title;
				m_state.TotalFuelCapacity = Data->TotalFuelCapacity;
				m_state.Squawk = Data->Squawk;
				m_state.Type = Data->Type;
				m_state.Vario = Data->Vario*60;
				emit(fireSimVariableEvent(m_state));
				break;
			}
			default:
			{
				if (data->dwRequestID >= (DWORD)(REQUEST_ID::AI_UPDATE))
				{
#ifdef FFS_DEBUG
					m_log->log("SCManager : " + tr("OnRecvSimObjectData AIUpdate ObjectID = ") + QString::number(data->dwRequestID-(DWORD)(REQUEST_ID::AI_UPDATE)), Qt::darkBlue,LEVEL_VERBOSE);
#endif
					AIUpdateStruct* Data =  (AIUpdateStruct*)(&data->dwData);
					CAircraftState AIData;
					AIData.Altitude = Data->AvionAltitude;
					AIData.GroundAltitude = Data->SolAltitude;
					AIData.Longitude = Data->Longitude;
					AIData.Latitude = Data->Latitude;
					emit(fireSimAIUpdated(AIData,data->dwObjectID));
				}
				else
				{
					SimEvent Event;
					Event.Data = data->dwData;
					switch (data->dwRequestID)
					{
						case REQUEST_ID::PARKING_BRAKE:
						{
							Event.Evt_Id = SIM_EVENT_ID::PARKING_BRAKE;
							m_state.ParkingBrake=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::ALTIMETER_SETTING:
						{
							Event.Evt_Id = SIM_EVENT_ID::ALTIMETER_SETTING;
							m_state.AltimeterSetting=Event.Data;
							break;
						}
						case REQUEST_ID::BEACON_LIGHT:
						{
							Event.Evt_Id = SIM_EVENT_ID::BEACON_LIGHT;
							m_state.BeaconLight=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::CRASH:
						{
							Event.Evt_Id = SIM_EVENT_ID::CRASH;
							m_state.Crash=(bool)(Event.Data==11);
							break;
						}
						case REQUEST_ID::FLAPS_HANDLE_INDEX:
						{
							Event.Evt_Id = SIM_EVENT_ID::FLAPS_HANDLE_INDEX;
							m_state.FlapsIndex=Event.Data;
							break;
						}
						case REQUEST_ID::FLAPS_HANDLE_PERCENT:
						{
							Event.Evt_Id = SIM_EVENT_ID::FLAPS_HANDLE_PERCENT;
							break;
						}
						case REQUEST_ID::GEAR_HANDLE_POSITION:
						{
							Event.Evt_Id = SIM_EVENT_ID::GEAR_HANDLE_POSITION;
							m_state.GearPos=Event.Data;
							break;
						}
						case REQUEST_ID::LANDING_LIGHT:
						{
							Event.Evt_Id = SIM_EVENT_ID::LANDING_LIGHT;
							m_state.LandingLight=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::NAV_LIGHT:
						{
							Event.Evt_Id = SIM_EVENT_ID::NAV_LIGHT;
							m_state.NavLight=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::ON_GROUND:
						{
							Event.Evt_Id = SIM_EVENT_ID::ON_GROUND;
							m_state.OnGround=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::OVERSPEED:
						{
							Event.Evt_Id = SIM_EVENT_ID::OVERSPEED;
							m_state.OverSpeed=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::PUSHBACK:
						{
							Event.Evt_Id = SIM_EVENT_ID::PUSHBACK;
							m_state.Pushback=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::RECOGNITION_LIGHT:
						{
							Event.Evt_Id = SIM_EVENT_ID::RECOGNITION_LIGHT;
							m_state.RecoLight=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::SLEW:
						{
							Event.Evt_Id = SIM_EVENT_ID::SLEW_ON;
							m_state.Slew=(bool)Event.Data;
							break;
						}
						case REQUEST_ID::SMOKE_ENABLE:
						{
							Event.Evt_Id = SIM_EVENT_ID::SMOKE_ENABLE;
							m_state.Smoke=(bool)Event.Data;
							break;
						}
						default:
						{
							Event.Evt_Id = SIM_EVENT_ID::UNUSED;
							break;
						}
					}
					emit(fireSimEvent(Event));
				}
				break;
			}
		}
	}

	///
	/// \brief CSCManager::OnRecvEventFrame
	/// \param data
	///
	void CSCManager::OnRecvEventFrame(SIMCONNECT_RECV_EVENT_FRAME* data)
	{
		emit(fireSimEventFrame(data->fFrameRate, data->fSimSpeed));
	}

	///
	/// \brief CSCManager::OnRecvEventObjectAddremove
	/// \param data
	///
	void CSCManager::OnRecvEventObjectAddremove(SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* data)
	{
		switch((EVENT_ID)data->uEventID)
		{
			case EVENT_ID::ADDED_AIRCRAFT:
			{
#ifdef FFS_DEBUG
				m_log->log("SCManager : " + tr("Object as been created type = ") + QString::number(data->eObjType)
						+ tr(" with n°") + QString::number(data->dwData)
					   ,Qt::darkBlue,LEVEL_NORMAL);
#endif
				break;
			}
			case EVENT_ID::REMOVED_AIRCRAFT:
			{
#ifdef FFS_DEBUG
				m_log->log("SCManager : " + tr("Object as been removed type = ") + QString::number(data->eObjType)
						+ tr(" with n°") + QString::number(data->dwData)
					   ,Qt::darkBlue,LEVEL_NORMAL);
#endif
				fireSimAIRemoved(data->dwData);
				break;
			}
		}
	}

	///
	/// \brief CSCManager::OnRecvAssignedObjectId
	/// \param data
	///
	void CSCManager::OnRecvAssignedObjectId(SIMCONNECT_RECV_ASSIGNED_OBJECT_ID* data)
	{
		QString Tail;
		QHash<uint,QString>::iterator it = m_AIProcess.find(data->dwRequestID);
		if (it != m_AIProcess.end())
		{
#ifdef FFS_DEBUG
			m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_ASSIGNED_OBJECT_ID Request = ") + QString::number(data->dwRequestID) +
					   tr(" , Name = ") + it.value() +
					   tr(" , Object_ID = ") + QString::number(data->dwObjectID),
					   Qt::darkBlue,LEVEL_NORMAL);
#endif
			SimConnect_AIReleaseControl(m_hSimConnect,data->dwObjectID, (SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::AI_RELEASE);
			trace("SimConnect_AIReleaseControl:679");
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ATTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			trace("SimConnect_TransmitClientEvent:681");
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			trace("SimConnect_TransmitClientEvent:684");
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			trace("SimConnect_TransmitClientEvent:686");
			emit(fireSimAICreated(it.value(),data->dwObjectID));
			m_AIProcess.remove(data->dwRequestID);
			SimConnect_RequestDataOnSimObject(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)((uint)REQUEST_ID::AI_UPDATE + data->dwObjectID),DEFINITION_ID::AI_UPDATE,data->dwObjectID,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT,0,0,0);
			trace("SimConnect_RequestDataOnSimObject:701");
		}
		else
		{
			m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_ASSIGNED_OBJECT_ID with unknown request ID  = ") + QString::number(data->dwRequestID), Qt::darkMagenta);
		}
	}

	///
	/// \brief CSCManager::sendScrollingText
	/// \param pMessage
	///
	void CSCManager::sendScrollingText(const QString& pMessage)
	{
		if(m_connected)
		{
			m_messageBuffer.append(pMessage);
			if (m_messageBuffer.count()<2) sendText(pMessage);
		}
	}

	///
	/// \brief CSCManager::sendText
	/// \param pMessage
	///
	void CSCManager::sendText(const QString &pMessage)
	{
		if(m_connected)
		{
			HRESULT result = SimConnect_Text(m_hSimConnect,SIMCONNECT_TEXT_TYPE_SCROLL_WHITE,m_timeToScroll,(SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::TEXT_SCROLL, pMessage.length()+1,(void*)pMessage.toStdString().c_str());
			trace("SimConnect_Text:702");
			if (result == E_FAIL)
			{
				m_log->log("SCManager: " + tr("Send Text Failed"),Qt::darkMagenta);
			}
		}
	}

	///
	/// \brief CSCManager::createAI
	/// \param pAircraft
	/// \param pTail
	/// \param pData
	/// \param pNonAIObject
	/// \return
	///
	bool CSCManager::createAI(AIResol &pAircraft, const QString &pTail, const CAircraftState &pData,bool pNonAIObject)
	{
		if (m_connected)
		{
#ifdef FFS_DEBUG
			m_log->log("SCManager : " + tr("Create_AI Title = ")+ QString::fromStdString(pAircraft.title()) + tr(" ; Tail = ")+pTail + tr(" ; NonAIObject=") + QString::number(pNonAIObject), Qt::darkBlue,LEVEL_NORMAL);
#endif
			SIMCONNECT_DATA_INITPOSITION data;
			data.Altitude = pData.Altitude;
			data.Latitude = pData.Latitude;
			data.Longitude = pData.Longitude;
			data.Heading = pData.Heading;
			data.Pitch = pData.Pitch;
			data.Bank = pData.Bank;
			data.OnGround = pData.OnGround;
			data.Airspeed = (DWORD) pData.IASSpeed;
			uint index = (uint)m_AIProcess.count()+ (uint)REQUEST_ID::AI_CREATE;
			m_AIProcess.insert(index,pTail);
			HRESULT result;
			if (!pNonAIObject)
			{
				result = SimConnect_AICreateNonATCAircraft(m_hSimConnect,
															pAircraft.title().c_str(),
															pTail.toStdString().c_str(),
															data,
															(SIMCONNECT_DATA_REQUEST_ID)index);
				trace("SimConnect_AICreateNonATCAircraft:741");
			}
			else
			{
				result = SimConnect_AICreateSimulatedObject(m_hSimConnect,
															   pAircraft.title().c_str(),
															   data,
															   (SIMCONNECT_DATA_REQUEST_ID)index);
				trace("SimConnect_AICreateSimulatedObject:750");
			}
			if (result==S_OK) return true;
		}
		return false;
	}

	bool CSCManager::removeAI(uint& pObjectID)
	{
#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("removeAI called object ID = ") + QString::number(pObjectID), Qt::darkBlue,LEVEL_NORMAL);
#endif
		if (m_connected)
		{
			SimConnect_AIRemoveObject(m_hSimConnect,pObjectID,(SIMCONNECT_DATA_REQUEST_ID)((uint)REQUEST_ID::AI_UPDATE + pObjectID));
			pObjectID = 0;
			trace("SimConnect_AIRemoveObject:768");
			return true;
		}
		m_log->log("SCManager : " + tr("Delete Aircrfaft failed"),Qt::darkMagenta);
		return false;
	}

	bool CSCManager::updateAI(uint pObjectID, const CAircraftState& pData)
	{
#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("updateAI called"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
		if (m_connected && (pObjectID>0))
		{
			AIMoveStruct MoveData;
			MoveData.AileronPos = pData.AileronPos;
			MoveData.Altitude = pData.Altitude;
			MoveData.Bank = pData.Bank;
			MoveData.BeaconLight = pData.BeaconLight;
			MoveData.Door1Pos = pData.Door1Pos;
			MoveData.Door2Pos = pData.Door2Pos;
			MoveData.Door3Pos = pData.Door3Pos;
			MoveData.Door4Pos = pData.Door4Pos;
			MoveData.ElevatorPos = pData.ElevatorPos;
			MoveData.FlapsIndex = pData.FlapsIndex;
			MoveData.GearPos = pData.GearPos;
			MoveData.Heading = pData.Heading;
			MoveData.LandingLight = pData.LandingLight;
			MoveData.Latitude = pData.Latitude;
			MoveData.Longitude = pData.Longitude;
			MoveData.NavLight = pData.NavLight;
			MoveData.ParkingBrakePos = pData.ParkingBrakePos;
			MoveData.Pitch = pData.Pitch;
			MoveData.RecoLight = pData.RecoLight;
			MoveData.RudderPos = pData.RudderPos;
			MoveData.Smoke = pData.Smoke;
			MoveData.SpoilerPos = pData.SpoilerPos;
			MoveData.Squawk = pData.Squawk;
			MoveData.StateEng1 = pData.StateEng1;
			MoveData.StateEng2 = pData.StateEng2;
			MoveData.StateEng3 = pData.StateEng3;
			MoveData.StateEng4 = pData.StateEng4;
			MoveData.StrobeLight = pData.StrobeLight;
			MoveData.ThrottleEng1 = pData.ThrottleEng1;
			MoveData.ThrottleEng2 = pData.ThrottleEng2;
			MoveData.ThrottleEng3 = pData.ThrottleEng3;
			MoveData.ThrottleEng4 = pData.ThrottleEng4;
			SimConnect_SetDataOnSimObject(m_hSimConnect, (SIMCONNECT_DATA_DEFINITION_ID)DEFINITION_ID::AI_MOVE,pObjectID,SIMCONNECT_DATA_SET_FLAG_DEFAULT,0,sizeof(AIMoveStruct),&MoveData);
			trace("SimConnect_SetDataOnSimObject:816");
			return true;
		}
		return false;
	}

	void CSCManager::freezeAI(uint pObjectID, bool pState)
	{
#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("freezeAI called"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
		if (m_connected && (pObjectID > 0))
		{
			if (pState)
			{
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				trace("SimConnect_TransmitClientEvent:832");
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				trace("SimConnect_TransmitClientEvent:834");
			}
			else
			{
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 0,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				trace("SimConnect_TransmitClientEvent:839");
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 0,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				trace("SimConnect_TransmitClientEvent:841");
			}
		}

	}

	void CSCManager::trace(const QString &pMessage)
	{
		DWORD id;
		SimConnect_GetLastSentPacketID(m_hSimConnect,&id);
		m_record.insert(id,pMessage);
		if (m_record.count()>100)
		{
			m_record.erase(m_record.begin());
		}
	}
}
