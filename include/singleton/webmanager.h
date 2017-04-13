/****************************************************************************
**
** Copyright (C) 2017 Gianni Peschiutta.
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

#ifndef WEBMANAGER_H
#define WEBMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDataStream>
#include <QSettings>
#include <QList>
#include <QTimer>
#include <QDateTime>
#include "singleton/clogger.h"
#include "singleton/scmanager.h"
#include "singleton/p2pmanager.h"

struct SUser
{
    QString Profil;
    QString Login;
    QString Password;
    QString URL;
};

Q_DECLARE_METATYPE(QList<SUser>)

enum XmlCode
{
    verify,
    //getflight,
    liveupdate,
    atc,
    //pirep,
    syncai,
    sendai,
    askai,
    logout
};

class CWebManager : public QObject
{
    Q_OBJECT
    //Q_ENUM(XmlCode)
    friend class DialogProfils;
private:
                        CWebManager(QObject *parent = 0);
    static CWebManager* createInstance();
    QSettings           s;
    QTimer              LiveUpdateTimer;
    QTimer              ATCTimer;
    CSCManager*         SCM;
    CP2PManager*        P2P;
    CLogger*            Log;

    // Etat de la connexion
    bool                m_bConnected;
    bool                m_SyncAIDone;
    QString             m_sURL;
    QDateTime           LastGoodUpdate;
    QList<SUser>        Users;
    SUser               CurentUser;
    QString             Key;
    QDomDocument        SendVerify;

    void                XmlBuild (QDomDocument& Doc, XmlCode Code);
protected:
    void                SaveConfig();

public:
                        ~CWebManager();
    static CWebManager* instance();
    bool                DelayedExit;
signals:

public slots:
};

#endif // WEBMANAGER_H
