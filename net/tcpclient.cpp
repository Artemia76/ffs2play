/****************************************************************************
**
** Copyright (C) 2019 FSFranceSimulateur team.
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
** tcpclient.cpp is part of FF2Play project
**
** This class provide an tcp socket to receive and send data over networks
****************************************************************************/

#include "net/tcpclient.h"

TCPClient::TCPClient(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");
    qRegisterMetaType<TCPClient*>("TCPClient*");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    m_Socket = nullptr;
    m_log = CLogger::instance();
}

TCPClient::~TCPClient()
{
    close();
#ifdef FFS_DEBUG
    m_log->log("TCPClient : " + tr("Destructor"), Qt::cyan,LEVEL_NORMAL);
#endif
}

///
/// \brief Handle existing socket from server side
/// \param ID
///
void TCPClient::handle(QTcpSocket* pSocket)
{
    if (pSocket==nullptr) return;
    if (m_Socket != nullptr)
    {
        close();
    }
    m_Socket=pSocket;
    m_Socket->setParent(this);
    m_PeerIP = m_Socket->peerAddress();
    m_Port = m_Socket->peerPort();
    connect(m_Socket, SIGNAL(readyRead()), this, SLOT(OnIncomingData()));
    connect(m_Socket, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));
    connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
}

void TCPClient::connectToHost(QHostAddress pHost, quint16 pPort)
{
   if (m_Socket!=nullptr)
   {
       close();
   }
   m_PeerIP = pHost;
   m_Port = pPort;
   m_Socket = new QTcpSocket(this);
   connect(m_Socket, SIGNAL(connected()), this, SLOT(OnConnected()));
   connect(m_Socket, SIGNAL(readyRead()), this, SLOT(OnIncomingData()));
   connect(m_Socket, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));
   connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(OnError(QAbstractSocket::SocketError)));
   m_Socket->connectToHost(m_PeerIP,m_Port);

}

void TCPClient::close()
{
    if (m_Socket!=nullptr)
    {
#ifdef FFS_DEBUG
        m_log->log("TCPClient : " + tr("Close socket IP= ") + m_PeerIP.toString(), Qt::cyan,LEVEL_NORMAL);
#endif
        if(m_Socket->isOpen())
        {
            m_Socket->close();
        }
    }
    m_Port=0;
    m_PeerIP.clear();
}

void TCPClient::send(const QNetworkDatagram& pData)
{
    if (m_Socket!=nullptr)
    {
        if (m_Socket->isOpen())
        {
#ifdef FFS_DEBUG
            m_log->log("TCPClient : " + tr("send data to ") +
                m_Socket->peerAddress().toString() +
                " port = " + QString::number(m_Socket->peerPort()) +
                " datagram = " + pData.data().toHex(), Qt::cyan,LEVEL_VERBOSE);
#endif
            m_Socket->write(pData.data());
            m_Socket->flush();
        }
    }
}

QHostAddress TCPClient::getPeerAddress()
{
    return m_PeerIP;
}

quint16 TCPClient::getPeerPort()
{
    return m_Port;
}

void TCPClient::OnIncomingData()
{
    if (m_Socket==nullptr) return;
    QNetworkDatagram datagram;
    datagram.setData(m_Socket->readAll());
    datagram.setSender(m_PeerIP,m_Port);
#ifdef FFS_DEBUG
    m_log->log("TCPClient : " + tr("receive data from ") +
        m_Socket->peerAddress().toString() +
        " port = " + QString::number(m_Socket->peerPort()) +
        " datagram = " + datagram.data().toHex(), Qt::cyan,LEVEL_VERBOSE);
#endif
    emit(fireIncomingData(datagram));
}

void TCPClient::OnConnected()
{
    if (m_Socket==nullptr) return;
    if(m_Socket->isOpen())
    {
#ifdef FFS_DEBUG
        m_log->log("TCPClient : " + tr("new socket connection Peer IP= ") +
            m_Socket->peerAddress().toString() +
            " port = " + QString::number(m_Socket->peerPort())
            , Qt::cyan,LEVEL_NORMAL);
#endif
        m_PeerIP = m_Socket->peerAddress();
        m_Port = m_Socket->peerPort();
    }

}

void TCPClient::OnDisconnected()
{
    emit(fireDisconnected(this));
#ifdef FFS_DEBUG
    m_log->log("TCPClient : " + tr("Peer disconnected IP = ") +
               m_PeerIP.toString(), Qt::cyan,LEVEL_NORMAL);
#endif
    this->deleteLater();
}

void TCPClient::OnError(QAbstractSocket::SocketError pError)
{
#ifdef FFS_DEBUG
    m_log->log("TCPClient : " + tr("Socket error = ") + m_Socket->errorString(), Qt::cyan,LEVEL_NORMAL);
#endif
    emit(fireDisconnected(this));
    this->deleteLater();
}
