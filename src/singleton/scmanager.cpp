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

#include "singleton/scmanager.h"
#include "singleton/singleton.h"

#include <QCoreApplication>

///
/// \brief CSCManager::CSCManager
/// \param parent
///

CSCManager::CSCManager(QObject* parent):
    QObject(parent)
{
    m_logger = CLogger::instance();
    // Initialisation de l'état de connexion avec le simulateur
    m_bConnected = false;
    m_hSimConnect = NULL;
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

CSCManager* CSCManager::createInstance()
{
    return new CSCManager();
}

///
/// \brief CSCManager::~CSCManager
///

CSCManager::~CSCManager()
{
    if (m_thread != NULL)
    {
        m_thread->quit();
        m_thread->wait();
    }
}

///
/// \brief CSCManager::instance
/// \return
///

CSCManager* CSCManager::instance()
{
    return Singleton<CSCManager>::instance(CSCManager::createInstance);
}

///
/// \brief CSCManager::open_close
///

void CSCManager::open_close()
{
    if (!m_bConnected) open();
    else close();
}

///
/// \brief CSCManager::open
///

void CSCManager::open ()
{
    if (!m_bConnected)
    {
        HRESULT hr;
        if (SUCCEEDED (hr = SimConnect_Open (&m_hSimConnect,QCoreApplication::applicationName().toStdString().c_str(),NULL,NULL, (HANDLE)WM_USER_SIMCONNECT,0)))
        {
            // Keep connexion status
            m_bConnected = true;
            // Thread starting
            m_thread->start();
            emit(opened());
        }
        else
        {
            m_logger->log(tr("Connexion to simulator failed."),Qt::darkMagenta);
        }
    }
}

///
/// \brief CSCManager::close
///

void CSCManager::close ()
{
    if (m_bConnected)
    {
        m_bConnected = false;
        m_thread->wait(0);
        SimConnect_Close(m_hSimConnect);
        m_logger->log(tr("Disconnected from simulator."));
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
/// \brief CSCManager::mainThread
///

void CSCManager::mainThread()
{
    HRESULT hr;
    while (m_bConnected)
    {
        hr=SimConnect_CallDispatch(m_hSimConnect,CSCManager::DispatchCallback,this);
        m_thread->msleep(1);
    }
    emit(finished());
}

///
/// \brief CSCManager::process
/// \param pData
/// \param cbData
///
///
void CSCManager::process(SIMCONNECT_RECV *pData, DWORD cbData)
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
#ifdef QT_DEBUG
            m_logger->log("SCManager : " + tr("Get an unknown Event ID"), Qt::darkMagenta,LEVEL_VERBOSE);
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
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "TITLE", NULL, SIMCONNECT_DATATYPE_STRING256);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "ATC TYPE", NULL, SIMCONNECT_DATATYPE_STRING256);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "ATC MODEL", NULL, SIMCONNECT_DATATYPE_STRING256);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "CATEGORY", NULL, SIMCONNECT_DATATYPE_STRING256);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "PLANE ALTITUDE", "feet", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "PLANE ALT ABOVE GROUND", "feet", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "VERTICAL SPEED", "feet per second", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "PLANE HEADING DEGREES TRUE", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "SIMULATION RATE", "Number", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "Plane Longitude", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "Plane Latitude", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GROUND VELOCITY", "Knots", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AIRSPEED TRUE", "Knots", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "FUEL TOTAL QUANTITY WEIGHT", "Pounds", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "FUEL TOTAL CAPACITY", "Gallons", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "FUEL WEIGHT PER GALLON", "Pounds", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "PLANE PITCH DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "PLANE BANK DEGREES", "degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "G FORCE", "Gforce", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "TOTAL WEIGHT", "Pounds", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "REALISM", "Number", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AMBIENT WIND VELOCITY", "Knots", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AMBIENT WIND DIRECTION", "Degrees", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AMBIENT PRECIP STATE", "Mask", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "SEA LEVEL PRESSURE", "Millibars", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AIRSPEED INDICATED", "Knots", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "ELEVATOR POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "AILERON POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "RUDDER POSITION", "Position", SIMCONNECT_DATATYPE_FLOAT64);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "EXIT OPEN:0", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "EXIT OPEN:1", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "EXIT OPEN:2", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "EXIT OPEN:3", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "NUMBER OF ENGINES", "Number", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:1", "Bool", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:2", "Bool", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:3", "Bool", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG COMBUSTION:4", "Bool", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:1", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:2", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:3", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "GENERAL ENG THROTTLE LEVER POSITION:4", "Percent", SIMCONNECT_DATATYPE_INT32);
    hr = SimConnect_AddToDataDefinition(m_hSimConnect,(SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, "TRANSPONDER CODE:1", "BCO16", SIMCONNECT_DATATYPE_INT32);

    // Définition de la méthode à apeller pour la récéption des données
    hr = SimConnect_RequestDataOnSimObject(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)REQUESTS_ID::PERIODIQUE, (SIMCONNECT_DATA_DEFINITION_ID)DEFINITIONS_ID::PERIODIQUE, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_VISUAL_FRAME, SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT, 0, 0, 0);


    // Subscribe to system events
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::PAUSE, "Pause");
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::SIMSTART, "SimStart");
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::SIMSTOP, "simstop");
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::FRAME, "Frame");
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::ADDED_AIRCRAFT, "ObjectAdded");
    hr = SimConnect_SubscribeToSystemEvent(m_hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)EVENT_ID::REMOVED_AIRCRAFT, "ObjectRemoved");
    hr = SimConnect_RequestSystemState(m_hSimConnect, (SIMCONNECT_DATA_REQUEST_ID)REQUESTS_ID::SYSTEM, "sim");
}

