#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QNetworkDatagram>

#include "tools/clogger.h"

class TCPServer;

class TCPClient : public QObject
{
    friend class TCPServer;
    Q_OBJECT
private:
    CLogger&    m_log = CLogger::instance();
    bool        m_running;
    qintptr     m_socketDescriptor;
    QHostAddress m_PeerIP;
    quint16     m_Port;
    QByteArray  m_Buffer;

protected:
    QTcpSocket* m_Socket;
    void        handle(QTcpSocket* pSocket);

public:
    explicit    TCPClient(QObject *parent = nullptr);
                ~TCPClient ();
    void        connectToHost(QHostAddress pHost, quint16 pPort);
    void        close();
    void        send(const QNetworkDatagram& pData);
    QHostAddress getPeerAddress();
    quint16     getPeerPort();

signals:
    void        fireIncomingData (QNetworkDatagram);
    void        fireDisconnected (TCPClient*);

private slots:
    void        OnIncomingData () ;
    void        OnConnected();
    void        OnDisconnected();
    void        OnError(QAbstractSocket::SocketError pError);
};

#endif // TCPCLIENT_H
