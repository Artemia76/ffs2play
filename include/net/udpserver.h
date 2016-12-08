#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>

class UDPServer : public QObject
{
    Q_OBJECT
    QUdpSocket m_Listenner;
public:
    explicit UDPServer(QObject *parent = 0);
    ~UDPServer();

signals:

public slots:
};

#endif // UDPSERVER_H