///
/// \brief CSCManager::OnRecvOpen
/// \param data
///

void CSCManager::OnRecvOpen (SIMCONNECT_RECV_OPEN* data)
{
    m_logger->log(tr("Connected to simulator."));
    InitSimConnect();
}

///
/// \brief CSCManager::OnRecvQuit
/// \param data
///

void CSCManager::OnRecvQuit(SIMCONNECT_RECV* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
    close();
}

///
/// \brief CSCManager::OnRecvSystemState
/// \param data
///

void CSCManager::OnRecvSystemState(SIMCONNECT_RECV_SYSTEM_STATE* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_SYSTEM_STATE event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief CSCManager::OnRecvException
/// \param data
///

void CSCManager::OnRecvException (SIMCONNECT_RECV_EXCEPTION* data)
{
    m_logger->log(tr("Get a SimConnect exception = ") + QString::number(data->dwException),Qt::red);
}

///
/// \brief CSCManager::OnRecvEvent
/// \param data
///

void CSCManager::OnRecvEvent(SIMCONNECT_RECV_EVENT* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_EVENT event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief CSCManager::OnRecvSimObjectData
/// \param data
///

void CSCManager::OnRecvSimObjectData (SIMCONNECT_RECV_SIMOBJECT_DATA* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_SIMOBJECT_DATA event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief CSCManager::OnRecvEventFrame
/// \param data
///

void CSCManager::OnRecvEventFrame(SIMCONNECT_RECV_EVENT_FRAME* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_EVENT_FRAME event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief CSCManager::OnRecvEventObjectAddremove
/// \param data
///

void CSCManager::OnRecvEventObjectAddremove(SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_EVENT_OBJECT_ADDREMOVE event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief CSCManager::OnRecvAssignedObjectId
/// \param data
///

void CSCManager::OnRecvAssignedObjectId(SIMCONNECT_RECV_ASSIGNED_OBJECT_ID* data)
{
#ifdef QT_DEBUG
    m_logger->log("SCManager : " + tr("Receive SIMCONNECT_RECV_ASSIGNED_OBJECT_ID event"), Qt::darkBlue,LEVEL_VERBOSE);
#endif
}
