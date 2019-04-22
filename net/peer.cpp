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
** Peer.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "net/peer.h"

///
/// \brief CPeer::CPeer
/// \param pServer
/// \param pExternalIP
/// \param pPort
/// \param pCallSign
/// \param pDisabled
/// \param pLocal
/// \param pInternalIP
/// \param pParent
/// \param pP2PRate
/// \param pP2PRadius
///
CPeer::CPeer(UDPServer* pUDPServer,
        TCPServer* pTCPServer,
        QHostAddress pExternalIP,
        quint16 pPort,
        QString pCallSign,
        bool pDisabled,
        bool pLocal,
        const QVector<QHostAddress>* pInternalIP,
        QObject *pParent,
        int pP2PRate,
        int pP2PRadius,
        QString pTeam) : QObject(pParent),
            CallSign(m_callSign),
            Disabled(m_disabled)
{
    m_log = CLogger::instance();
    m_AIMapping = AIMapping::instance();
    m_simManager = simbasemanager::instance();
    m_callSign = pCallSign;
    m_UDPServer = pUDPServer;
    m_TCPServer = pTCPServer;
    m_externalIP = pExternalIP;
    if (pInternalIP != nullptr)
    {
        m_internalIP = *pInternalIP;
    }
    m_port = pPort;
    m_local = pLocal;
    if (!m_local) m_EP = m_externalIP;
    else m_EP.clear();
    m_disabled = true;
    m_sel_iplocal = 0;
    m_trySimpleAI = false;
    Disable(pDisabled);
    m_visible = false;
    m_spawnable = false;
    m_blockData = false;
    m_team = pTeam;
    P2PRadius = pP2PRadius;
    P2PRate = pP2PRate;
    m_PeerMode = UDP;
    m_TCPClient=nullptr;
    if (m_simManager->getType()!=SIM_TYPE::FSX) m_disableCalculation = true;
    else m_disableCalculation = false;
    connect(&m_heartBeat,SIGNAL(timeout()),this,SLOT(OnHeartBeat()));
    connect(&m_timerCreateAI,SIGNAL(timeout()),this,SLOT(OnTimerCreateAI()));
    connect(m_TCPServer, SIGNAL(fireNewConnexion(TCPClient*)),this,SLOT(OnNewTCPConnection(TCPClient*)));
}

///
/// \brief CPeer::~CPeer
///
CPeer::~CPeer()
{
    disconnect(&m_heartBeat,SIGNAL(timeout()),this,SLOT(OnHeartBeat()));
    disconnect(&m_timerCreateAI,SIGNAL(timeout()),this,SLOT(OnTimerCreateAI()));
    Stop();
}
void CPeer::Disable(bool pFlag)
{
    if (m_disabled == pFlag) return;
    else
    {
        if (pFlag) Stop();
        else Start();
    }
}

///
/// \brief CPeer::Start
///
void CPeer::Start()
{
    if (!m_disabled) return;
    connect(m_UDPServer, SIGNAL(fireIncomingData(QNetworkDatagram)), this, SLOT(OnReceiveMessage(QNetworkDatagram)));
    connect(m_simManager, SIGNAL(fireSimEvent(SimEvent)), this, SLOT(OnSimEvent(SimEvent)));
    connect(m_simManager, SIGNAL(fireSimVariableEvent(CAircraftState)), this, SLOT(OnSimStateChange(CAircraftState)));
    connect(m_simManager, SIGNAL(fireSimEventFrame(float,float)), this, SLOT(OnSimEventFrame(float,float)));
    connect(m_simManager, SIGNAL(fireSimAICreated(QString,uint)),this, SLOT(OnSimAICreated(QString,uint)));
    connect(m_simManager, SIGNAL(fireSimAIUpdated(CAircraftState,uint)),this,SLOT(OnSimAIUpdated(CAircraftState,uint)));
    connect(m_simManager, SIGNAL(fireSimAIRemoved(uint)),this,SLOT(OnSimAIRemoved(uint)));
    connect(m_simManager, SIGNAL(closed()),this,SLOT(OnSimClosed()));
    m_heartBeat.start(5000);
    m_lastData = Now();
    m_lastStateEvent = m_lastData;
    m_counter = 0;
    m_counter_In = 0;
    m_counter_Out = 0;
    m_distance = -1;
    m_onLine = false;
    m_sendInterval = milliseconds(5000);
    m_ecart = 0;
    m_objectID = 0;
    m_version = 0;
    m_spawned = 0;
    //m_Refresh = false;
    m_miniPing = milliseconds(1000);
    //m_PredictiveTime = 1000;
    m_settings.beginGroup("P2P");
    if (m_settings.value("P2PInfoEnable", true).toBool())
    {
        if (m_simManager->getVersion()!=SimConnector::SIM_VERSION::XPLANE)
            m_simManager->sendScrollingText(CallSign + tr(" is connected on P2P Network"));
    }
    m_settings.endGroup();
    m_disabled = false;
    m_old_fps = std::numeric_limits<double>::quiet_NaN();
}

