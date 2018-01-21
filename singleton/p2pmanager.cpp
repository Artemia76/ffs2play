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

#include "singleton/p2pmanager.h"
#include <QNetworkDatagram>
#include "singleton/singleton.h"
#include <cfloat>
#ifdef Q_OS_WIN
#include "WinSock2.h"
#endif

#include <sstream>

namespace ffs2play {
	///
	/// \brief CP2PManager::CP2PManager
	/// \param parent
	///

	CP2PManager::CP2PManager(QObject *parent) : QObject(parent)
	{
		m_log=CLogger::instance();
		m_server = new UDPServer(this);
		connect (&m_checkAITimer,SIGNAL(timeout()),this,SLOT(onCheckAI()));
		m_checkAITimer.start(5000);
	}

	///
	/// \brief CP2PManager::createInstance
	/// \return
	///

	CP2PManager* CP2PManager::createInstance()
	{
		return new CP2PManager();
	}

	///
	/// \brief CP2PManager::~CP2PManager
	///

	CP2PManager::~CP2PManager()
	{
		init(false);
	}

	///
	/// \brief CP2PManager::instance
	/// \return
	///

	CP2PManager* CP2PManager::instance()
	{
		return Singleton<CP2PManager>::instance(CP2PManager::createInstance);
	}

	void CP2PManager::onCheckAI()
	{
		int NbInvisible = 0;
		int NbVisible = 0;
		int PosLointain = 0;
		double ScoreLointain = -1;
		int PosPret = 0;
		double ScorePret=DBL_MAX;
		for (int i=0; i<m_peers.count(); i++)
		{
			if (m_peers[i]->m_spawnable)
			{
				if (m_peers[i]->m_visible)
				{
					NbVisible++;
					if (m_peers[i]->m_distance > ScoreLointain)
					{
						ScoreLointain = m_peers[i]->m_distance;
						PosLointain = i;
					}
				}
				else
				{
					NbInvisible++;
					if (m_peers[i]->m_distance < ScorePret)
					{
						ScorePret = m_peers[i]->m_distance;
						PosPret = i;
					}
				}
			}
		}
		// Si on en a plus de visible que la limite alors on en choisi un pour le rendre invisible (les plus lointains)
		if (NbVisible > m_AILimite)
		{
			//On rend le plus lointain invisible
			m_peers[PosLointain]->m_visible = false;
#ifdef FFS_DEBUG
			m_log->log("P2PManager : Peer " + m_peers[PosLointain]->m_callSign + tr(" become unvisible"),Qt::darkBlue);
#endif
		}
		//Sinon , si on a des invisibles et que le nombre de visibles est inférieur à la limite
		//cela signifie qu'on a des invisibles qu'on peut afficher (les plus prêt)
		else if ((NbInvisible > 0) && (NbVisible < m_AILimite))
		{
			m_peers[PosPret]->m_visible = true;
#ifdef FFS_DEBUG
			m_log->log("P2PManager : Peer " + m_peers[PosPret]->m_callSign + tr(" become visible"),Qt::darkBlue);
#endif
		}
	}

	///
	/// \brief CP2PManager::init
	/// \param flag
	///

	void CP2PManager::init(bool flag)
	{
		if (flag)
		{
			m_settings.beginGroup("P2P");
			m_port = m_settings.value("Port",54857).toInt();
			m_bUPNP = false;
			m_server->setPort(m_port);
			m_server->start();
			if (m_settings.value("UPNPEnable", true).toBool())
			{
				initUPNP(true);
			}
			m_Shadow = m_settings.value("ShadowEnable", false).toBool();
			m_AILimite = m_settings.value("AILimite",10).toInt();
			m_settings.endGroup();
		}
		else
		{
			m_server->stop();
			initUPNP(false);
		}
	}

	///
	/// \brief CP2PManager::initUPNP
	/// \param flag
	///

