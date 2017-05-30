#ifndef SIMBASEMANAGER_H
#define SIMBASEMANAGER_H

#include <QObject>
#include <QThread>
#include "singleton/clogger.h"
#include "sim/simconnector.h"
#include "tools/ffs2core_global.h"

class FFS2CORE_EXPORT simbasemanager : public QObject
{
    Q_OBJECT
private:
static simbasemanager* createInstance();
    explicit simbasemanager(QObject *parent = nullptr);
    CLogger*    m_logger;
    bool        m_initialized;
    SimConnector* m_connector;
    QThread*    m_thread;
    bool        m_bRunning;

public:
static simbasemanager* instance();
                ~simbasemanager();
    void        init(SimConnector* p_connector);
    void        open();
    void        close();
    bool        isConnected();
    bool        isInitialized();
    const CAircraftState getAircraftState();

private slots:
    void        mainThread();
public slots:
    void        open_close();

signals:
    void        finished();
    void        opened();
    void        closed();
    void        fireSimVariableEvent();
    void        fireSimEvent(SimEvent);
};

#endif // SIMBASEMANAGER_H