///
/// \brief CPeer::Stop
///
void CPeer::Stop()
{
    if (m_disabled) return;
    spawnAI(false);
    m_heartBeat.stop();
    disconnect(m_UDPServer, SIGNAL(fireIncomingData(QNetworkDatagram)), this,SLOT(OnReceiveMessage(QNetworkDatagram)));
    disconnect(m_simManager, SIGNAL(fireSimEvent(SimEvent)), this, SLOT(OnSimEvent(SimEvent)));
    disconnect(m_simManager, SIGNAL(fireSimVariableEvent(CAircraftState)),this, SLOT(OnSimStateChange(CAircraftState)));
    disconnect(m_simManager, SIGNAL(fireSimEventFrame(float,float)), this, SLOT(OnSimEventFrame(float,float)));
    disconnect(m_simManager, SIGNAL(fireSimAICreated(QString,uint)),this, SLOT(OnSimAICreated(QString,uint)));
    disconnect(m_simManager, SIGNAL(fireSimAIUpdated(CAircraftState,uint)),this,SLOT(OnSimAIUpdated(CAircraftState,uint)));
    disconnect(m_simManager, SIGNAL(fireSimAIRemoved(uint)),this,SLOT(OnSimAIRemoved(uint)));
    disconnect(m_simManager, SIGNAL(closed()),this,SLOT(OnSimClosed()));
    m_timerCreateAI.stop();

    m_settings.beginGroup("P2P");
    if (m_settings.value("P2PInfoEnable", true).toBool())
    {
        if (m_simManager->getVersion()!=SimConnector::SIM_VERSION::XPLANE)
            m_simManager->sendScrollingText(CallSign + tr(" leave from P2P Network"));
    }
    m_settings.endGroup();
    m_disabled = true;
    m_spawned = 0;
    m_PeerMode = UDP;
}

///
/// \brief CPeer::spawnAI
/// \param pFlag
///
void CPeer::spawnAI(bool pFlag)
{
    if (pFlag && (m_objectID == 0) && (m_spawned == 0))
    {
#ifdef FFS_DEBUG
        m_log->log("Peer [" + m_callSign + "] " +
                tr(" Local request for Title solve = ") + m_data.Title +
                tr (" , Model = ") + m_data.Model ,
                Qt::darkBlue,LEVEL_NORMAL);
#endif
        m_AIResolution = m_AIMapping->SolveTitle(m_data.Title,m_data.Model, m_data.Type);
        m_spawned = 2;
        createAI();
    }
    if ((!pFlag) && (m_spawned > 0))
    {
        m_spawned = 0;
        m_timerCreateAI.stop();
        if (m_objectID != 0)
        {
            if (m_simManager->removeAI(m_objectID))
            {
#ifdef FFS_DEBUG
                m_log->log("Peer [" + m_callSign + "] " + tr(" Remove aircraft"), Qt::darkBlue,LEVEL_NORMAL);
#endif
            }
            else m_log->log("Peer [" + m_callSign + "] " + tr(" Error during removing of the aircraft"), Qt::darkMagenta);
        }
    }
}

