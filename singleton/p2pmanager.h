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

#ifndef P2PMANAGER_H
#define P2PMANAGER_H

#include <QObject>
#include <QSettings>
#include "singleton/clogger.h"
#include "net/udpserver.h"
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "protobuf/ffs2play.pb.h"
#include "net/peer.h"

namespace ffs2play
{
	class CP2PManager : public QObject
	{
		Q_OBJECT
	private:
							CP2PManager(QObject *parent = 0);
		static CP2PManager*	createInstance();
		void				initUPNP(bool flag);
		CLogger*			m_log;
		UDPServer*			m_server;
		QSettings			m_settings;
		int					m_port;
		int					m_AILimit;
		int					m_AIRadius;
		int					m_P2PRate;
		bool				m_bUPNP;
		bool				m_Shadow;
		LocalIPList			m_InternalIP;
		QHostAddress		m_ExternalIP;
		QString				m_CallSign;
		UPNPUrls			m_urls;
		IGDdatas			m_data;
		QVector<CPeer*>		m_peers;
		QTimer				m_checkAITimer;
	public:
		~CP2PManager();
		static CP2PManager*	instance();
		void				init (bool pFlag);
		int					getPort();
		QString				getLocalIPs();
		void				setCallsign(const QString& pCallSign);
		QString				getCallsign();
		void				whazzup_Update(QVector<PeerData>& pWhazzup);
		bool				isDisabled(const QString& pCallsign);
		void				clear();
		void				sendTChat(const QString& pMessage);
		int					getTxRate();
		void				setTxRate(int pTxRate);
		int					getAILimit();
		void				setAILimit(int pAILimit);
		int					getAIRadius();
		void				setAIRadius(int pAIRadius);
		bool				getShadowMode();
		void				setShadowMode(bool pFlag);
		QString				getTitle(const QString& pPeerName);
		void				ResetAI(const QString& pPeerName);
        bool                getBlockedData(const QString &pPeerName);
        void                setBlockedData(const QString &pPeerName, bool pFlag);

	signals:
		void				firePeerAdd(QString);
		void				firePeerRemove(QString);
		void				firePeerChange(QString,int);
		void				fireSendTChat(QString);
		void				fireReceiveTChat(QString,QString);
	private slots:
		void				onCheckAI();
		void				onReceiveTChat(QString,QString);
	};
}
#endif // P2PMANAGER_H
