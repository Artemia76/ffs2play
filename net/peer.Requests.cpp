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

#include "peer.h"

namespace ffs2play
{
	void CPeer::send(const QByteArray &pOutput)
	{
		if (!m_EP.isNull())
		{
			QNetworkDatagram Output(pOutput,m_EP, m_port);
			m_server->send(Output);
#ifdef FFS_DEBUG
			m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
				Output.destinationAddress().toString() +
				" port = " + QString::number(Output.destinationPort()) +
				" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
		}
		else
		{
			if (m_local && (m_internalIP.count()>0))
			{
				if (m_sel_iplocal >= m_internalIP.count()) m_sel_iplocal = 0;
				QNetworkDatagram Output(pOutput,m_internalIP[m_sel_iplocal], m_port);
				m_server->send(Output);
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
				m_sel_iplocal++;
			}
		}
	}

	void CPeer::sendChat(const QString &pMessage)
	{
		if (!m_onLine) return;
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::CHAT;
		stream << pMessage;
		send(Data);
	}

	// SendPing

	void CPeer::sendPing()
	{
		m_lastPing = Now();
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::PING;
		send(Data);
		m_counter++;
		if ((m_counter > 2) && m_onLine)
		{
#ifdef FFS_DEBUG
			if (m_counter > 2)
				m_log->log("Peer [" + m_callSign + "] " + tr(" go offline = Timeout"),Qt::darkBlue,LEVEL_NORMAL);
			else
				m_log->log("Peer [" + m_callSign + "] " + tr(" go offline = Diconnected"),Qt::darkBlue,LEVEL_NORMAL);
#endif
			m_onLine = false;
			m_version = 0;
			m_miniPing = milliseconds(1000);
			//Spawn_AI(false);
		}
	}

	void CPeer::sendPong(const datetime& pTime)
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::PONG;
		stream << (quint64)toTimeStamp(pTime);
		send(Data);
	}

	void CPeer::sendVersion()
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::VERSION;
		stream << PROTO_VERSION;
		stream << QString::fromStdString(m_sendData.title());
		stream << QString::fromStdString(m_sendData.type());
		stream << QString::fromStdString(m_sendData.model());
		stream << QString::fromStdString(m_sendData.category());
		send(Data);
	}

	void CPeer::requestVersion()
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::REQ_VERSION;
		QNetworkDatagram Output(Data,m_EP,m_port);
		send(Data);
	}

	void CPeer::sendData()
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::DATA;
		if (m_counter_Out == 255) m_counter_Out = 0;
		else m_counter_Out++;
		stream << m_counter_Out;
		stream << QByteArray::fromStdString(m_sendData.SerializeAsString());
		send(Data);
	}
}
