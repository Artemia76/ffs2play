#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

#include "misc/clogger.h"

class UDPServer : public QObject
{
    Q_OBJECT
private:
    QUdpSocket* m_pListenner;

    CLogger* Logger;
public:
    explicit UDPServer(QObject *parent = 0);
    ~UDPServer();

signals:
    void fireIncomingData (QByteArray);
private slots:
    void incomingData () ;
};

#endif // UDPSERVER_H
