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

#ifdef FFS_WITH_SC

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
	}

	CSCManager::~CSCManager()
	{
		close();
	}

	///
	/// \brief CSCManager::open
	///

	bool CSCManager::open ()
	{
		if (!m_connected)
		{
			HRESULT hr;
			if (SUCCEEDED (hr = SimConnect_Open (&m_hSimConnect,QCoreApplication::applicationName().toStdString().c_str(),NULL,NULL, (HANDLE)WM_USER_SIMCONNECT,0)))
			{
				// Keep connexion status
				m_connected = true;
				emit(opened());
				return true;
			}
			else
			{
				m_log->log(tr("Connexion to FS failed."),Qt::darkMagenta);
			}
		}
		return false;
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
			emit(closed());
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
		hr=SimConnect_CallDispatch(m_hSimConnect,CSCManager::DispatchCallback,this);
	}

	///
	/// \brief CSCManager::getVersion
	/// \return
	///

	SIM_VERSION CSCManager::getVersion()
	{
		if (!m_connected) return SIM_VERSION::UNKNOWN;
		switch (m_appVersionMajor)
		{
			case 2:
				return SIM_VERSION::P3D_V2;
			case 3:
				return SIM_VERSION::P3D_V3;
			case 4:
				return SIM_VERSION::P3D_V4;
			case 10:
				return SIM_VERSION::FSX;
			default:
				return SIM_VERSION::FSX_STEAM;
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

	void CSCManager::OnRecvOpen (SIMCONNECT_RECV_OPEN*)
	{
		m_log->log(tr("Connected to simulator."));
		InitSimConnect();
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
		m_log->log(tr("Get a SimConnect exception = ") + QString::number(data->dwException),Qt::red);
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
	/// \brief CSCManager::OnRecvSimObjectData
	/// \param data
	///

	void CSCManager::OnRecvSimObjectData (SIMCONNECT_RECV_SIMOBJECT_DATA* data)
	{
		switch (data->dwRequestID)
		{
			case REQUEST_ID::PERIODIQUE:
			{
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
			case REQUEST_ID::AI_UPDATE:
			{
				//fireSCReceiveAIUpdate
				break;
			}
			default:
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

	}

	///
	/// \brief CSCManager::OnRecvEventObjectAddremove
	/// \param data
	///

	void CSCManager::OnRecvEventObjectAddremove(SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* data)
	{
	#ifdef FFS_DEBUG
		m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE event"), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
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
					   Qt::darkBlue,LEVEL_VERBOSE);
#endif
			SimConnect_AIReleaseControl(m_hSimConnect,data->dwObjectID, (SIMCONNECT_DATA_REQUEST_ID)REQUEST_ID::AI_RELEASE);
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ATTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			SimConnect_TransmitClientEvent(m_hSimConnect ,data->dwObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

			emit(fireSimAICreated(it.value(),data->dwObjectID));
			m_AIProcess.remove(data->dwRequestID);
			SimConnect_RequestDataOnSimObject(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)((uint)REQUEST_ID::AI_UPDATE + data->dwObjectID),DEFINITION_ID::AI_UPDATE,data->dwObjectID,SIMCONNECT_PERIOD_SIM_FRAME,SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT,0,0,0);
		}
		else
		{
			m_log->log("SCManager : " + tr("Receive SIMCONNECT_RECV_ASSIGNED_OBJECT_ID with unknown request ID  = ") + QString::number(data->dwRequestID), Qt::darkMagenta);
		}
	}

	void CSCManager::sendScrollingText(const QString& pMessage)
	{
		if(m_connected)
		{
			m_messageBuffer.append(pMessage);
			if (m_messageBuffer.count()<2) sendText(pMessage);
		}
	}

	void CSCManager::sendText(const QString &pMessage)
	{
		if(m_connected)
		{
			SimConnect_Text(m_hSimConnect,SIMCONNECT_TEXT_TYPE_SCROLL_WHITE,m_timeToScroll,(SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::TEXT_SCROLL, pMessage.size(),(void*)pMessage.toStdString().c_str());
		}
	}

	bool CSCManager::createAI(const QString &pAircraft, const QString &pTail, const CAircraftState &pData)
	{
		if (m_connected)
		{
#ifdef FFS_DEBUG
			m_log->log("SCManager : " + tr("Create_AI Title = ")+ pAircraft + tr(" ; Tail = ")+pTail, Qt::darkBlue,LEVEL_NORMAL);
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
			if (pData.Category != "Helicopter")
				result = SimConnect_AICreateNonATCAircraft(m_hSimConnect,
															pAircraft.toStdString().c_str(),
															pTail.toStdString().c_str(),
															data,
															(SIMCONNECT_DATA_REQUEST_ID)index);
			else
				result = SimConnect_AICreateSimulatedObject(m_hSimConnect,
															   pAircraft.toStdString().c_str(),
															   data,
															   (SIMCONNECT_DATA_REQUEST_ID)index);
			if (result==S_OK) return true;
		}
		return false;
	}

	bool CSCManager::removeAI(uint& pObjectID)
	{
		if (m_connected)
		{
			HRESULT result = SimConnect_AIRemoveObject(m_hSimConnect,pObjectID,(SIMCONNECT_DATA_REQUEST_ID)((uint)REQUEST_ID::AI_UPDATE + pObjectID));
			if (result==S_OK) return true;
		}
		m_log->log("SCManager: " + tr("Delete Aircrfaft failed"),Qt::darkMagenta);
		return false;
	}

	void CSCManager::freezeAI(uint pObjectID, bool pState)
	{
		if (m_connected && (pObjectID > 0))
		{
			if (pState)
			{
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 1,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			}
			else
			{
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_LATLONG, 0,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				SimConnect_TransmitClientEvent(m_hSimConnect ,pObjectID, (SIMCONNECT_CLIENT_EVENT_ID) EVENT_ID::FREEZE_ALTITUDE, 0,SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			}
		}

	}
}
#endif //FFS_WITH_SC
