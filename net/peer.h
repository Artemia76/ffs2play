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
** Peer.h is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#ifndef PEER_H
#define PEER_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QMutex>
#include <QDataStream>
#include <QNetworkDatagram>
#include <cstddef>

#include "sim/simbasemanager.h"
#include "disk/aimapping.h"
#include "ffs2play.qpb.h"
#include "tools/clogger.h"
#include "net/udpserver.h"
#include "net/tcpserver.h"
#include "sim/aircraftstate.h"
#include "tools/datagram.h"

enum class Protocol
{
    PING,
    PONG,
    CHAT,
    DATA,
    VERSION,
    EVENT,
    REQ_VERSION
};

struct Deltas
{
    double Longitude;
    double Latitude;
    double Altitude;
    double Heading;
    double Pitch;
    double Bank;
};

struct PeerData
{
    QString CallSign;
    QHostAddress externalIP;
    QHostAddress externalIP6;
    QVector<QHostAddress> localIPs;
    quint16 Port;
    QString Team;
};

// This enum is used to find the best way to connect peer
enum PeerMode
{
    NONE,
    UDP,
    TCP_SERVER,
    TCP_CLIENT
};

class PeerConnexionState
{
public:
    PeerConnexionState()
    {
        Port=0;
        Ping=0;
        RcvCnt=0;
        SndCnt=0;
        Mode=NONE;
    }
    QHostAddress IP;
    quint32 Port;
    double  Ping;
    quint8  RcvCnt;
    quint8  SndCnt;
    PeerMode Mode;
};

typedef unsigned char byte;

const quint8 PROTO_VERSION = 14;

// Déclaration of Singleton manager to have recursion access and friendly protection


//class singleton::CP2PManager;

class CPeer : public QObject
{
    Q_OBJECT
    friend class CP2PManager;
public:
    const QString&          CallSign;
    const bool&             Disabled;
    int                     P2PRadius;
    int                     P2PRate;
protected:
                            CPeer(
                                UDPServer* pUDPServer,
                                TCPServer* pTCPServer,
                                QHostAddress pExternalIP,
                                QHostAddress pExternalIP6,
                                quint16 pPort,
                                QString pCallSign,
                                bool pDisabled=false,
                                bool pLocal = false,
                                const QVector<QHostAddress>* pInternalIP=NULL,
                                QObject *pParent = nullptr,
                                int pP2PRate = 200,
                                int pP2PRadius = 10,
                                QString pTeam="",
                                bool pEnforceTCP=false
                            );
                            ~CPeer ();
    void                    Start();
    void                    Stop();
    void                    Disable(bool pFlag);
    int                     GetSignal();
    void                    SetTxRate(int pTxRate);
    void                    SetAIRadius(int pAIRadius);
    PeerConnexionState      GetConnexionState ();

private:
    PeerMode                m_PeerMode;
    QSettings               m_settings;
    QTimer                  m_heartBeat;
    int                     m_counter;
    quint8                  m_counter_In;
    quint8                  m_counter_Out;
    bool                    m_onLine;
    QString                 m_callSign;
    QString                 m_tail;
    simbasemanager*         m_simManager;
    AIMapping*              m_AIMapping;
    QHostAddress            m_EP;
    QVector<QHostAddress>   m_internalIP;
    QHostAddress            m_externalIP;
    QHostAddress            m_externalIP6;
    QVector<QHostAddress>   m_candidates;
    bool                    isCandidate(const QHostAddress& pAddress) const;
    static QHostAddress     normalized(const QHostAddress& pAddress);
    quint16                 m_port;
    CAircraftState          m_data;
    CAircraftState          m_oldData;
    AirData                 m_sendData;
    CAircraftState          m_futurData;
    CAircraftState          m_actualPos;
    CAircraftState          m_AIData;
    CAircraftState          m_AISimData;
    datetime                m_lastPing;
    datetime                m_lastData;
    timespan                m_sendInterval;
    datetime                m_lastStateEvent;
    UDPServer*              m_UDPServer;
    TCPServer*              m_TCPServer;
    TCPClient*              m_TCPClient;
    CLogger*                m_log;
    timespan                m_latence;
    double                  m_distance;
    timespan                m_refreshRate;
    timespan                m_remoteRefreshRate;
    timespan                m_decalage;
    timespan                m_miniPing;
    quint32                 m_objectID;
    quint8                  m_version;
    int                     m_spawned;
    bool                    m_refresh;
    timespan                m_predictiveTime;
    QVector<double>         m_frameRateArray;
    QTimer                  m_timerCreateAI;
    AIResol                 m_AIResolution;
    int                     m_frameCount;
    double                  m_ecart;
    datetime                m_lastAIUpdate;
    bool                    m_disabled;
    bool                    m_disableCalculation;
    QMutex                  m_mutex;
    bool                    m_selected;
    bool                    m_local;
    int                     m_sel_iplocal;
    bool                    m_trySimpleAI;
    datetime                m_lastRender;
    double                  m_deltaAltitude;
    double                  m_deltaLongitude;
    double                  m_deltaLatitude;
    double                  m_deltaHeading;
    double                  m_deltaPitch;
    double                  m_deltaBank;
    bool                    m_visible;
    bool                    m_spawnable;
    bool                    m_blockData;
    double                  m_old_fps;
    double                  m_old_GND_AI;
    QString                 m_team;
    bool                    m_EnforceTCP;

    ///
    /// \brief send
    /// \param pOutput
    ///
    void                    send(const QByteArray& pOutput);

    ///
    /// \brief sendChat
    /// \param pMessage
    ///
    void                    sendChat(const QString& pMessage);

    ///
    /// \brief sendPing
    ///
    void                    sendPing();

    ///
    /// \brief sendPong
    /// \param pTime
    ///
    void                    sendPong(const datetime& pTime);

    ///
    /// \brief sendVersion
    ///
    void                    sendVersion();

    ///
    /// \brief requestVersion
    ///
    void                    requestVersion();

    ///
    /// \brief sendData
    ///
    void                    sendData();

    ///
    /// \brief spawnAI
    /// \param pFlag
    ///
    void                    spawnAI(bool pFlag);
    void                    refreshData();
    void                    updateAI(datetime pTime, float pFPS);
    void                    createAI();
    void                    resetAI();
signals:
    void                    fireReceiveTChat(QString,QString);
protected slots:
    void                    OnSendTChat(QString);
private slots:
    void                    OnReceiveMessage(QNetworkDatagram pDatagram);
    void                    OnNewTCPConnection(TCPClient* pTCPClient);
    void                    OnTCPClientDisconnected(TCPClient* pTCPClient);
    void                    OnSimStateChange(CAircraftState pState);
    void                    OnSimEvent(SimEvent);
    void                    OnSimEventFrame(float,float);
    void                    OnSimAICreated(QString,uint);
    void                    OnSimAIUpdated(CAircraftState,uint);
    void                    OnSimAIRemoved(uint);
    void                    OnHeartBeat();
    void                    OnTimerCreateAI();
    void                    OnSimClosed();
};

#endif // PEER_H
