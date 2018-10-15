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
 * simbasemanager.cpp is part of FF2Play project
 *
 * This singleton provide simulator manager interface
 * It working friendly
 * **************************************************************************/

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
        qRegisterMetaType<SIM_TYPE>("SIM_TYPE");
        m_log = CLogger::instance();
        m_connector =nullptr;
        m_initialized = false;
        m_bRunning = false;
#ifdef Q_OS_WIN
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
        m_thread = new QThread();
        //this->moveToThread(m_thread);
        connect(m_thread, SIGNAL(started()), this, SLOT(mainThread()),Qt::DirectConnection);
        connect(this, SIGNAL(finished()), m_thread, SLOT(quit()), Qt::DirectConnection );
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
    simbasemanager* simbasemanager::instance()
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
#ifdef Q_OS_WIN
            case SIM_TYPE::FSX:
            {
                m_connector = new CSCManager(this);
                break;
            }
#endif
            case SIM_TYPE::XPLANE:
            {
                m_connector = new CXPManager(this);
                break;
            }

            case SIM_TYPE::FLIGHTGEAR:
            {
                m_connector = new CFGManager(this);
                break;
            }
        }
        m_initialized=true;
        connect(m_connector,SIGNAL(fireSimEvent(SimEvent)),this,SLOT(OnSimEvent(SimEvent)));
        connect(m_connector,SIGNAL(fireSimVariableEvent(CAircraftState)),this,SLOT(OnSimVariableEvent(CAircraftState)));
        connect(m_connector,SIGNAL(fireSimEventFrame(float,float)),this,SLOT(OnSimEventFrame(float,float)));
        connect(m_connector,SIGNAL(fireSimAICreated(QString,uint)),this,SLOT(OnSimAICreated(QString,uint)));
        connect(m_connector,SIGNAL(fireSimAIUpdated(CAircraftState,uint)),this,SLOT(OnSimAIUpdated(CAircraftState,uint)));
        connect(m_connector,SIGNAL(fireSimAIRemoved(uint)),this,SLOT(OnSimAIRemoved(uint)));
        connect(m_connector,SIGNAL(fireSimConnected()),this,SLOT(OnSimConnected()));
        connect(m_connector,SIGNAL(fireSimDisconnected()),this,SLOT(OnSimDisconnected()));
        connect(m_connector,SIGNAL(fireSimTimedOut()),this,SLOT(OnSimTimedOut()));
        emit(loaded(m_simType));
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
            disconnect(m_connector,SIGNAL(fireSimEvent(SimEvent)),this,SLOT(OnSimEvent(SimEvent)));
            disconnect(m_connector,SIGNAL(fireSimVariableEvent(CAircraftState)),this,SLOT(OnSimVariableEvent(CAircraftState)));
            disconnect(m_connector,SIGNAL(fireSimEventFrame(float,float)),this,SLOT(OnSimEventFrame(float,float)));
            disconnect(m_connector,SIGNAL(fireSimAICreated(QString,uint)),this,SLOT(OnSimAICreated(QString,uint)));
            disconnect(m_connector,SIGNAL(fireSimAIUpdated(CAircraftState,uint)),this,SLOT(OnSimAIUpdated(CAircraftState,uint)));
            disconnect(m_connector,SIGNAL(fireSimAIRemoved(uint)),this,SLOT(OnSimAIRemoved(uint)));
            disconnect(m_connector,SIGNAL(fireSimConnected()),this,SLOT(OnSimConnected()));
            disconnect(m_connector,SIGNAL(fireSimDisconnected()),this,SLOT(OnSimDisconnected()));
            disconnect(m_connector,SIGNAL(fireSimTimedOut()),this,SLOT(OnSimTimedOut()));
            delete m_connector;
            m_connector=nullptr;
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
            // Thread starting
            m_thread->start();
        }
    }

    ///
    /// \brief CSCManager::close
    ///
    void simbasemanager::close ()
    {
        if (!m_bRunning) return;
        m_bRunning=false;
        m_thread->wait(0);
        if (m_connector->isConnected())
        {
            m_connector->close();
        }
        emit(closed());
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
    /// \brief CSCManager::mainThread
    ///
    void simbasemanager::mainThread()
    {
        m_bRunning=true;
        while (m_bRunning)
        {
            m_connector->main_loop();
            //QApplication::processEvents();
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
        emit(fireSimVariableEvent(pState));
    }

    ///
    /// \brief simbasemanager::OnSimEvent
    /// \param pEvent
    ///
    void simbasemanager::OnSimEvent(SimEvent pEvent)
    {
        emit(fireSimEvent(pEvent));
    }

    ///
    /// \brief simbasemanager::OnSimAICreated
    /// \param pTail
    /// \param pObjectID
    ///
    void simbasemanager::OnSimAICreated(QString pTail, uint pObjectID)
    {
        emit(fireSimAICreated(pTail,pObjectID));
    }

    ///
    /// \brief simbasemanager::OnSimAIUpdated
    /// \param pAIData
    /// \param pObjectID
    ///
    void simbasemanager::OnSimAIUpdated(CAircraftState pAIData, uint pObjectID)
    {
        emit(fireSimAIUpdated(pAIData,pObjectID));
    }

    ///
    /// \brief simbasemanager::OnSimAIRemoved
    /// \param pObjectID
    ///
    void simbasemanager::OnSimAIRemoved(uint pObjectID)
    {
        emit(fireSimAIRemoved(pObjectID));
    }

    ///
    /// \brief simbasemanager::OnSimEventFrame
    /// \param pFramRate
    /// \param SimSpeed
    ///
    void simbasemanager::OnSimEventFrame(float pFramRate, float SimSpeed)
    {
        emit(fireSimEventFrame(pFramRate, SimSpeed));
    }

    ///
    /// \brief simbasemanager::OnSimConnected
    ///
    void simbasemanager::OnSimConnected()
    {
        emit(opened());
    }

    ///
    /// \brief simbasemanager::OnSimDisconnected
    ///
    void simbasemanager::OnSimDisconnected()
    {
        close();
    }

    ///
    /// \brief simbasemanager::OnSimTimedOut
    ///
    void simbasemanager::OnSimTimedOut()
    {
        m_log->log(tr("Connexion to simulator timed out."),Qt::darkMagenta);
        close();
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
}
