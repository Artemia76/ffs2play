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
** analyzer.cpp is part of FF2Play project
**
** This singleton provide analysis tools to detect aircraft situation and
** phases
****************************************************************************/

#include "analyzer.h"
#include "tools/singleton.h"

///
/// \brief Analyzer::Analyzer
/// \param parent
///
Analyzer::Analyzer(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Phases>("Phases");
    m_metaPhases = QMetaEnum::fromType<Analyzer::Phases>();
    m_log = CLogger::instance();
    m_sim = simbasemanager::instance();
    m_bFlyInProgress = false;
    m_phase = Phases::IDLE;
    m_currentState.OnGround=true;

    connect(m_sim,SIGNAL(fireSimVariableEvent(CAircraftState)),this,SLOT(onSimVariableEvent(CAircraftState)));
    connect(m_sim, SIGNAL(fireSimEvent(SimEvent)), this, SLOT(onSimEvent(SimEvent)));
    connect(m_sim,SIGNAL(opened()),this,SLOT(onSimOpened()));
    connect(m_sim, SIGNAL(closed()), this, SLOT(onSimClosed()));
}

///
/// \brief Analyzer::~Analyzer
///
Analyzer::~Analyzer()
{
    disconnect(m_sim,SIGNAL(fireSimVariableEvent(CAircraftState)),this,SLOT(onSimVariableEvent(CAircraftState)));
    disconnect(m_sim, SIGNAL(fireSimEvent(SimEvent)), this, SLOT(onSimEvent(SimEvent)));
    disconnect(m_sim,SIGNAL(opened()),this,SLOT(onSimOpened()));
    disconnect(m_sim, SIGNAL(closed()), this, SLOT(onSimClosed()));
}

///
/// \brief Analyzer::createInstance
/// \return
///
Analyzer* Analyzer::createInstance()
{
    return new Analyzer();
}

///
/// \brief Analyzer::instance
/// \return
///
Analyzer* Analyzer::instance()
{
    return Singleton<Analyzer>::instance(Analyzer::createInstance);
}

///
/// \brief Analyzer::onSimVariableEvent
/// \param pState
///
void Analyzer::onSimVariableEvent(CAircraftState pState)
{
    m_currentState.TimeStamp = pState.TimeStamp;
    m_currentState.Category = pState.Category;
    m_currentState.Altitude = pState.Altitude;
    m_currentState.GAltitude = pState.GroundAltitude;
    m_currentState.GSpeed = pState.GroundSpeed;
    m_currentState.VSpeed = pState.Vario;
    m_currentState.ParkingBrake = pState.ParkingBrake;
    m_currentState.OnGround = pState.OnGround;
    if (m_currentState.OnGround && (!m_lastState.OnGround))
    {
        double TouchSpeed = (m_lastState.VSpeed + m_currentState.VSpeed) /2;
        emit(fireTouchSpeed(TouchSpeed));
    }
    if (m_sim->isConnected()) analyze();
    m_lastState = m_currentState;
}

///
/// \brief Analyzer::onSimEvent
/// \param pEvent
///
void Analyzer::onSimEvent(SimEvent pEvent)
{
    switch (pEvent.Evt_Id)
    {
    case SIM_EVENT_ID::PARKING_BRAKE:
        m_currentState.ParkingBrake = static_cast<bool>(pEvent.Data);
        break;
    case SIM_EVENT_ID::CRASH:
        if (pEvent.Data>0)
        {
            m_currentState.Crash = true;
        }
        else
        {
            m_currentState.Crash = false;
        }
        break;
    case SIM_EVENT_ID::PUSHBACK:
        m_currentState.Pushback = pEvent.Data<3;
        break;
    case SIM_EVENT_ID::ON_GROUND:
        m_currentState.OnGround = static_cast<bool>(pEvent.Data);
        if (m_currentState.OnGround)
        {
            if (m_currentState.VSpeed < -800)
            {
                m_currentState.Crash=true;
            }
        }
        break;
    default:
        break;
    }
}

///
/// \brief Analyzer::onSimOpened
///
void Analyzer::onSimOpened()
{
    m_bFlyInProgress = true;
}

///
/// \brief Analyzer::onSimClosed
///
void Analyzer::onSimClosed()
{
    m_bFlyInProgress = false;
}

