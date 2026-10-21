/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
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
** udpserver.cpp is part of FF2Play project
**
** This class provide an udp server to receive and send data over networks
****************************************************************************/

#include "net/udpserver.h"


///
/// \brief UDPServer::UDPServer
/// \param parent
///
UDPServer::UDPServer(QObject *pParent) : QObject(pParent)
{
    qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");
#ifdef FFS_DEBUG
    m_log.log("UPDServer : constructor", Qt::darkBlue,LEVEL_NORMAL);
#endif
    m_running = false;
    m_pListenner = new QUdpSocket(this);
    m_pListenner->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    connect(m_pListenner, &QUdpSocket::readyRead, this, &UDPServer::OnIncomingData);
    m_port = 0;
}

///
/// \brief UDPServer::~UDPServer
///
UDPServer::~UDPServer()
{
    stop();
}

///
/// \brief UDPServer::setPort
/// \param pPort
///
void UDPServer::setPort(quint16 pPort)
{
    if (!m_running) m_port = pPort;
}

///
/// \brief UDPServer::start
///
bool UDPServer::start ()
{
    if ((!m_running) && (m_pListenner!=nullptr))
    {
        if (m_pListenner->isOpen()) stop();
        m_running = m_pListenner->bind (QHostAddress::Any, m_port);
        if (m_running)
        {
#ifdef FFS_DEBUG
            m_log.log("UPDServer : listenning on port " + QString::number(m_port), Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
        else
        {
            m_log.log("UDPServer : Failed to open UDP port " + QString::number(m_port) + ",Reason = " + m_pListenner->errorString(), Qt::darkMagenta);
        }
    }
    return m_running;
}

///
/// \brief UDPServer::stop
///
void UDPServer::stop ()
{
    if ((m_running) && (m_pListenner !=nullptr))
    {
        m_pListenner->close();
        m_running = false;
    }
}

///
/// \brief UDPServer::incomingData
///
void UDPServer::OnIncomingData()
{
    while (m_pListenner->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_pListenner->receiveDatagram();
#ifdef FFS_DEBUG
        m_log.log("UDPServer : receive data from " +
            datagram.senderAddress().toString() +
            " port = " + QString::number(datagram.senderPort()) +
            " datagram = " + datagram.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
        emit fireIncomingData(datagram);
    }
}

///
/// \brief UDPServer::send
/// \param pData
///
void UDPServer::send(const QNetworkDatagram& pData)
{
    if ((m_running) && (m_pListenner !=nullptr))
    {
#ifdef FFS_DEBUG
        m_log.log("UDPServer : send data to " +
            pData.destinationAddress().toString() +
            " port = " + QString::number(pData.destinationPort()) +
            " datagram = " + pData.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
        m_pListenner->writeDatagram(pData);
    }
}
