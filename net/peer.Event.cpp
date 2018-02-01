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
	void CPeer::OnReceiveMessage(QNetworkDatagram pDatagram)
	{
	#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("receive data from ") +
					pDatagram.senderAddress().toString() +
					" port = " + QString::number(pDatagram.senderPort()) +
					" datagram = " + pDatagram.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		if(m_EP.isNull())
		{
		  if(m_internalIP.contains(pDatagram.senderAddress()))
		  {
			  m_EP=pDatagram.senderAddress();
		  }
		  else return;
		}
		else if((!m_EP.isEqual(pDatagram.senderAddress())) || (m_port != pDatagram.senderPort())) return;

		datetime curr_time=Now();
		QByteArray data(pDatagram.data());
		QDataStream stream(&data,QIODevice::ReadOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		quint8 Code;
		stream>>Code;
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
						m_oldData=m_data;
						quint32 Len;
						stream >> Len;
						data.remove(0,6);
						AirData ReceivedData;
						ReceivedData.ParseFromString(data.toStdString());
						m_data.fromAirData(ReceivedData);
#ifdef FFS_DEBUG
						m_log->log("Peer [" + m_callSign + "] " + tr(" LAtitude = ") + QString::number(m_data.Latitude), Qt::darkBlue,LEVEL_NORMAL);
#endif
						m_data.TimeStamp += m_decalage;
						if (m_data.TimeStamp<= m_oldData.TimeStamp)
						{
#ifdef FFS_DEBUG
							m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram dropped : too late") +
										" Décalage mSec=" + QString::number(m_decalage.count()) +
									   " OldTime = " + QString::number(toTimeStamp(m_oldData.TimeStamp)) +
									   " NewTime = " + QString::number(toTimeStamp(m_data.TimeStamp))
									   ,Qt::darkBlue,LEVEL_NORMAL);
#endif
						}
						else
						{
							if (m_spawned==4) m_spawned++;
							m_refreshRate = curr_time - m_lastData;
							m_lastData = curr_time;
							m_remoteRefreshRate = m_data.TimeStamp - m_oldData.TimeStamp;
							m_distance = distance(m_data.Latitude,m_data.Longitude,m_sendData.latitude(),m_sendData.longitude(), 'N');
#ifdef FFS_DEBUG
							if ((CounterIn - m_counter_In) > 1) m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram missing quantity = ") + QString::number(CounterIn - m_counter_In - 1),Qt::darkBlue,LEVEL_NORMAL);
#endif
							if (m_spawned >=5) refreshData();
						}
					}
					else
					{
#ifdef FFS_DEBUG
						m_log->log("Peer [" + m_callSign + "] " + tr(" UDP Datagram dropped : too late since ") + QString::number(m_counter_In - CounterIn),Qt::darkBlue,LEVEL_NORMAL);
#endif
					}
					m_counter_In = CounterIn;
					m_mutex.unlock();
				}
				break;
			}
			case Protocol::VERSION:
			{
				stream >> m_version;
				if (m_version == PROTO_VERSION)
				{
					stream >> m_data.Title;
					stream >> m_data.Type;
					stream >> m_data.Model;
					stream >> m_data.Category;
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
		sendPing();
	}

	void CPeer::OnSimEventFrame(float pFrameRate, float)
	{
		if (m_spawned >=6)
		{
			updateAI(Now(),pFrameRate);
		}
	}

	void CPeer::OnSimAICreated(QString pTail,uint pObjectID)
	{
		if (pTail == m_tail)
		{
			if ((pObjectID != 0)&&(m_objectID==0))
			{
				m_objectID = pObjectID;
				m_spawned = 4;
				m_simManager->freezeAI(m_objectID, m_data.OnGround);
				m_lastAIUpdate = Now();
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Receive Object_ID = ") + QString::number(m_objectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
			}
			else
			{
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Receive an unwanted object") + QString::number(m_objectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
			}
		}
	}

	void CPeer::OnSimAIRemoved(uint pObjectID)
	{
		if (pObjectID == m_objectID)
		{
			m_objectID = 0;
			m_spawned = 0;
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Receive a removed Object from sim ID = ") + QString::number(pObjectID),Qt::darkBlue,LEVEL_NORMAL);
#endif
		}
	}

	void CPeer::OnSimEvent(SimEvent)
	{

	}

	void CPeer::OnSimAIUpdated(CAircraftState pAIData, uint pObjectID)
	{
		if (m_objectID != pObjectID) return;
		m_lastAIUpdate = Now();
		m_AISimData = pAIData;
		m_ecart = distance(m_data.Latitude,m_data.Longitude,m_AISimData.Latitude,m_AISimData.Longitude, 'N');
	}

	void CPeer::OnSimStateChange(CAircraftState pState)
	{
		bool Title_Changed = false;
		if (m_sendData.title() != pState.Title.toStdString())
		{
			Title_Changed = true;
		}
		pState.toAirData(m_sendData);
		if ((pState.TimeStamp - m_lastStateEvent) < m_sendInterval) return;
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

	void CPeer::OnTimerCreateAI()
	{
		m_timerCreateAI.stop();
		if ((m_spawned >= 2) && (m_objectID == 0))
		{
			m_spawned = 2;
			if (!m_trySimpleAI)
			{
				m_trySimpleAI = true;
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Simulator have not created the requested AI, Let's try with simple object") ,Qt::darkBlue,LEVEL_NORMAL);
#endif
			}
			else
			{
				m_AIResolution.Titre = "Mooney Bravo";
				m_AIResolution.CG_Height = 3.7;
				m_AIResolution.Pitch = 2.9;
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Simulator have not created the requested AI, we are trying with default simulator aircraft") ,Qt::darkBlue,LEVEL_NORMAL);
#endif
			}
			createAI();
		}
		else
		{
#ifdef FFS_DEBUG
				m_log->log("Peer [" + m_callSign + "] " + tr(" Simulator has been created the requested AI") ,Qt::darkBlue,LEVEL_NORMAL);
#endif
		}
	}
}
