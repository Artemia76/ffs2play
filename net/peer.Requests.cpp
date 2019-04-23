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
** Peer.Requests.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "net/peer.h"
#include <QBuffer>

///
/// \brief CPeer::send
/// \param pOutput
///
void CPeer::send(const QByteArray &pOutput)
{
    if (!m_EP.isNull())
    {
        QNetworkDatagram Output(pOutput,m_EP, m_port);
        if (m_PeerMode==UDP)
            m_UDPServer->send(Output);
        else if ((m_PeerMode==TCP_CLIENT) || (m_PeerMode==TCP_SERVER))
            m_TCPClient->send(Output);
    }
    else
    {
        if (m_local && (m_internalIP.count()>0))
        {
            if (m_sel_iplocal >= m_internalIP.count()) m_sel_iplocal = 0;
            QNetworkDatagram Output(pOutput,m_internalIP[m_sel_iplocal], m_port);
            m_UDPServer->send(Output);
            m_sel_iplocal++;
        }
    }
}

///
/// \brief CPeer::sendChat
/// \param pMessage
///
void CPeer::sendChat(const QString &pMessage)
{
    QString Message=pMessage;
    if (!m_onLine) return;
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    if (Message.length()>255)
    {
        Message=Message.left(255);
    }
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::CHAT);
    StringToStream(stream,pMessage);
    send(Data);
}

///
/// \brief CPeer::sendPing
///
void CPeer::sendPing()
{
    m_lastPing = Now();
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::PING);
    send(Data);
    m_counter++;
    if ((m_counter > 2) && m_onLine)
    {
#ifdef FFS_DEBUG
        if (m_counter > 2)
            m_log->log("Peer [" + m_callSign + "] " + " go offline = Timeout",Qt::darkBlue,LEVEL_NORMAL);
        else
            m_log->log("Peer [" + m_callSign + "] " + " go offline = Diconnected",Qt::darkBlue,LEVEL_NORMAL);
#endif
        m_onLine = false;
        m_version = 0;
        m_miniPing = milliseconds(1000);
        spawnAI(false);
    }
    if ((m_counter > 7) && (!m_onLine) && (m_PeerMode==UDP) && (m_TCPClient==nullptr) && (!m_EP.isNull()))
    {
#ifdef FFS_DEBUG
        m_log->log("Peer [" + m_callSign + "] " + " 5 retry occured, switch to TCPClient mode",Qt::darkBlue,LEVEL_NORMAL);
#endif
        m_TCPClient = new TCPClient(this);
        m_TCPClient->connectToHost(m_EP, m_port);
        connect(m_TCPClient, SIGNAL(fireIncomingData(QNetworkDatagram)), this, SLOT(OnReceiveMessage(QNetworkDatagram)));
        connect(m_TCPClient, SIGNAL(fireDisconnected(TCPClient*)), this, SLOT(OnTCPClientDisconnected(TCPClient*)));
        m_PeerMode=TCP_CLIENT;
    }
}

///
/// \brief CPeer::sendPong
/// \param pTime
///
void CPeer::sendPong(const datetime& pTime)
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::PONG);
    stream << static_cast<quint64>(toTimeStamp(pTime));
    send(Data);
}

///
/// \brief CPeer::sendVersion
///
void CPeer::sendVersion()
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::VERSION);
    stream << PROTO_VERSION;
    StringToStream(stream,QString::fromStdString(m_sendData.title()));
    StringToStream(stream,QString::fromStdString(m_sendData.type()));
    StringToStream(stream,QString::fromStdString(m_sendData.model()));
    StringToStream(stream,QString::fromStdString(m_sendData.category()));
    send(Data);
}

///
/// \brief CPeer::requestVersion
///
void CPeer::requestVersion()
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::REQ_VERSION);
    send(Data);
}

///
/// \brief CPeer::sendData
///
void CPeer::sendData()
{
    QByteArray Data;
    QDataStream stream(&Data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<quint8>(Protocol::DATA);
    if (m_counter_Out == 255) m_counter_Out = 0;
    else m_counter_Out++;
    stream << m_counter_Out;
    ProtobufToStream(stream,m_sendData);
    send(Data);
}
