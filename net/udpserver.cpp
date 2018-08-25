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
 * udpserver.cpp is part of FF2Play project
 *
 * This class provide an udp server to receive and send data over networks
 * **************************************************************************/

#include "net/udpserver.h"

#include <QTextCodec>
#include <QNetworkDatagram>

namespace ffs2play
{
    ///
    /// \brief UDPServer::UDPServer
    /// \param parent
    ///
    UDPServer::UDPServer(QObject *pParent) : QObject(pParent)
    {
        qRegisterMetaType<QNetworkDatagram>("QNetworkDatagram");
        m_log = CLogger::instance();
    #ifdef FFS_DEBUG
        m_log->log("UPDServer : " + tr("constructor"), Qt::darkBlue,LEVEL_NORMAL);
    #endif
        m_running = false;
        m_pListenner = new QUdpSocket(this);
        connect(m_pListenner, SIGNAL(readyRead()),this, SLOT(incomingData()));
        m_port = 0;
    }

    ///
    /// \brief UDPServer::setPort
    /// \param pPort
    ///
    void UDPServer::setPort(int pPort)
    {
        if (!m_running) m_port = pPort;
    }

    ///
    /// \brief UDPServer::start
    ///
    void UDPServer::start ()
    {
        if ((!m_running) && (m_pListenner!=NULL))
        {
            if (m_pListenner->isOpen()) stop();
            m_running = m_pListenner->bind (QHostAddress::AnyIPv4, m_port);
#ifdef FFS_DEBUG
            m_log->log("UPDServer : " + tr("listenning on port ") + QString::number(m_port), Qt::darkBlue,LEVEL_NORMAL);
#endif
        }
    }

    ///
    /// \brief UDPServer::stop
    ///
    void UDPServer::stop ()
    {
        if ((m_running) && (m_pListenner !=NULL))
        {
            m_pListenner->close();
            m_running = false;
        }
    }

    ///
    /// \brief UDPServer::~UDPServer
    ///
    UDPServer::~UDPServer()
    {
        stop();
    }

    ///
    /// \brief UDPServer::incomingData
    ///
    void UDPServer::incomingData()
    {
        while (m_pListenner->hasPendingDatagrams())
        {
            QNetworkDatagram datagram = m_pListenner->receiveDatagram();
#ifdef FFS_DEBUG
            m_log->log("UDPServer : " + tr("receive data from ") +
                datagram.senderAddress().toString() +
                " port = " + QString::number(datagram.senderPort()) +
                " datagram = " + datagram.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
            emit(fireIncomingData(datagram));
        }
    }

    ///
    /// \brief UDPServer::send
    /// \param pData
    ///
    void UDPServer::send(const QNetworkDatagram& pData)
    {
        if ((m_running) && (m_pListenner !=NULL))
        {
#ifdef FFS_DEBUG
            m_log->log("UDPServer : " + tr("send data to ") +
                pData.destinationAddress().toString() +
                " port = " + QString::number(pData.destinationPort()) +
                " datagram = " + pData.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
            m_pListenner->writeDatagram(pData);
        }
    }
}