///
/// \brief Analyzer::analyze
///
void Analyzer::analyze()
{
    if (!m_bFlyInProgress) return;
    switch (m_phase)
    {
        case Phases::IDLE:
        {
            if ((m_currentState.Category == "Airplane") && m_currentState.ParkingBrake)
            {
                setBoarding();
            }
            else setBoarding();
            break;
        }
        case Phases::Unboarding:
        case Phases::Boarding:
        {
            if (m_currentState.Category == "Airplane")
            {
                if (!m_currentState.ParkingBrake && m_currentState.Pushback)
                {
                    setPushBack();
                }
                if (m_currentState.GSpeed>5)
                {
                    setOnTaxi();
                }
            }
            else if (m_currentState.Category == "Helicopter")
            {
                if (m_currentState.OnGround)
                {
                    setTakeOff();
                }
            }
            else
            {
                if (m_currentState.GSpeed > 10)
                {
                    setMission();
                }
            }
            break;
        }
        case Phases::Pushback:
        {
            if (m_currentState.GSpeed > 5)
            {
                setOnTaxi();
            }
            break;
        }
        case Phases::Taxiway:
        {
            if (m_currentState.GSpeed > 40)
            {
                setTakeOff();
            }
            break;
        }
        case Phases::Takeoff:
        {
            if (m_currentState.Category=="Airplane")
            {
                if (!m_currentState.OnGround && ((m_currentState.Altitude - m_currentState.GAltitude) > 100))
                {
                    setClimb();
                }
            }
            else
            {
                if ((m_currentState.Altitude - m_currentState.GAltitude) > 100)
                {
                    setFlight();
                }
            }
            break;
        }
        case Phases::Climb:
        {
            if (m_currentState.OnGround)
            {
                setLanding();
            }
            else if (m_currentState.VSpeed < 300)
            {
                setCruise();
            }
            break;
        }
        case Phases::Cruise:
        {
            if (m_currentState.OnGround) setLanding();
            else if (m_currentState.VSpeed < -500) setDescent();
            else if (m_currentState.VSpeed > 500) setClimb();
            else testApproach();
            break;
        }
        case Phases::Mission:
        case Phases::InFlight:
        {
            if ((m_currentState.Category=="Airplane")||(m_currentState.Category=="Helicopter"))
            {
                if (m_lastState.OnGround)
                {
                    setLanding();
                }
                else testApproach();
            }
            else
            {
                if (m_currentState.ParkingBrake) setUnboarding();
            }
            break;
        }
        case Phases::Descent:
        {
            if (m_currentState.OnGround)
            {
                setLanding();
            }
            else if (m_currentState.VSpeed > -300)
            {
                setCruise();
            }
            else testApproach();
            break;
        }
        case Phases::Approach:
        {
            if (m_currentState.OnGround)
            {
                setLanding();
            }
            else if ((m_currentState.Altitude - m_currentState.GAltitude) < 500)
            {
                setFinal();
            }
            else if ((m_currentState.Altitude - m_currentState.GAltitude) > 1000)
            {
                setGoAround();
            }
            break;
        }
        case Phases::Final:
        {
            if (m_currentState.OnGround)
            {
                setLanding();
            }
            else if (((m_currentState.Altitude - m_currentState.GAltitude) > 600) && (m_currentState.VSpeed>0))
            {
                setGoAround();
            }
            break;
        }
        case Phases::GoAround:
        {
            if (m_currentState.OnGround)
            {
                setLanding();
            }
            else testApproach();
            break;
        }
        case Phases::Landing:
        {
            if (m_currentState.OnGround && (!m_lastState.OnGround))
            {
                emit(fireTouchSpeed((m_lastState.VSpeed + m_currentState.VSpeed)/2));
            }
            if (m_currentState.GAltitude > 100)
            {
                setGoAround();
            }
            if ((m_currentState.GSpeed < 40) && m_currentState.OnGround)
            {
                setJoinGate();
            }
            break;
        }
        case Phases::JoinGate:
        {
            if (((m_currentState.Category == "Airplane" && m_currentState.ParkingBrake) || (m_currentState.Category != "Airplane")) && (m_currentState.GSpeed < 5))
            {
                setUnboarding();
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

///
/// \brief Analyzer::testApproach
///
void Analyzer::testApproach()
{
    if (((m_currentState.Altitude - m_currentState.GAltitude) < 1000) && (m_currentState.VSpeed < -300))
    {
        setApproach();
    }
}

///
/// \brief Analyzer::setBoarding
///
void Analyzer::setBoarding()
{
    setPhase(Phases::Boarding);
}

///
/// \brief Analyzer::setPushBack
///
void Analyzer::setPushBack()
{
    setPhase(Phases::Pushback);
}

///
/// \brief Analyzer::setOnTaxi
///
void Analyzer::setOnTaxi()
{
    setPhase(Phases::Taxiway);
}

///
/// \brief Analyzer::setTakeOff
///
void Analyzer::setTakeOff()
{
    setPhase(Phases::Takeoff);
}

///
/// \brief Analyzer::setClimb
///
void Analyzer::setClimb()
{
    setPhase(Phases::Climb);
}

///
/// \brief Analyzer::setStepLevel
///
void Analyzer::setStepLevel()
{
    setPhase(Phases::StepLevel);
}

///
/// \brief Analyzer::setFlight
///
void Analyzer::setFlight()
{
    setPhase(Phases::InFlight);
}

///
/// \brief Analyzer::setMission
///
void Analyzer::setMission()
{
    setPhase(Phases::Mission);
}

///
/// \brief Analyzer::setCruise
///
void Analyzer::setCruise()
{
    setPhase(Phases::Cruise);
}

///
/// \brief Analyzer::setDescent
///
void Analyzer::setDescent()
{
    setPhase(Phases::Descent);
}

///
/// \brief Analyzer::setApproach
///
void Analyzer::setApproach()
{
    setPhase(Phases::Approach);
}

///
/// \brief Analyzer::setFinal
///
void Analyzer::setFinal()
{
    setPhase(Phases::Final);
}

///
/// \brief Analyzer::setGoAround
///
void Analyzer::setGoAround()
{
    setPhase(Phases::GoAround);
}

///
/// \brief Analyzer::setLanding
///
void Analyzer::setLanding()
{
    setPhase(Phases::Landing);
}

///
/// \brief Analyzer::setJoinGate
///
void Analyzer::setJoinGate()
{
    setPhase(Phases::JoinGate);
}

///
/// \brief Analyzer::setUnboarding
///
void Analyzer::setUnboarding()
{
    setPhase(Phases::Unboarding);
}

///
/// \brief Analyzer::setCrash
///
void Analyzer::setCrash()
{
    setPhase(Phases::Crash);
}

///
/// \brief Analyzer::setPhase
/// \param pPhase
///
void Analyzer::setPhase(Phases pPhase)
{
    m_phase = pPhase;

#ifdef FFS_DEBUG
    m_log->log("Analyzer : Change Phase = " +  getPhaseName(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
    firePhaseChange(pPhase);
}

///
/// \brief Analyzer::getPhaseName
/// \return
///
QString Analyzer::getPhaseName()
{
    return m_metaPhases.valueToKey(m_phase);
}
