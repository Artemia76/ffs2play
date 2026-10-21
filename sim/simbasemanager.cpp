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

/****************************************************************************
** simbasemanager.cpp is part of FF2Play project
**
** This singleton provide simulator manager interface
** It working friendly
****************************************************************************/

#include "simbasemanager.h"
#include "tools/singleton.h"

///
/// \brief CSCManager::CSCManager
/// \param parent
///
simbasemanager::simbasemanager(QObject* parent):
    QObject(parent)
{
    qRegisterMetaType<SimEvent>("SimEvent");
    qRegisterMetaType<CAircraftState>("CAircraftState");
    qRegisterMetaType<SIM_TYPE>("SIM_TYPE");
    m_initialized = false;
#ifdef SIMCONNECT_BUILD
    m_simAvailable.append(SIM_TYPE::FSX);
#endif
    m_simAvailable.append(SIM_TYPE::XPLANE);
    m_simAvailable.append(SIM_TYPE::FLIGHTGEAR);
    m_settings.beginGroup("Sim");
    m_simType = static_cast<SIM_TYPE>(m_settings.value("DefaultSim",static_cast<int>(m_simAvailable.first())).toInt());
    m_settings.endGroup();
    if (!m_simAvailable.contains(m_simType))
    {
        m_simType=m_simAvailable.first();
    }
    // Retry Connexion in case of lost connexion from simulator
    m_RetryTimer.setSingleShot(true);
    m_RetryTimer.setInterval(5000);
    m_RetryCounter = 0;
    connect(&m_RetryTimer, &QTimer::timeout, this, &simbasemanager::OnRetryTimer);
    load(m_simType);
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
    unload();
}

///
/// \brief CSCManager::instance
/// \return
///
simbasemanager& simbasemanager::instance()
{
    return Singleton<simbasemanager>::instance(simbasemanager::createInstance);
}

///
/// \brief simbasemanager::load
/// \param pSimType
///
void simbasemanager::load(SIM_TYPE pSimType)
{
    unload();
    switch (pSimType)
    {
        case SIM_TYPE::FSX:
        {
#ifdef SIMCONNECT_BUILD
            m_connector = std::make_unique<CSCManager>();
#endif
            break;
        }
        case SIM_TYPE::XPLANE:
        {
            m_connector = std::make_unique<CXPManager>();
            break;
        }
        case SIM_TYPE::FLIGHTGEAR:
        {
            m_connector = std::make_unique<CFGManager>();
            break;
        }
    }
    if (!m_connector) return;
    m_initialized=true;
    SimConnector* Connector = m_connector.get();
    connect(Connector, &SimConnector::fireSimEvent, this, &simbasemanager::OnSimEvent);
    connect(Connector, &SimConnector::fireSimVariableEvent, this, &simbasemanager::OnSimVariableEvent);
    connect(Connector, &SimConnector::fireSimEventFrame, this, &simbasemanager::OnSimEventFrame);
    connect(Connector, &SimConnector::fireSimAICreated, this, &simbasemanager::OnSimAICreated);
    connect(Connector, &SimConnector::fireSimAIUpdated, this, &simbasemanager::OnSimAIUpdated);
    connect(Connector, &SimConnector::fireSimAIRemoved, this, &simbasemanager::OnSimAIRemoved);
    connect(Connector, &SimConnector::fireSimConnected, this, &simbasemanager::OnSimConnected);
    connect(Connector, &SimConnector::fireSimDisconnected, this, &simbasemanager::OnSimDisconnected);
    connect(Connector, &SimConnector::fireSimTimedOut, this, &simbasemanager::OnSimTimedOut);
    emit loaded(m_simType);
}

///
/// \brief simbasemanager::unload
///
void simbasemanager::unload()
{
    if (!m_initialized) return;
    if (m_connector!=nullptr)
    {
        close();
        // Destroying the connector also removes its connections
        m_connector.reset();
    }
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
        m_connector->open();
        if (m_RetryCounter>2) m_RetryCounter=0;
    }
}

///
/// \brief CSCManager::close
///
void simbasemanager::close ()
{
    // Also releases a connection still waiting for the simulator answer
    if (m_connector!=nullptr) m_connector->close();
    emit closed();
}

///
/// \brief simbasemanager::isInitialized
/// \return
///
bool simbasemanager::isInitialized()
{
    if (m_connector==nullptr) return false;
    return m_initialized;
}

///
/// \brief simbasemanager::isConnected
/// \return
///
bool simbasemanager::isConnected()
{
    if (m_connector==nullptr) return false;
    return m_connector->isConnected();
}

