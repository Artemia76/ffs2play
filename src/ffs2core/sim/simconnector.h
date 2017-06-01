#ifndef SIMCONNECTOR_H
#define SIMCONNECTOR_H

#include <QObject>
#include "tools/aircraftstate.h"
#include "singleton/clogger.h"
#include "tools/ffs2core_global.h"

/// <summary>
/// Enumération des évenements du simulateur à traiter
/// </summary>
enum class FFS2CORE_EXPORT SIM_EVENT_ID
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

class FFS2CORE_EXPORT SimEvent
{
public:
    SimEvent()
    {
        Time = QDateTime::currentDateTimeUtc();
    }

    SimEvent(SIM_EVENT_ID pEvt_ID, int pData )
    {
        Time = QDateTime::currentDateTimeUtc();
        Data = pData;
        Evt_Id = pEvt_ID;
    }

    SIM_EVENT_ID Evt_Id;
    int Data;
    QDateTime Time;
};

class FFS2CORE_EXPORT SimConnector : public QObject
{
    Q_OBJECT
    friend class simbasemanager;
public:
    explicit        SimConnector(QObject *parent = nullptr);
    virtual         ~SimConnector();
    bool            isConnected();
protected:
    CLogger*        Log;
    bool            m_bConnected;
    CAircraftState  State;
    virtual bool    open()=0;
    virtual void    close()=0;
    virtual void    main_loop()=0;
signals:
    void            finished();
    void            opened();
    void            closed();
    void            fireSimVariableEvent(CAircraftState);
    void            fireSimEvent(SimEvent);
public slots:
};

#endif // SIMCONNECTOR_H
