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
** webmanager.h is part of FF2Play project
**
** This singleton provide Web Manager for web server communication
****************************************************************************/

#ifndef WEBMANAGER_H
#define WEBMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDataStream>
#include <QSettings>
#include <QVector>
#include <QTimer>
#include <QDateTime>
#include <QMetaEnum>
#include <QMetaObject>
#include <QNetworkAccessManager>

#include "tools/clogger.h"
#include "sim/simbasemanager.h"
#include "net/p2pmanager.h"
#include "sim/analyzer.h"
#include "web/chttprequest.h"
#include "web/caddrrequest.h"
#include "tools/crypto.h"
#include "sim/aircraftstate.h"

class DialogProfils;

struct SUser
{
    QString Profil;
    QString Login;
    QString Password;
    QString URL;
};

QDataStream& operator<<(QDataStream& out, const QVector<SUser>& v);
QDataStream& operator>>(QDataStream& in, QVector<SUser>& v);

class CWebManager : public QObject
{
    Q_OBJECT
    friend class        DialogProfils;
public:
    enum class XmlCode
    {
        hello,
        verify,
        liveupdate,
        atc,
        logout,
        addr
    };
    Q_ENUM(XmlCode)

private:
                        CWebManager(QObject *parent = nullptr);
    static CWebManager* createInstance();
    QSettings           m_settings;
    QTimer              m_liveUpdateTimer;
    QTimer              m_ATCTimer;
    simbasemanager&     m_simManager = simbasemanager::instance();
    CP2PManager&        m_P2P = CP2PManager::instance();
    CLogger&            m_log = CLogger::instance();
    Analyzer&           m_Analyzer = Analyzer::instance();
    QMetaEnum           m_metaXmlCode;
    XmlCode             m_code;

    // Etat de la connexion
    bool                m_bConnected;
    bool                m_SyncAIDone;
    bool                m_AIManagement;
    QString             m_sURL;
    QString             m_sCrypted_AESKey;
    QDateTime           m_lastGoodUpdate;
    int                 m_currentUser;
    QString             m_key;
    QUrl                m_portalURL;
    QHostAddress        m_publicIPv4;
    QHostAddress        m_publicIPv6;
    CCrypto             m_crypto;
    void                XmlBuild(QDomDocument& pDoc, XmlCode pCode);
    bool                CheckError(QDomDocument& pDoc);
    void                CheckWhazzup (QDomDocument& pDoc);
    void                UpdatePublicAddress (QHostAddress pAddress);

protected:
    QVector<SUser>      m_users;
    void                SaveConfig();
    void                SetCurrentUser(int pNum);

public:
                        ~CWebManager();
    static CWebManager&	instance();
    void                Connect();
    void                Disconnect();
    bool                isConnected();
    void                CheckNewVersion();
    void                CheckXPPluginVersion();

signals:
    void                FireHelloCallback();
    void                FireConnected();
    void                FireDisconnected();
    void                FireExit();
public slots:
    void                connect_disconnect();
private slots:
    void                HelloCallback (CHttpRequest* pRequest);
    void                ConnectCallback (CHttpRequest* pRequest);
    void                LiveUpdateCallback (CHttpRequest* pRequest);
    void                ATCCallback (CHttpRequest* pRequest);
    void                LogoutCallback (CHttpRequest* pRequest);
    void                CheckNewVersionCallback (CHttpRequest* pRequest);
    void                CheckXPPluginVersionCallback (CHttpRequest* pRequest);
    void                SendVerify();
    void                SendLiveUpdate();
    void                SendATCUpdate();
    void                SendAddr();
    void                AddrCallback (CAddrRequest* pRequest);
    void                OnATCUpdate();
    void                OnLiveUpdate();
};

Q_DECLARE_METATYPE(QVector<SUser>)

#endif // WEBMANAGER_H
