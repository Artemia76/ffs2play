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
** simbasemanager.h is part of FF2Play project
**
** This singleton provide simulator manager interface
** It working friendly
****************************************************************************/

#ifndef SIMBASEMANAGER_H
#define SIMBASEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QSettings>
#include <memory>
#include "tools/clogger.h"

#ifdef SIMCONNECT_BUILD
    #include "sim/scmanager.h"
#endif

#include "sim/xpmanager.h"
#include "sim/fgmanager.h"
#include "ffs2play.qpb.h"

enum class SIM_TYPE
{
    FSX,
    XPLANE,
#ifdef FFS_DEBUG
    FLIGHTGEAR
#endif
};

class simbasemanager : public QObject
{
    Q_OBJECT
private:
static	simbasemanager*         createInstance();
    explicit                    simbasemanager(QObject *parent = nullptr);
    CLogger*                    m_log;
    QSettings                   m_settings;
    bool                        m_initialized;
    std::unique_ptr<SimConnector> m_connector;
    bool                        m_bEnaDispCallSign;
    SIM_TYPE                    m_simType;
    QList<SIM_TYPE>             m_simAvailable;
    QTimer                      m_RetryTimer;
    int                         m_RetryCounter;

public:
static simbasemanager*          instance();
                                ~simbasemanager();
    void                        load(SIM_TYPE pSimType);
    void                        unload();
    void                        open();
    void                        close();
    bool                        isConnected();
    bool                        isInitialized();
    const CAircraftState        getAircraftState();
    void                        sendScrollingText(const QString& pMessage);
    SimConnector::SIM_VERSION   getVersion();
    sVersion                    getPlugVersion();
    SIM_TYPE                    getType();
    const QList<SIM_TYPE>&      getSimList();
    void                        setSimType(SIM_TYPE pSimType);
    bool                        createAI(AIResol& pAircraft, const QString& pTail, const CAircraftState& pData, bool pNonAIObject);
    bool                        removeAI(uint& pObjectID);
    bool                        updateAI(uint pObjectID, const CAircraftState& pData);
    void                        freezeAI(uint pObjectID, bool pState);
    void                        enableCallSign(bool pState);
    void                        setOptionChange(SimOption pOption,bool pFlag);
    bool                        getOption(SimOption pOption);
    void                        setCallSignColor(const QColor& pColor);
    QColor                      getCallSignColor();
    void                        setPathList(const QStringList& pPaths);

private slots:
    void                        OnSimVariableEvent(CAircraftState);
    void                        OnSimEvent(SimEvent);
    void                        OnSimAICreated(QString,uint);
    void                        OnSimAIUpdated(CAircraftState, uint);
    void                        OnSimAIRemoved(uint);
    void                        OnSimEventFrame(float,float);
    void                        OnSimConnected();
    void                        OnSimDisconnected();
    void                        OnSimTimedOut();
    void                        OnRetryTimer();

public slots:
    void                        open_close();

signals:
    void                        loaded(SIM_TYPE);
    void                        opened();
    void                        closed();
    void                        fireSimVariableEvent(CAircraftState);
    void                        fireSimEvent(SimEvent);
    void                        fireSimAICreated(QString,uint);
    void                        fireSimAIUpdated(CAircraftState, uint);
    void                        fireSimAIRemoved(uint);
    void                        fireSimEventFrame(float,float);
};

#endif // SIMBASEMANAGER_H
