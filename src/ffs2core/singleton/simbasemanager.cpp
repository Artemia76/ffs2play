#include "simbasemanager.h"
#include "singleton/singleton.h"

///
/// \brief CSCManager::CSCManager
/// \param parent
///

simbasemanager::simbasemanager(QObject* parent):
    QObject(parent)
{
    qRegisterMetaType<SimEvent>("SimEvent");
    m_logger = CLogger::instance();
    m_connector =NULL;
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

simbasemanager* simbasemanager::instance()
{
    return Singleton<simbasemanager>::instance(simbasemanager::createInstance);
}

void simbasemanager::init(SimConnector *p_connector)
{
    if (p_connector==NULL) return;
    m_connector=p_connector;
    m_initialized=true;
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
            m_logger->log(tr("Connexion to simulator failed."),Qt::darkMagenta);
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
        m_logger->log(tr("Disconnected from simulator."));
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
        m_thread->msleep(1);
    }
    emit(finished());
}

const CAircraftState simbasemanager::getAircraftState()
{
    CAircraftState State;
    if (m_initialized && (m_connector!=NULL))
        State = m_connector->State;
    return State;
}