///
/// \brief simbasemanager::getAircraftState
/// \return
///
const CAircraftState simbasemanager::getAircraftState()
{
    CAircraftState State;
    if (m_initialized && (m_connector!=nullptr))
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

///
/// \brief simbasemanager::getVersion
/// \return
///
SimConnector::SIM_VERSION simbasemanager::getVersion()
{
    SimConnector::SIM_VERSION Version = SimConnector::SIM_VERSION::UNKNOWN;
    if (m_initialized && (m_connector!=nullptr))
    {
        Version = m_connector->getVersion();
    }
    return Version;
}

///
/// \brief simbasemanager::getType
/// \return
///
SIM_TYPE simbasemanager::getType()
{
    return m_simType;
}

///
/// \brief simbasemanager::getSimList
/// \return
///
const QList<SIM_TYPE>& simbasemanager::getSimList()
{
    return m_simAvailable;
}

///
/// \brief simbasemanager::setSimType
/// \param pSimType
///
void simbasemanager::setSimType(SIM_TYPE pSimType)
{
    if (!m_simAvailable.contains(pSimType)) return;
    m_simType = pSimType;
    m_settings.beginGroup("Sim");
    m_settings.setValue("DefaultSim",static_cast<int>(m_simType));
    m_settings.endGroup();
    if (m_initialized) load(m_simType);
}

///
/// \brief simbasemanager::createAI
/// \param pAircraft
/// \param pTail
/// \param pData
/// \param pNonAIObject
/// \return
///
bool simbasemanager::createAI(AIResol& pAircraft, const QString& pTail, const CAircraftState& pData,bool pNonAIObject)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        return m_connector->createAI(pAircraft,pTail,pData,pNonAIObject);
    }
    return false;
}

///
/// \brief simbasemanager::updateAI
/// \param pObjectID
/// \param pData
/// \return
///
bool simbasemanager::updateAI(uint pObjectID, const CAircraftState &pData)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        return m_connector->updateAI(pObjectID,pData);
    }
    return false;
}

///
/// \brief simbasemanager::removeAI
/// \param pObjectID
/// \return
///
bool simbasemanager::removeAI(uint& pObjectID)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        return m_connector->removeAI(pObjectID);
    }
    return false;
}

///
/// \brief simbasemanager::freezeAI
/// \param pObjectID
/// \param pState
///
void simbasemanager::freezeAI(uint pObjectID, bool pState)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        m_connector->freezeAI(pObjectID,pState);
    }
}

///
/// \brief simbasemanager::OnSimVariableEvent
/// \param pState
///
void simbasemanager::OnSimVariableEvent(CAircraftState pState)
{
    emit fireSimVariableEvent(pState);
}

///
/// \brief simbasemanager::OnSimEvent
/// \param pEvent
///
void simbasemanager::OnSimEvent(SimEvent pEvent)
{
    emit fireSimEvent(pEvent);
}

///
/// \brief simbasemanager::OnSimAICreated
/// \param pTail
/// \param pObjectID
///
void simbasemanager::OnSimAICreated(QString pTail, uint pObjectID)
{
    emit fireSimAICreated(pTail,pObjectID);
}

///
/// \brief simbasemanager::OnSimAIUpdated
/// \param pAIData
/// \param pObjectID
///
void simbasemanager::OnSimAIUpdated(CAircraftState pAIData, uint pObjectID)
{
    emit fireSimAIUpdated(pAIData,pObjectID);
}

///
/// \brief simbasemanager::OnSimAIRemoved
/// \param pObjectID
///
void simbasemanager::OnSimAIRemoved(uint pObjectID)
{
    emit fireSimAIRemoved(pObjectID);
}

///
/// \brief simbasemanager::OnSimEventFrame
/// \param pFramRate
/// \param SimSpeed
///
void simbasemanager::OnSimEventFrame(float pFramRate, float SimSpeed)
{
    emit fireSimEventFrame(pFramRate, SimSpeed);
}

///
/// \brief simbasemanager::OnSimConnected
///
void simbasemanager::OnSimConnected()
{
    emit opened();
}

///
/// \brief simbasemanager::OnSimDisconnected
///
void simbasemanager::OnSimDisconnected()
{
    m_log.log(tr("Disconnected from simulator."),Qt::darkMagenta);
    close();
}

///
/// \brief simbasemanager::OnSimTimedOut
///
void simbasemanager::OnSimTimedOut()
{
    m_log.log(tr("Connexion to simulator timed out."),Qt::darkMagenta);
    close();
    if (++m_RetryCounter<3)
    {
        m_RetryTimer.start();
    }
}

///
/// \brief simbasemanager::setOptionChange
/// \param pOption
/// \param pFlag
///
void simbasemanager::setOptionChange(SimOption pOption,bool pFlag)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        m_connector->setOptionChange(pOption,pFlag);
    }
}

///
/// \brief simbasemanager::getOption
/// \param pOption
/// \return
///
bool simbasemanager::getOption(SimOption pOption)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        return m_connector->getOption(pOption);
    }
    return false;
}

sVersion simbasemanager::getPlugVersion()
{
    sVersion version;
    if (m_initialized && (m_connector!=nullptr))
    {
        version = m_connector->m_version;
    }
    return version;
}

QColor simbasemanager::getCallSignColor()
{
    QColor color;
    if (m_initialized && (m_connector!=nullptr))
    {
        color = m_connector->getCallSignColor();
    }
    return color;
}

void simbasemanager::setCallSignColor(const QColor& pColor)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        m_connector->setCallSignColor(pColor);
    }
}

void simbasemanager::OnRetryTimer()
{
    open();
}

void simbasemanager::setPathList(const QStringList &pPaths)
{
    if (m_initialized && (m_connector!=nullptr))
    {
        m_connector->setAIPaths(pPaths);
    }
}
