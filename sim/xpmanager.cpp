/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#include "xpmanager.h"
#include "singleton/singleton.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QNetworkDatagram>
#include <QTextCodec>
#include <QApplication>

namespace ffs2play
{
	///
	/// \brief CXPManager::CXPManager
	/// \param parent
	///

	CXPManager::CXPManager(QObject* parent):
		SimConnector(parent)
	{
		m_LocalPort=47600;
		m_DestPort=47500;
		Server = new UDPServer(this);
		connect (Server,SIGNAL(fireIncomingData(const QNetworkDatagram*)), this,SLOT(OnReceiveDatagram(const QNetworkDatagram*)));
		Server->setPort(m_LocalPort);
		Server->start();
	}

	CXPManager::~CXPManager()
	{
		close();
		Server->stop();
		//delete Server;
	}

	///
	/// \brief CXPManager::open
	///

	bool CXPManager::open ()
	{
		if (!m_connected)
		{
			QByteArray Data;
			QDataStream stream(&Data,QIODevice::WriteOnly);
			stream << (quint8)PING;
			QNetworkDatagram Output(Data,QHostAddress("localhost"),m_DestPort);
			Server->send(Output);
			int wait=0;
			while ((!m_connected) && (wait < 5))
			{
				QThread::sleep(1);
				QApplication::processEvents();
				wait++;
			}
			if (m_connected) emit(opened());
			return m_connected;
		}
		return false;
	}

	///
	/// \brief CXPManager::close
	///

	void CXPManager::close ()
	{
		if (m_connected)
		{
			QByteArray Data;
			QDataStream stream(&Data,QIODevice::WriteOnly);
			stream << (quint8)DISC;
			QNetworkDatagram Output(Data,QHostAddress("localhost"),m_DestPort);
			Server->send(Output);
			m_connected=false;
			m_log->log(tr("Disconnected from XPlane."));
			emit(closed());
		}
	}


	///
	/// \brief CXPManager::main_loop()
	///

	void CXPManager::main_loop()
	{
	}

	///
	/// \brief CXPManager::getVersion
	/// \return
	///

	SIM_VERSION CXPManager::getVersion()
	{
		return SIM_VERSION::XPLANE;
	}

	void CXPManager::sendScrollingText(const QString& pMessage)
	{

	}
	bool CXPManager::createAI(const QString& pAircraft,const QString& Tail,const CAircraftState& pData,bool pNonAIObject)
	{
		return false;
	}
	bool CXPManager::removeAI(uint& pObjectID)
	{
		return false;
	}
	bool CXPManager::updateAI(uint pObjectID, const CAircraftState& data)
	{
		return false;
	}
	void CXPManager::freezeAI(uint pObjectID, bool pState)
	{

	}

	///
	/// \brief CXPManager::OnReceiveDatagram
	///

	void CXPManager::OnReceiveDatagram(const QNetworkDatagram* datagram)
	{
		QByteArray data(datagram->data());
		QDataStream stream(&data,QIODevice::ReadOnly);
		quint8 Code;
		stream>>Code;
		data.remove(0,1);
		switch ((FFS2Proto)Code)
		{
			case PONG:
			{
				if (!m_connected)
				{
					m_connected=true;
				}
				break;
			}
			case DATA_SEND:
			{
				try
				{
					PlayerData.ParseFromString(data.toStdString());
					m_state.fromAirData(PlayerData);
					emit (fireSimVariableEvent(m_state));
				}
				catch (std::exception& e)
				{
					m_log->log(tr("DataSend Error : ") + QString(e.what()),Qt::magenta);
				}
				break;
			}
			default:
			{
#ifdef FFS_DEBUG
			m_log->log("XPManager : " + tr("unknown code = ") +
					QString(Code),
					Qt::darkBlue,LEVEL_VERBOSE);
#endif
			}
		}
	}


}
