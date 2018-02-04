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
#include <QBuffer>

namespace ffs2play
{
	void CPeer::send(const QByteArray &pOutput)
	{
		if (!m_EP.isNull())
		{
			QNetworkDatagram Output(pOutput,m_EP, m_port);
			m_server->send(Output);
		}
		else
		{
			if (m_local && (m_internalIP.count()>0))
			{
				if (m_sel_iplocal >= m_internalIP.count()) m_sel_iplocal = 0;
				QNetworkDatagram Output(pOutput,m_internalIP[m_sel_iplocal], m_port);
				m_server->send(Output);
				m_sel_iplocal++;
			}
		}
	}

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
		stream << (quint8)Protocol::CHAT;
		stream << (byte)Message.length();
		stream.writeRawData(Message.toStdString().c_str(),Message.length());
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
		stream << (byte)m_sendData.title().length();
		stream.writeRawData(m_sendData.title().c_str(),m_sendData.title().length());
		stream << (byte)m_sendData.type().length();
		stream.writeRawData(m_sendData.type().c_str(),m_sendData.type().length());
		stream << (byte)m_sendData.model().length();
		stream.writeRawData(m_sendData.model().c_str(),m_sendData.model().length());
		stream << (byte)m_sendData.category().length();
		stream.writeRawData(m_sendData.category().c_str(),m_sendData.category().length());
		send(Data);
	}

	void CPeer::requestVersion()
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::REQ_VERSION;
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
		char* buffer = new char[m_sendData.ByteSize()];
		m_sendData.SerializeToArray(buffer,m_sendData.ByteSize());
		stream.writeRawData(buffer,m_sendData.ByteSize());
		delete buffer;
		send(Data);
	}
}
