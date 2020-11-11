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
** tcpserver.cpp is part of FF2Play project
**
** This class provide an tcp server to receive and send data over networks
****************************************************************************/
#include "tcpserver.h"

///
/// \brief TCPServer::TCPServer
/// \param parent
///
TCPServer::TCPServer(QObject *parent) :
    QTcpServer(parent)
{
    qRegisterMetaType<TCPClient*>("TCPClient*");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    m_log = CLogger::instance();
#ifdef FFS_DEBUG
    m_log->log("TCPServer : constructor", Qt::darkBlue,LEVEL_NORMAL);
#endif
    m_running = false;
    m_port = 0;
}

///
/// \brief TCPServer::~TCPServer
///
TCPServer::~TCPServer()
{
    stop();
}

///
/// \brief TCPServer::setPort
/// \param pPort
///
void TCPServer::setPort(quint16 pPort)
{
    if (!m_running) m_port = pPort;
}

///
/// \brief TCPServer::start
///
bool TCPServer::start ()
{
    if (this->isListening()) stop();
    m_running = this->listen(QHostAddress::Any, m_port);
    if (m_running)
    {
#ifdef FFS_DEBUG
        m_log->log("TCPServer : listenning on port " + QString::number(m_port), Qt::darkBlue,LEVEL_NORMAL);
#endif
        connect(this,SIGNAL(newConnection()),this,SLOT(OnNewConnection()));
    }
    else
    {
        m_log->log("TCPServer : Failed to open TCP port " + QString::number(m_port), Qt::darkMagenta);
    }
    return m_running;
}

///
/// \brief TCPServer::stop
///
void TCPServer::stop ()
{
    disconnect(this,SIGNAL(newConnection()),this,SLOT(OnNewConnection()));
    if (this->isListening())
    {
        this->close();
        m_running = false;
    }
}

void TCPServer::OnNewConnection()
{
    QTcpSocket* socket = nextPendingConnection();
    if (socket==nullptr) return;
#ifdef FFS_DEBUG
    m_log->log("TCPServer : incomingConnection...", Qt::darkBlue,LEVEL_NORMAL);
#endif
    TCPClient* client = new TCPClient(this);
    client->handle(socket);
    connect(client, SIGNAL(fireDisconnected(TCPClient*)), this, SLOT(OnCloseClient(TCPClient*)));
    m_clients.append(client);
    fireNewConnexion(client);
}

void TCPServer::accept(TCPClient*)
{
}

void TCPServer::OnCloseClient(TCPClient* pPeer)
{
#ifdef FFS_DEBUG
    m_log->log("TCPServer : Peer Disconnected : " + pPeer->getPeerAddress().toString(), Qt::darkBlue,LEVEL_NORMAL);
#endif
    int i= m_clients.indexOf(pPeer);
    if (i!=-1)
        m_clients.removeAt(i);
}
