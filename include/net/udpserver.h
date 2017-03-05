#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QSettings>

#include "singleton/clogger.h"

class UDPServer : public QObject
{
    Q_OBJECT
private:
    QUdpSocket* m_pListenner;
    CLogger*    Logger;
    bool        m_running;
    int         m_port;
    QSettings    s;
public:
                UDPServer(QObject *parent = 0);
                ~UDPServer();
    void        start();
    void        stop();
    void        setPort(int pPort);
signals:
    void fireIncomingData (const QNetworkDatagram*);
private slots:
    void incomingData () ;
};

#endif // UDPSERVER_H
