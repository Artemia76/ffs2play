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
#include "tools/calculation.h"

namespace ffs2play
{
	void CPeer::OnReceiveMessage(const QNetworkDatagram* pDatagram)
	{
	#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("receive data from ") +
					pDatagram->senderAddress().toString() +
					" port = " + QString::number(pDatagram->senderPort()) +
					" datagram = " + pDatagram->data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		if(m_EP.isNull())
		{
		  if(m_internalIP.contains(pDatagram->senderAddress()))
		  {
			  m_EP=pDatagram->senderAddress();
		  }
		  else return;
		}
		else if((!m_EP.isEqual(pDatagram->senderAddress())) || (m_port != pDatagram->senderPort())) return;

		datetime curr_time=Now();
		QByteArray data(pDatagram->data());
		QDataStream stream(&data,QIODevice::ReadOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		quint8 Code;
		stream>>Code;
		data.remove(0,1);
		switch ((Protocol)Code)
		{
			case Protocol::PING:
			{
				sendPong(curr_time);
				break;
			}
			case Protocol::PONG:
			{
				m_latence = duration_cast<milliseconds> (curr_time - m_lastPing)/2;
				m_counter = 0;
				quint64 Time;
				stream >> Time;
				datetime TimePong = fromTimeStamp(Time);
				if (m_miniPing > m_latence)
				{
					m_decalage = duration_cast<milliseconds> (curr_time - m_latence - TimePong);
					m_miniPing = m_latence;
				}
				if (!m_onLine)
				{
#ifdef FFS_DEBUG
					m_log->log("Peer [" + m_callSign + "] " + tr(" go online"),Qt::darkBlue,LEVEL_NORMAL);
#endif
					m_onLine = true;
					m_distance = -1;
					m_counter_In = 0;
					m_counter_Out = 0;
				}
				if (m_version == 0) requestVersion();
				break;
			}
			case Protocol::CHAT:
			{
				QString Message;
				stream >> Message;
				m_simManager->sendScrollingText(CallSign + " : " + Message);
				break;
			}
			case Protocol::DATA:
			{
				if (m_version == PROTO_VERSION)
				{
					if(m_blockData) return;
					m_mutex.lock();
					quint8 CounterIn;
					stream >> CounterIn;
					// If counter received is greater we have fresh data
					if (((CounterIn - m_counter_In) > 0) || (m_counter_In == 255))
					{
						m_oldData.CopyFrom(m_data);
						quint32 Len;
						stream >> Len;
						data.remove(0,5);
						m_data.ParseFromString(data.toStdString());
						m_data.set_timestamp(m_data.timestamp()+ duration_cast<milliseconds>(m_decalage).count());
						if (m_data.timestamp() <= m_oldData.timestamp())
						{
#ifdef FFS_DEBUG
							m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram dropped : too late"),Qt::darkBlue,LEVEL_NORMAL);
#endif
						}
						else
						{
							if (m_spawned==4) m_spawned++;
							m_refreshRate = curr_time - m_lastData;
							m_lastData = curr_time;
							m_remoteRefreshRate = milliseconds(m_data.timestamp() - m_oldData.timestamp());
							m_distance = distance(m_data.latitude(),m_data.longitude(),m_sendData.latitude(),m_sendData.longitude(), 'N');
#ifdef FFS_DEBUG
							m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram missing quantity = ") + QString::number(CounterIn - m_counter_In - 1),Qt::darkBlue,LEVEL_NORMAL);
#endif
							if (m_spawned >=5) refreshData();
						}
					}
					else
					{
#ifdef FFS_DEBUG
						m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram dropped : too late since ") + QString::number(m_counter_In - CounterIn),Qt::darkBlue,LEVEL_NORMAL);
#endif
						m_counter_In = CounterIn;
					}
					m_mutex.unlock();
				}
				break;
			}
			case Protocol::VERSION:
			{
				stream >> m_version;
				if (m_version == PROTO_VERSION)
				{
					QString Title,Type,Model,Category;
					stream >> Title;
					stream >> Type;
					stream >> Model;
					stream >> Category;
					m_data.set_title(Title.toStdString());
					m_data.set_type(Type.toStdString());
					m_data.set_model(Model.toStdString());
					m_data.set_category(Category.toStdString());
				}
	#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" received version number = ") + QString::number(m_version),Qt::darkBlue,LEVEL_NORMAL);
	#endif
				break;
			}
			case Protocol::REQ_VERSION:
			{
				sendVersion();
				break;
			}
		}
	}

	void CPeer::OnHeartBeat()
	{
		if (!m_EP.isNull()) sendPing();
	}

	void CPeer::OnStateChange(CAircraftState pState)
	{
		bool Title_Changed = false;
		if (m_sendData.title() != pState.Title.toStdString())
		{
			Title_Changed = true;
		}
		pState.toAirData(m_sendData);
		if (duration_cast<milliseconds>(pState.TimeStamp - m_lastStateEvent) < m_sendInterval) return;
		if ((!m_onLine) || (m_EP.isNull()) || (m_version < 1)) return;
		sendData();
		if (Title_Changed) sendVersion();
		m_lastStateEvent = pState.TimeStamp;
		// AI Display Management
		m_spawnable = (m_distance < P2PRadius) && (m_distance >=0);
		if (m_spawnable)
		{
			m_sendInterval = milliseconds(P2PRate);
		}
		else
		{
			m_sendInterval =  milliseconds(5000);
		}
		if (m_spawnable && m_visible && (m_spawned == 0))
		{
			spawnAI(true);
#ifdef FFS_DEBUG
			m_log->log("Peer [" + m_callSign + "] " + tr(" Build AI Object") ,Qt::darkBlue,LEVEL_NORMAL);
#endif
		}
		if (((!m_spawnable) || (!m_visible)) && (m_spawned > 0))
		{
			spawnAI(false);
#ifdef FFS_DEBUG
			m_log->log("Peer [" + m_callSign + "] " + tr(" Destruct AI Object Visible=") + QString::number(m_visible) + tr(" Spawnable = ") + QString::number(m_spawnable),Qt::darkBlue,LEVEL_NORMAL);
#endif
		}
	}
}