///
/// \brief CPeer::createAI
///
void CPeer::createAI()
{
    if (m_spawned == 2)
    {
        if (m_objectID != 0)
        {
            m_simManager->removeAI(m_objectID);
        }
        if (m_callSign.length()>8) m_tail = m_callSign.left(8);
        else m_tail = m_callSign;
        if (m_data.Category == "Helicopter") m_trySimpleAI = true;
        m_timerCreateAI.start(5000);
#ifdef FFS_DEBUG
        m_log->log("Peer [" + m_callSign + "] " + tr(" Requesting creation of aircraft with Title = ") + QString::fromStdString(m_AIResolution.title())
            + tr(" Model = ") + QString::fromStdString(m_AIResolution.model())
            , Qt::darkBlue,LEVEL_NORMAL);
#endif
        m_simManager->createAI(m_AIResolution, m_tail,m_data, m_trySimpleAI);
    }
}

///
/// \brief CPeer::refreshData
///
void CPeer::refreshData()
{
    if (m_disableCalculation)
    {
        m_AIData = m_data;
        if (m_spawned >= 5)
        {
            m_spawned = 7;
            m_simManager->updateAI(m_objectID, m_AIData);
        }
        return;
    }
    if ((m_objectID == 0) || (m_version != PROTO_VERSION)) return;
    // if it is the first update, we initialize datas with raw position
    if (m_spawned == 5)
    {
        m_AIData = m_data;
        m_simManager->freezeAI(m_objectID, m_data.OnGround);
        m_lastRender = Now();
        if (m_spawned==5) m_spawned=6;
    }
    // We have fresh data and are ready to compute extrapolation
    if (m_spawned >=6)
    {
        m_predictiveTime = m_remoteRefreshRate*3;
        // Extrapolation compute
        timespan Retard = m_lastData - m_data.TimeStamp; //Calcul du retard absolu de la donnée
        if (Retard.count() < 0) Retard = milliseconds(0);
        //double Periode = m_RemoteRefreshRate.TotalMilliseconds; //Calcul de la période entre deux données
        if (m_remoteRefreshRate.count() > 0)
        {
            // On mémorise la position actuelle de l'extrapolation
            //m_actualPos = m_AIData;
            m_actualPos.TimeStamp = m_lastRender;
            m_actualPos.Altitude = m_AIData.Altitude;
            m_actualPos.Latitude = m_AIData.Latitude;
            m_actualPos.Longitude = m_AIData.Longitude;
            m_actualPos.Heading = m_AIData.Heading;
            m_actualPos.Bank = m_AIData.Bank;
            m_actualPos.Pitch = m_AIData.Pitch;

            m_futurData.TimeStamp = m_lastRender + m_predictiveTime;
            double CoefHoriz = static_cast<double>((m_futurData.TimeStamp - m_data.TimeStamp).count()) / static_cast<double>(m_remoteRefreshRate.count());
            m_futurData.Altitude = m_data.Altitude + ((m_data.Altitude - m_oldData.Altitude) * CoefHoriz);
            m_futurData.Longitude = m_data.Longitude + ((m_data.Longitude - m_oldData.Longitude) * CoefHoriz);
            m_futurData.Latitude = m_data.Latitude + ((m_data.Latitude - m_oldData.Latitude) * CoefHoriz);
            double Gam_Heading = m_data.Heading - m_oldData.Heading;
            if (Gam_Heading < -180) Gam_Heading += 360;
            if (Gam_Heading > 180) Gam_Heading -= 360;
            m_futurData.Heading = m_data.Heading + (Gam_Heading * CoefHoriz);
            if (m_futurData.Heading < 0) m_futurData.Heading = m_futurData.Heading + 360;
            if (m_futurData.Heading >= 360) m_futurData.Heading = m_futurData.Heading - 360;
            double Gam_Pitch = m_data.Pitch - m_oldData.Pitch;
            if (Gam_Pitch < -180) Gam_Pitch += 360;
            if (Gam_Pitch > 180) Gam_Pitch -= 360;
            m_futurData.Pitch = m_data.Pitch + (Gam_Pitch * CoefHoriz);
            if (m_futurData.Pitch < -180) m_futurData.Pitch = m_futurData.Pitch + 360;
            if (m_futurData.Pitch >= 180) m_futurData.Pitch = m_futurData.Pitch - 360;
            double Gam_Bank = m_data.Bank - m_oldData.Bank;
            if (Gam_Bank < -180) Gam_Bank += 360;
            if (Gam_Bank > 180) Gam_Bank -= 360;
            m_futurData.Bank = m_data.Bank + (Gam_Bank * CoefHoriz);
            if (m_futurData.Bank < -180) m_futurData.Bank = m_futurData.Bank + 360;
            if (m_futurData.Bank >= 180) m_futurData.Bank = m_futurData.Bank - 360;
            //Calcul des deltas de correction
            m_deltaAltitude = m_futurData.Altitude - m_actualPos.Altitude;
            m_deltaLatitude = m_futurData.Latitude - m_actualPos.Latitude;
            m_deltaLongitude = m_futurData.Longitude - m_actualPos.Longitude;
            m_deltaHeading = m_futurData.Heading - m_actualPos.Heading;
            if (m_deltaHeading < -180) m_deltaHeading += 360;
            if (m_deltaHeading >= 180) m_deltaHeading -= 360;
            m_deltaPitch = m_futurData.Pitch - m_actualPos.Pitch;
            if (m_deltaPitch < -180) m_deltaPitch += 360;
            if (m_deltaPitch >= 180) m_deltaPitch -= 360;
            m_deltaBank = m_futurData.Bank - m_actualPos.Bank;
            if (m_deltaBank < -180) m_deltaBank += 360;
            if (m_deltaBank >= 180) m_deltaBank -= 360;
            /*m_log->log(tr("Horizon = ") + QString::number(CoefHoriz) +
                       " Retard = " + QString::number(Retard.count()) +
                       " RemoteRefreshyRate =" + QString::number(m_remoteRefreshRate.count()) +
                       " m_predictiveTime =" + QString::number(m_predictiveTime.count())
                       ,Qt::darkBlue,LEVEL_NORMAL);*/
        }
        m_spawned = 7;
        m_actualPos.Pitch = m_AIData.Pitch;
        if (m_data.OnGround != m_oldData.OnGround) m_simManager->freezeAI(m_objectID,m_data.OnGround);
    }
}

