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
** p2pmanager.h is part of FF2Play project
**
** This singleton provide peers manager from peer to peer network
****************************************************************************/

#ifndef P2PMANAGER_H
#define P2PMANAGER_H

#include <QObject>
#include <QSettings>
#include "tools/clogger.h"
#include "sim/analyzer.h"
#include "net/udpserver.h"
#include "net/tcpserver.h"
#include "net/upnpclient.h"
#include "ffs2play.qpb.h"
#include "net/peer.h"
#include "tools/datetime.h"

class CP2PManager : public QObject
{
    Q_OBJECT
private:
                        CP2PManager(QObject *parent = nullptr);
    static CP2PManager* createInstance();
    void                initUPNP(bool flag);
    void                deleteUser(const QString pCallsign);

    CLogger*            m_log;
    Analyzer*           m_analyser;
    UDPServer*          m_UDPServer;
    TCPServer*          m_TCPServer;
    QSettings           m_settings;
    quint16             m_port{};
    int                 m_AILimit{};
    int                 m_AIRadius{};
    int                 m_P2PRate{};
    bool                m_bUPNP;
    bool                m_EnforceTCP;
    bool                m_Shadow{};
    bool                m_DropRateDisplay{};
    bool                m_SendDropRate{};
    datetime            m_LastDropRate;
    QStringList         m_InternalIP;
    QHostAddress        m_ExternalIP;
    QString             m_CallSign;
    CUPnPClient*        m_upnp;
    QVector<CPeer*>     m_peers;
    QTimer              m_checkAITimer;
    QStringList         m_BlackList;
    QString             m_Team;
public:
    ~CP2PManager();
    static CP2PManager*	instance();
    void                init (bool pFlag);
    int                 getPort();
    QString             getLocalIPs();
    void                setCallsign(const QString& pCallSign);
    QString             getCallsign();
    void                whazzup_Update(QVector<PeerData>& pWhazzup);
    bool                isDisabled(const QString& pCallsign);
    void                setDisable(const QString& pCallsign, bool pFlag);
    void                clear();
    void                sendTChat(const QString& pMessage);
    int                 getTxRate();
    void                setTxRate(int pTxRate);
    int                 getAILimit();
    void                setAILimit(int pAILimit);
    int                 getAIRadius();
    void                setAIRadius(int pAIRadius);
    bool                getShadowMode();
    void                setShadowMode(bool pFlag);
    QString             getTitle(const QString& pPeerName);
    QString             getModel(const QString& pPeerName);
    QString             getType(const QString& pPeerName);
    void                ResetAI(const QString& pPeerName);
    bool                getBlockedData(const QString &pPeerName);
    void                setBlockedData(const QString &pPeerName, bool pFlag);
    bool                getDropRateDisplay();
    void                setDropRateDisplay(bool pFlag);
    bool                getSendDropRate();
    void                setSendDropRate(bool pFlag);
    bool                getMaskedIA(const QString &pPeerName);
    void                setMaskedIA(const QString &pPeerName, bool pFlag);
    CAircraftState      getPeerData (const QString &pPeerName);
    double              getPeerDistance (const QString &pPeerName);
    PeerConnexionState  getPeerConnexionState (const QString &pPeerName);

signals:
    void                firePeerAdd(QString);
    void                firePeerRemove(QString);
    void                firePeerChange(QString,int);
    void                fireSendTChat(QString);
    void                fireReceiveTChat(QString,QString);
private slots:
    void                onCheckAI();
    void                onReceiveTChat(QString,QString);
    void                onLanding(double pVSpeed);
    void                onUPnPFailed(CUPnPClient::Error pError);
    void                onUPnPExternalAddress(QHostAddress pAddress);
    void                onUPnPPortMapped(QString pProtocol, int pErrorCode);
    void                onUPnPPortUnmapped(QString pProtocol, int pErrorCode);
};

#endif // P2PMANAGER_H