	void CP2PManager::initUPNP(bool flag)
	{
		if (flag)
		{
			if (m_bUPNP) return;
	#ifdef Q_OS_WIN
			WSADATA wsaData;
			int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
			if(nResult != NO_ERROR) {
				m_log->log("P2PManager : "+ tr("UPNP Windows Socket failed."),Qt::red);
				return;
			}
	#endif
			UPNPDev* devlist;
			char lanaddr[64]; // IP address on the LAN
			char wanaddr[64]; // IP address on the Internet
			const char* multicastif = 0;
			const char* minissdpdpath = 0;
			int error;
			devlist = upnpDiscover(
						1000,
						multicastif,
						minissdpdpath,
						0,
						0,
	#if MINIUPNPC_API_VERSION >= 14
						4,
	#endif
						&error);
			if (devlist != NULL)
			{
#ifdef FFS_DEBUG
				for (UPNPDev* device = devlist; device !=NULL; device = device->pNext)
				{
					m_log->log("P2PManager : " + tr("IGP Desc = ") + QString(device->descURL) + tr(" st: ") + QString(device->st),Qt::darkBlue,LEVEL_NORMAL);
				}
#endif
				if ((error = UPNP_GetValidIGD(devlist, &m_urls, &m_data, lanaddr, 64)) != 0)
				{
#ifdef FFS_DEBUG
					switch (error)
					{
						case 1:
						{
							m_log->log("P2PManager : " + tr("Found valid IGD : ") + QString(m_urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
							break;
						}
						case 2:
						{
							m_log->log("P2PManager : " + tr("Found a (not connected?) IGD : ") + QString(m_urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
							break;
						}
						case 3:
						{
							m_log->log("P2PManager : " + tr("UPnP device found. Is it an IGD ? : ") + QString(m_urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
							break;
						}
						default:
						{
							m_log->log("P2PManager : " + tr("Found device (IGD ?) : ") + QString(m_urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
							break;
						}
					}
#endif
					m_InternalIP.add_ip(lanaddr);
					UPNP_GetExternalIPAddress(m_urls.controlURL,m_data.first.servicetype,wanaddr);
					m_ExternalIP = QString(wanaddr);
#ifdef FFS_DEBUG
					m_log->log("P2PManager : " + tr("Local LAN IP address : "),Qt::darkBlue,LEVEL_NORMAL);
					for (int i=0 ; i< m_InternalIP.ip_size(); ++i)
					{
						m_log->log("P2PManager : " + tr("IP address : ") + QString(m_InternalIP.ip(i).c_str()),Qt::darkBlue,LEVEL_NORMAL);
					}
					m_log->log("P2PManager : " + tr("Internet IP address : ") + QString(m_ExternalIP),Qt::darkBlue,LEVEL_NORMAL);
#endif
					error = UPNP_AddPortMapping(
							m_urls.controlURL,
							m_data.first.servicetype,
							QString::number(m_port).toLatin1(),
							QString::number(m_port).toLatin1(),
							lanaddr,
							"FFS2Play",
							"UDP",
							NULL,
							"0"
					);
					if (error != UPNPCOMMAND_SUCCESS)
						m_log->log("P2PManager : " + tr("Adding port mappping failed with code ") + QString::number(error),Qt::red);
					else m_bUPNP=true;
				}
				else
				{
					m_log->log("P2PManager : " + tr("No valid UPNP Internet Gateway Device found."),Qt::red);
				}
				freeUPNPDevlist(devlist);
			}
			else
			{
				m_log->log("P2PManager : " + tr("No IGD UPNP Device found on the network !"),Qt::red);
			}

		}
		else
		{
			if (!m_bUPNP) return;
			int error = UPNP_DeletePortMapping(
						m_urls.controlURL,
						m_data.first.servicetype,
						QString::number(m_port).toLatin1(),
						"UDP",
						NULL
			);
			if (error != UPNPCOMMAND_SUCCESS)
				m_log->log("P2PManager : " + tr("Delete port mapping failed with code ") + QString::number(error),Qt::red);
			m_bUPNP = false;
		}
	}

	int CP2PManager::getPort()
	{
		return m_port;
	}

	QString CP2PManager::getLocalIPs()
	{
		std::ostringstream out;
		m_InternalIP.SerializeToOstream(&out);
		QByteArray OutPut(out.str().c_str());
		return QString(OutPut.toBase64());
	}

	void CP2PManager::setCallsign(const QString &pCallSign)
	{
		m_CallSign = pCallSign;
	}

	void CP2PManager::whazzup_Update(QVector<PeerData>& pWhazzup)
	{
		QString UserName = m_CallSign.replace('[','_');
		UserName = UserName.replace(']','_');
		UserName = UserName.toLower();
		for (PeerData data : pWhazzup)
		{
			bool Shadow = false;
			bool Disabled = isDisabled(data.CallSign);
			// Shadow Detection
			if (UserName == data.CallSign.toLower())
			{
#ifdef FFS_DEBUG
				if (m_Shadow) Shadow = true;
				else
#endif
					continue;
			}
			QVector<CPeer*>::iterator it;
			QHostAddress IP;
			//Find any existing peer in list
			for (it = m_peers.begin();it != m_peers.end(); ++it )
			{
				//If we found existing peer in list
				if ((*it)->CallSign == data.CallSign)
				{
					break;
				}
			}
			// if iteraotr reach end of vector, that mean peer does not yet exist
			// so we create it...
			if (it==m_peers.end())
			{
				if (Shadow) IP = QHostAddress("127.0.0.1");
				else
				{
					IP = QHostAddress(data.externalIP);
					if (IP.isNull())
					{
						m_log->log("P2PManager : " + tr("Bad IP Address"),Qt::darkMagenta);
					}
				}
				if ((!IP.isNull()) && (data.Port>0))
				{
					m_peers.push_back(new CPeer(m_server, IP,data.Port,data.CallSign,Disabled,!data.externalIP.isEqual(IP),&data.localIPs));
					emit(firePeerAdd(data.CallSign));
#ifdef FFS_DEBUG
					m_log->log("P2PManager : " + tr("Adding P2P user callsign = ")+ data.CallSign + ", IP=" + IP.toString() + ", Port=" + QString::number(data.Port),Qt::darkBlue);
#endif
				}
				else
				{
					m_log->log("P2PManager : " + tr("User ") + data.CallSign + " discarded", Qt::darkMagenta);
				}
			}
			// if user already exist, we update it
			else
			{
				CPeer* Peer = (*it);
				if (Shadow) IP = QHostAddress("127.0.0.1");
				else
				{
					IP = QHostAddress(data.externalIP);
					if (IP.isNull())
					{
						m_log->log("P2PManager : " + tr("Bad IP Address"),Qt::darkMagenta);
					}
				}
				if (!Peer->m_externalIP.isEqual(IP))
				{
#ifdef FFS_DEBUG
					m_log->log("P2PManager : " + tr("Change IP for user ") + Peer->CallSign + ", IP=" + Peer->m_externalIP.toString(),Qt::blue, LEVEL_NORMAL);
#endif
				}
				if (Peer->m_port != data.Port)
				{
#ifdef FFS_DEBUG
					m_log->log("P2PManager : " + tr("Change port for user ") + Peer->CallSign + ", Port=" + QString::number(Peer->m_port) ,Qt::blue, LEVEL_NORMAL);
#endif
				}
			}
		}
		//Make invert research to purge disconnected peers
		for (int i = 0; i < m_peers.count() ; i++)
		{
			bool find=false;
			for (PeerData data : pWhazzup)
			{
				if (data.CallSign==m_peers[i]->CallSign)
				{
					if ((!m_Shadow) && (data.CallSign == m_CallSign))
					{
						find = false;
					}
					else find = true;
				}
			}
			if (!find)
			{
#ifdef FFS_DEBUG
				m_log->log("P2PManager : " + tr("Remove user ") + m_peers[i]->CallSign, Qt::blue, LEVEL_NORMAL);
#endif
				emit(firePeerRemove(m_peers[i]->CallSign));
				m_peers.removeAt(i);

			}
		}
	}

	bool CP2PManager::isDisabled(const QString &pCallsign)
	{
		m_settings.beginGroup("P2P");
		QStringList List = m_settings.value("BlackList").value<QStringList>();
		m_settings.endGroup();
		return List.contains(pCallsign);
	}

	void CP2PManager::clear ()
	{
		for (QVector<CPeer*>::iterator it = m_peers.begin(); it != m_peers.end() ; ++it)
		{
			QString CallSign = (*it)->CallSign;
#ifdef FFS_DEBUG
			m_log->log("P2PManager : " + tr("Remove user ") + CallSign, Qt::blue, LEVEL_NORMAL);
#endif
			delete (*it);
			emit(firePeerRemove(CallSign));
		}
		m_peers.clear();
	}
}
