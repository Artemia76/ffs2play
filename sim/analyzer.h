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
** analyzer.h is part of FF2Play project
**
** This singleton provide analysis tools to detect aircraft situation and
** phases
****************************************************************************/

#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>
#include <QVector>
#include <QMetaEnum>

#include "tools/clogger.h"
#include "sim/simbasemanager.h"

namespace ffs2play { namespace sim {

struct AircraftSituation
{
    datetime TimeStamp;
    QString     Category;
    double      Altitude;
    double      GAltitude;
    double      VSpeed;
    double      GSpeed;
    bool        ParkingBrake;
    bool        Crash;
    bool        Pushback;
    bool        OnGround;
};

class Analyzer: public QObject
{
    Q_OBJECT
public:
    enum Phases
    {
        IDLE,
        Boarding,
        Pushback,
        Taxiway,
        Takeoff,
        Climb,
        Cruise,
        Mission,
        InFlight,
        StepLevel,
        Descent,
        Approach,
        Final,
        GoAround,
        Landing,
        JoinGate,
        Unboarding,
        Crash
    };
    Q_ENUM(Phases)
private:
    explicit            Analyzer(QObject *parent = nullptr);
    static  Analyzer*   createInstance();
    void                updateVar(CAircraftState pState, datetime pTime);
    void                analyze();
    void                testApproach();
    void                setBoarding();
    void                setPushBack();
    void                setOnTaxi();
    void                setTakeOff();
    void                setClimb();
    void                setStepLevel();
    void                setFlight();
    void                setMission();
    void                setCruise();
    void                setDescent();
    void                setApproach();
    void                setFinal();
    void                setGoAround();
    void                setLanding();
    void                setJoinGate();
    void                setUnboarding();
    void                setCrash();

    void                setPhase(Phases pPhase);

    AircraftSituation   m_lastState;
    AircraftSituation   m_currentState;
    Phases              m_phase;
    bool                m_bFlyInProgress;
    QMetaEnum           m_metaPhases;

    CLogger*            m_log;
    simbasemanager*     m_sim;

public:
                        ~Analyzer();
    static  Analyzer*   instance();
    QString             getPhaseName();
    AircraftSituation   getLastState();

private slots:
    void                onSimVariableEvent(CAircraftState pState);
    void                onSimEvent(SimEvent pEvent);
    void                onSimOpened();
    void                onSimClosed();

signals:
    void                firePhaseChange(Phases pPhase);
    void                fireTouchSpeed(double pToucheSpeed);
};

}} // namespace ffs2play::sim

#endif // ANALYZER_H
