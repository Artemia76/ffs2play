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

#ifndef PEER_H
#define PEER_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QDateTime>
#include <sstream>
#include <cstddef>

#include "tools/ffs2core_global.h"
#include "singleton/simbasemanager.h"
#include "singleton/aimapping.h"
#include "protobuf/ffs2play.pb.h"
#include "singleton/clogger.h"
#include "net/udpserver.h"
#include "sim/aircraftstate.h"


using namespace ffs2play;
using namespace std;

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
    QList<QHostAddress> localIPs;
    quint16 Port;
};

typedef unsigned char byte;

const byte PROTO_VERSION = 14;

// Déclaration of Singleton manager to have recursion access and friendly protection


class CP2PManager;

class FFS2CORE_EXPORT CPeer : public QObject
{
    Q_OBJECT
    friend class CP2PManager;
protected:
    CPeer(
        UDPServer* pServer,
        QHostAddress pExternalIP,
        int pPort,
        QString CallSign,
        bool pDisabled=false,
        bool pLocal = false,
        const QList<QHostAddress>* pInternalIP=NULL,
        QObject *parent = nullptr
        );
public:
    ~CPeer ();
private:
    QTimer                  HeartBeat;
    int                     m_Counter;
    unsigned char           m_Counter_In;
    bool                    m_OnLine;
    QString                 m_CallSign;
    QString                 m_Tail;
    simbasemanager*         SIM;
    AIMapping*              m_AIMapping;
    QHostAddress            m_EP;
    QList<QHostAddress>     m_InternalIP;
    QHostAddress            m_ExternalIP;
    int                     m_Port;
    AirData                 m_Data;
    AirData                 m_SendData;
    AirData                 m_FuturData;
    AirData                 m_ActualPos;
    CAIBase                 m_AIData;
    QDateTime               m_LastPing;
    QDateTime               m_LastData;
    quint32                 m_SendInterval;
    QDateTime               m_LastStateEvent;
    UDPServer*              Server;
    CLogger*                Log;
    int                     m_Latence;
    double                  m_Distance;
    int                     m_RefreshRate;
    int                     m_RemoteRefreshRate;
    int                     m_Decalage;
    double                  m_MiniPing;
    quint32                 m_ObjectID;
    byte                    m_Version;
    int                     m_Spawned;
    bool                    m_Refresh;
    double                  m_PredictiveTime;
    QList<double>           m_FrameRateArray;
    QTimer                  m_TimerCreateAI;
    AIResol                 m_AIResolution;
    int                     m_FrameCount;
    double                  m_Ecart;
    QDateTime               m_LastAIUpdate;
    bool                    m_bDisabled;
    //Mutex                   m_Mutex;
    bool                    m_bSelected;
    bool                    m_bLocal;
    int                     m_sel_iplocal;
    bool                    m_TrySimpleAI;
    QDateTime               m_LastRender;
    double                  m_DeltaAltitude;
    double                  m_DeltaLongitude;
    double                  m_DeltaLatitude;
    double                  m_DeltaHeading;
    double                  m_DeltaPitch;
    double                  m_DeltaBank;
    bool                    m_Visible;
    bool                    m_bSpawnable;
    double                  m_old_fps;
    
signals:

public slots:
};

#endif // PEER_H
