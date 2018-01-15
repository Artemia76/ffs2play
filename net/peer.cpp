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

#ifdef Q_OS_WIN
	#include <windows.h>
#endif

namespace ffs2play
{
	CPeer::CPeer(
			UDPServer* pServer,
			QHostAddress pExternalIP,
			int pPort,
			QString pCallSign,
			bool pDisabled,
			bool pLocal,
			const QList<QHostAddress>* pInternalIP,
			QObject *pParent
			) : QObject(pParent),
				CallSign(m_callSign),
				Disabled(m_disabled)
	{
		m_log = CLogger::instance();
		m_AIMapping = AIMapping::instance();
		m_simManager = simbasemanager::instance();
		m_callSign = pCallSign;
		m_server = pServer;
		m_externalIP = pExternalIP;
		if (pInternalIP != NULL)
		{
			m_internalIP = *pInternalIP;
		}
		m_port = pPort;
		m_local = pLocal;
		if (!m_local) m_EP = m_externalIP;
		else m_EP.clear();
		m_disabled = true;
		m_sel_iplocal = 0;
		m_trySimpleAI = false;
		Disable(pDisabled);
		m_visible = false;
		m_spawnable = false;
		connect(&m_heartBeat,SIGNAL(timeout()),SLOT(OnHeartBeat()));
	}

	CPeer::~CPeer()
	{
		Stop();
	}
	void CPeer::Disable(bool pFlag)
	{
		if (m_disabled == pFlag) return;
		else
		{
			if (pFlag) Stop();
			else Start();
		}
	}
	void CPeer::Start()
	{
		if (!m_disabled) return;
		connect(m_server, SIGNAL(fireIncomingData(const QNetworkDatagram*)), this, SLOT(OnReceiveMessage(const QNetworkDatagram*)));
		connect(m_simManager, SIGNAL(fireSimVariableEvent(CAircraftState)), this, SLOT(OnStateChange(CAircraftState)));
		m_heartBeat.start(5000);
		m_lastData = Now();
		m_lastStateEvent = m_lastData;
		m_counter = 0;
		m_counter_In = 0;
		m_counter_Out = 0;
		m_distance = -1;
		m_onLine = false;
		m_sendInterval = milliseconds(5000);
		m_ecart = 0;
		m_objectID = 0;
		m_version = 0;
		m_spawned = 0;
		//m_Refresh = false;
		m_miniPing = milliseconds(1000);
		//m_PredictiveTime = 1000;
		m_settings.beginGroup("P2P");
		if (m_settings.value("P2PInfoEnable", true).toBool())
		{
			if (m_simManager->getVersion()!=SIM_VERSION::XPLANE)
				m_simManager->sendScrollingText(CallSign + tr(" is connected on P2P Network"));
		}
		m_settings.endGroup();
		m_disabled = false;
	}

	void CPeer::Stop()
	{
		if (m_disabled) return;
		//Spawn_AI(false);
		m_heartBeat.stop();
		disconnect(m_server, SIGNAL(fireIncomingData(const QNetworkDatagram*)), this,SLOT(OnReceiveMessage(const QNetworkDatagram*)));
		disconnect(m_simManager, SIGNAL(fireSimVariableEvent(ffs2play::CAircraftState)),this, SLOT(OnStateChange(ffs2play::CAircraftState)));
		//m_TimerCreateAI.Stop();
		//m_TimerCreateAI.Dispose();
		//m_Analyseur.OnStateChange -= OnStateChange;
		//m_SC.OnSCReceiveEvent -= OnSCReceiveEvent;
		//m_SC.OnSCReceiveEventFrame -= OnSCReceiveEventFrame;
		//m_SC.OnAICreated -= OnAICreated;
		//m_SC.OnAIRemoved -= OnAIRemoved;
		//Server.OnReceiveMessage -= OnReceiveMessage;
		//m_SC.OnSCReceiveAIUpdate -= OnSCReceiveAIUpdate;
		m_settings.beginGroup("P2P");
		if (m_settings.value("P2PInfoEnable", true).toBool())
		{
			if (m_simManager->getVersion()!=SIM_VERSION::XPLANE)
				m_simManager->sendScrollingText(CallSign + tr(" leave from P2P Network"));
		}
		m_settings.endGroup();
		m_disabled = true;
		m_spawned = 0;
	}

	void CPeer::sendPing()
	{
		m_lastPing = Now();
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::PING;
		QNetworkDatagram Output(Data,m_EP,m_port);
		m_server->send(Output);
#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
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
		QNetworkDatagram Output(Data,m_EP,m_port);
		m_server->send(Output);
#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
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
		QNetworkDatagram Output(Data,m_EP,m_port);
		m_server->send(Output);
#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
	}

	void CPeer::requestVersion()
	{
		QByteArray Data;
		QDataStream stream(&Data,QIODevice::WriteOnly);
		stream.setByteOrder(QDataStream::LittleEndian);
		stream << (quint8)Protocol::REQ_VERSION;
		QNetworkDatagram Output(Data,m_EP,m_port);
		m_server->send(Output);
#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
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
		std::string adata=m_sendData.SerializePartialAsString();
		stream << QString::fromStdString(adata);
		QNetworkDatagram Output(Data,m_EP,m_port);
		m_server->send(Output);
#ifdef FFS_DEBUG
		m_log->log("Peer [" + m_callSign + "] " + tr("send data to ") +
					Output.destinationAddress().toString() +
					" port = " + QString::number(Output.destinationPort()) +
					" datagram = " + Output.data().toHex(), Qt::darkBlue,LEVEL_VERBOSE);
#endif
	}
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
	}

	void CPeer::OnHeartBeat()
	{
		if (!m_EP.isNull()) sendPing();
	}
}
