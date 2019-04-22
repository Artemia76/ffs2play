#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QNetworkDatagram>

#include "tools/clogger.h"

class TCPServer;

class TCPClient : public QThread
{
    friend class TCPServer;
    Q_OBJECT
private:
    CLogger*    m_log;
    bool        m_running;
    qintptr     m_socketDescriptor;
    QHostAddress m_PeerIP;
    quint16     m_Port;
    QByteArray  m_Buffer;

protected:
    QTcpSocket* m_Socket;
    void        run() override;
    void        handle(qintptr ID);

public:
    explicit    TCPClient(QObject *parent = nullptr);
                ~TCPClient ();
    void        connectToHost(QHostAddress pHost, quint16 pPort);
    void        close();
    void        send(const QNetworkDatagram& pData);
    QHostAddress getPeerAddress();
    quint16     getPeerPort();

signals:
    void        error(QTcpSocket::SocketError socketError);
    void        fireIncomingData (QNetworkDatagram);
    void        fireDisconnected (TCPClient*);

private slots:
    void        OnIncomingData () ;
    void        OnConnected();
    void        OnDisconnected();
};

#endif // TCPCLIENT_H
