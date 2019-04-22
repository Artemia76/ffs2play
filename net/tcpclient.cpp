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
    QThread(parent)
{
    qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");
    qRegisterMetaType<TCPClient*>("TCPClient*");
    m_Socket = nullptr;
    m_socketDescriptor = 0;
    m_running=false;
    m_log = CLogger::instance();
}

TCPClient::~TCPClient()
{
    Close();
}

///
/// \brief Handle existing socket from server side
/// \param ID
///
void TCPClient::handle(qintptr ID)
{
    m_socketDescriptor = ID;
    start();
    m_running=true;
}

void TCPClient::Connect(QHostAddress pHost, quint16 pPort)
{
   if (m_running) Close();
   m_PeerIP = pHost;
   m_Port = pPort;
   m_socketDescriptor=0;
   start();
}

void TCPClient::run()
{
    if (m_Socket!=nullptr)
    {
        if (m_Socket->isOpen()) Close();
    }
    m_Socket = new QTcpSocket(this);
    if (m_socketDescriptor==0)
    {
        m_Socket->connectToHost(m_PeerIP,m_Port);
        connect(m_Socket, SIGNAL(connected()), this, SLOT(Connected()));
    }
    else if (!m_Socket->setSocketDescriptor(this->m_socketDescriptor))
    {
        emit error(m_Socket->error());
        return;
    }
    else
    {
        m_PeerIP=m_Socket->peerAddress();
        m_Port =m_Socket->peerPort();
    }
#ifdef FFS_DEBUG
    m_log->log("TCPSocket : " + tr("new socket Peer IP= ") +
        m_Socket->peerAddress().toString() +
        " port = " + QString::number(m_Socket->peerPort())
        , Qt::cyan,LEVEL_NORMAL);
#endif
    connect(m_Socket, SIGNAL(readyRead()), this, SLOT(ReadyRead()), Qt::DirectConnection);
    connect(m_Socket, SIGNAL(disconnected()), this, SLOT(Disconnected()));
    m_running=true;
    while (m_running)
    {
        msleep(100);
    }
}

void TCPClient::Close()
{
    if (m_running)
    {
        if (m_Socket!=nullptr)
        {
            if(m_Socket->isOpen())
            {
                m_Socket->close();
            }
            m_Socket->deleteLater();
        }
        m_running=false;
        wait();
    }
}

void TCPClient::Send(const QNetworkDatagram& pData)
{
    if (m_Socket!=nullptr)
    {
        if (m_Socket->isOpen())
        {
            m_Socket->write(pData.data());
        }
    }
}

QHostAddress TCPClient::GetPeerAddress()
{
    return m_PeerIP;
}

quint16 TCPClient::GetPeerPort()
{
    return m_Port;
}

void TCPClient::ReadyRead()
{
    QNetworkDatagram datagram;
    datagram.setData(m_Socket->readAll());
    datagram.setDestination(m_PeerIP,m_Port);
#ifdef FFS_DEBUG
    m_log->log("TCPSocket : " + tr("receive data from ") +
        m_Socket->peerAddress().toString() +
        " port = " + QString::number(m_Socket->peerPort()) +
        " datagram = " + datagram.data().toHex(), Qt::cyan,LEVEL_VERBOSE);
#endif
    emit(fireIncomingData(datagram));
}

void TCPClient::Connected()
{
    if (m_Socket!=nullptr)
    {
        if(m_Socket->isOpen())
        {
            m_PeerIP = m_Socket->peerAddress();
            m_Port = m_Socket->peerPort();
        }
    }

}

void TCPClient::Disconnected()
{
#ifdef FFS_DEBUG
    m_log->log("TCPSocket : " + tr("Peer disconnected IP = ") +
               m_PeerIP.toString(), Qt::cyan,LEVEL_NORMAL);
#endif
    emit(fireDisconnected(this));
    m_Socket->deleteLater();
    m_socketDescriptor = 0;
    exit(0);
}