///
/// \brief CPeer::updateAI
/// \param pTime
/// \param pFPS
///
void CPeer::updateAI(datetime pTime, float pFPS)
{
    if (m_disableCalculation)
    {
        return;
    }
    m_mutex.lock();
    if ((m_objectID == 0) || (m_version != PROTO_VERSION))
    {
        m_lastRender = pTime;
    }
    else
    {
        if (m_spawned >=5)
        {
            m_AIData.AileronPos = m_data.AileronPos;
            m_AIData.ElevatorPos = m_data.ElevatorPos;
            m_AIData.RudderPos = m_data.RudderPos;
            m_AIData.SpoilerPos = m_data.SpoilerPos;
            m_AIData.ParkingBrakePos = m_data.ParkingBrakePos;
            m_AIData.Door1Pos = m_data.Door1Pos;
            m_AIData.Door2Pos = m_data.Door2Pos;
            m_AIData.Door3Pos = m_data.Door3Pos;
            m_AIData.Door4Pos = m_data.Door4Pos;
            m_AIData.StateEng1 = m_data.StateEng1;
            m_AIData.StateEng2 = m_data.StateEng2;
            m_AIData.StateEng3 = m_data.StateEng3;
            m_AIData.StateEng4 = m_data.StateEng4;
            m_AIData.ThrottleEng1 = m_data.ThrottleEng1;
            m_AIData.ThrottleEng2 = m_data.ThrottleEng2;
            m_AIData.ThrottleEng3 = m_data.ThrottleEng3;
            m_AIData.ThrottleEng4 = m_data.ThrottleEng4;
            m_AIData.Squawk = m_data.Squawk;
            m_AIData.GearPos =m_data.GearPos;
            m_AIData.FlapsIndex = m_data.FlapsIndex;
            m_AIData.LandingLight = m_data.LandingLight;
            m_AIData.StrobeLight =m_data.StrobeLight;
            m_AIData.BeaconLight = m_data.BeaconLight;
            m_AIData.NavLight = m_data.NavLight;
            m_AIData.RecoLight = m_data.RecoLight;
            m_AIData.Smoke = m_data.Smoke;
            // If we have enougth data (new + old) we start run interpolation
            if (m_spawned >=7)
            {
                double FPSAvg = static_cast<double>(pFPS);
                if (!std::isnan(m_old_fps))
                {
                    FPSAvg = (FPSAvg + m_old_fps)/2.0;
                }
                m_old_fps = static_cast<double>(pFPS);
                timespan Temps = duration_cast<milliseconds>(milliseconds(1000) / FPSAvg);
                double CoefInterpol = static_cast<double>(Temps.count()) / static_cast<double>(duration_cast<milliseconds>(m_futurData.TimeStamp - m_actualPos.TimeStamp).count());
                m_lastRender = pTime;
                m_AIData.Altitude += m_deltaAltitude * CoefInterpol;
                m_AIData.Latitude += m_deltaLatitude * CoefInterpol;
                m_AIData.Longitude += m_deltaLongitude * CoefInterpol;
                m_AIData.Heading += m_deltaHeading * CoefInterpol;
                if (m_AIData.Heading < 0) m_AIData.Heading += 360;
                if (m_AIData.Heading >= 360) m_AIData.Heading -= 360;
                m_AIData.Pitch += m_deltaPitch * CoefInterpol;
                if (m_AIData.Pitch < -180) m_AIData.Pitch += 360;
                if (m_AIData.Pitch >= 180) m_AIData.Pitch -= 360;
                m_AIData.Bank += m_deltaBank * CoefInterpol;
                if (m_AIData.Bank < -180) m_AIData.Bank += 360;
                if (m_AIData.Bank >= 180) m_AIData.Bank -= 360;
            }
            if (m_data.OnGround && (!m_simManager->getOption(DisableAltCorr)))
            {
                // Mise à jour de l'altitude sol de référence
                m_old_GND_AI = m_AISimData.GroundAltitude;
                m_AIData.Altitude = m_AISimData.GroundAltitude + m_AIResolution.cgheight();
            }
            if (m_spawned >= 7)
            {
                m_simManager->updateAI(m_objectID, m_AIData);
            }
        }
    }
    m_mutex.unlock();
}

///
/// \brief CPeer::resetAI
///
void CPeer::resetAI()
{
    if (m_spawned >=3)
    {
        spawnAI(false);
    }
}

///
/// \brief CPeer::GetSignal
/// \return
///
int CPeer::GetSignal()
{
    if (m_disabled) return 4;
    else if (m_onLine)
    {
        int latence = static_cast<int>(m_latence.count());
        if (latence < 200) return 3;
        else if ((latence >= 200) && (latence < 500)) return 2;
        else return 1;
    }
    else return 0;
}

void CPeer::SetTxRate(int pTxRate)
{
    P2PRate = pTxRate;
}

void CPeer::SetAIRadius(int pAIRadius)
{
    P2PRadius = pAIRadius;
}

PeerConnexionState CPeer::GetConnexionState ()
{
    PeerConnexionState State;
    State.IP = m_EP;
    State.Port = m_port;
    State.Ping = m_latence.count();
    State.RcvCnt = m_counter_In;
    State.SndCnt = m_counter_Out;
    State.Mode = m_PeerMode;
    return State;
}
