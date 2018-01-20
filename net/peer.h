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

#ifndef PEER_H
#define PEER_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QMutex>
#include <QDataStream>
#include <QNetworkDatagram>
#include <sstream>
#include <cstddef>

#include "singleton/simbasemanager.h"
#include "singleton/aimapping.h"
#include "protobuf/ffs2play.pb.h"
#include "singleton/clogger.h"
#include "net/udpserver.h"
#include "sim/aircraftstate.h"

namespace ffs2play
{

	enum class Protocol
	{
		PING,
		PONG,
		CHAT,
		DATA,
		VERSION,
		EVENT,
		REQ_VERSION
	};

	struct Deltas
	{
		double Longitude;
		double Latitude;
		double Altitude;
		double Heading;
		double Pitch;
		double Bank;
	};

	struct PeerData
	{
		QString CallSign;
		QHostAddress externalIP;
		QVector<QHostAddress> localIPs;
		quint16 Port;
	};

	typedef unsigned char byte;

	const byte PROTO_VERSION = 13;

	// Déclaration of Singleton manager to have recursion access and friendly protection


	class CP2PManager;

	class CPeer : public QObject
	{
		Q_OBJECT
		friend class CP2PManager;
	public:
		const QString&			CallSign;
		const bool&				Disabled;
		int						P2PRadius;
		int						P2PRate;
	protected:
								CPeer(
									UDPServer*pServer,
									QHostAddress pExternalIP,
									int pPort,
									QString pCallSign,
									bool pDisabled=false,
									bool pLocal = false,
									const QVector<QHostAddress>* pInternalIP=NULL,
									QObject *pParent = nullptr,
									int pP2PRate = 200,
									int pP2PRadius = 10
								);
								~CPeer ();
		void					Start();
		void					Stop();
		void					Disable(bool pFlag);

	private:
		QSettings				m_settings;
		QTimer					m_heartBeat;
		int						m_counter;
		quint8					m_counter_In;
		quint8					m_counter_Out;
		bool					m_onLine;
		QString					m_callSign;
		QString					m_tail;
		simbasemanager*			m_simManager;
		AIMapping*				m_AIMapping;
		QHostAddress			m_EP;
		QVector<QHostAddress>	m_internalIP;
		QHostAddress			m_externalIP;
		quint16					m_port;
		AirData					m_data;
		AirData					m_oldData;
		AirData					m_sendData;
		AirData					m_futurData;
		AirData					m_actualPos;
		CAIBase					m_AIData;
		datetime				m_lastPing;
		datetime				m_lastData;
		timespan				m_sendInterval;
		datetime				m_lastStateEvent;
		UDPServer*				m_server;
		CLogger*				m_log;
		timespan				m_latence;
		double					m_distance;
		timespan				m_refreshRate;
		timespan				m_remoteRefreshRate;
		timespan				m_decalage;
		timespan				m_miniPing;
		quint32					m_objectID;
		byte					m_version;
		int						m_spawned;
		bool					m_refresh;
		timespan				m_predictiveTime;
		QVector<double>			m_frameRateArray;
		QTimer					m_timerCreateAI;
		AIResol					m_AIResolution;
		int						m_frameCount;
		double					m_ecart;
		datetime				m_lastAIUpdate;
		bool					m_disabled;
		QMutex					m_mutex;
		bool					m_selected;
		bool					m_local;
		int						m_sel_iplocal;
		bool					m_trySimpleAI;
		datetime				m_lastRender;
		double					m_deltaAltitude;
		double					m_deltaLongitude;
		double					m_deltaLatitude;
		double					m_deltaHeading;
		double					m_deltaPitch;
		double					m_deltaBank;
		bool					m_visible;
		bool					m_spawnable;
		double					m_old_fps;
		bool					m_blockData;

		///
		/// \brief send
		/// \param pOutput
		///
		void					send(const QByteArray& pOutput);

		///
		/// \brief sendChat
		/// \param pMessage
		///
		void					sendChat(const QString& pMessage);

		///
		/// \brief sendPing
		///
		void					sendPing();

		///
		/// \brief sendPong
		/// \param pTime
		///
		void					sendPong(const datetime& pTime);

		///
		/// \brief sendVersion
		///
		void					sendVersion();

		///
		/// \brief requestVersion
		///
		void					requestVersion();

		///
		/// \brief sendData
		///
		void					sendData();

		///
		/// \brief spawnAI
		/// \param pFlag
		///
		void					spawnAI(bool pFlag);
		void					refreshData();
		void					createAI();
	signals:

	private slots:
		void					OnReceiveMessage(const QNetworkDatagram* pDatagram);
		void					OnStateChange(CAircraftState pState);
		void					OnHeartBeat();
	};
}
#endif // PEER_H
