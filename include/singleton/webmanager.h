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
