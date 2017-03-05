#ifndef P2PMANAGER_H
#define P2PMANAGER_H

#include <QObject>
#include <QSettings>
#include "singleton/clogger.h"
#include "net/udpserver.h"
#include "miniupnpc.h"
#include "upnpcommands.h"

class CP2PManager : public QObject
{
    Q_OBJECT
private:
                        CP2PManager(QObject *parent = 0);
    static CP2PManager* createInstance();
    void                initUPNP(bool flag);
    CLogger*            m_logger;
    UDPServer*          m_server;
    QSettings           s;
    int                 m_port;
    bool                m_bUPNP;
    QString             m_InternalIP;
    QString             m_ExternalIP;
    UPNPUrls            urls;
    IGDdatas            data;
public:
    ~CP2PManager();
    static CP2PManager* instance();
    void                init (bool flag);
signals:

private slots:
    void OnReceiveDatagram(const QNetworkDatagram* data);
};

#endif // P2PMANAGER_H
