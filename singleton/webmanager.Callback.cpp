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

#include "singleton/webmanager.h"
#include "singleton/singleton.h"

#include <QCryptographicHash>

namespace ffs2play
{

	///
	/// \brief CWebManager::HelloCallback
	/// \param Request
	///

	void CWebManager::HelloCallback (CHttpRequest* Request)
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get HelloCallback"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		QDomDocument Doc;
		if (Request->getAsXml(Doc))
		{
			QDomNode NodeCert = Doc.elementsByTagName("certificat").at(0);
			QString SCert = NodeCert.firstChild().nodeValue();
			m_sCrypted_AESKey = m_crypto.RSA_EncryptToBase64(SCert, m_crypto.EncryptionKey());
	#ifdef FFS_DEBUG
			m_log->log("WebManager: AES Key encrypted with RSA = " + m_sCrypted_AESKey,Qt::darkBlue,LEVEL_NORMAL);
	#endif
			emit (FireHelloCallback());
		}
		Request->deleteLater();
	}

	///
	/// \brief CWebManager::ConnectCallback
	/// \param Request
	///

	void CWebManager::ConnectCallback(CHttpRequest *pRequest)
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get ConnectCallback"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		QDomDocument Doc;
		if (pRequest->getAsXml(Doc))
		{
			bool bError = CheckError(Doc);
			QDomElement Element = Doc.elementsByTagName("loginStatus").at(0).toElement();
			if (!Element.isNull())
			{
				m_bConnected = Element.firstChild().nodeValue().toInt()==1;
				if (m_bConnected && (!bError))
				{
					m_log->log(tr("FFS2Play is connected on server"));
					QDomElement xmlExternalIP = Doc.elementsByTagName("your_ip").at(0).toElement();
					if (!xmlExternalIP.isNull())
					{
						QHostAddress ExternalIP (xmlExternalIP.firstChild().nodeValue());
	#ifdef FFS_DEBUG
						if (ExternalIP.isNull())
						{
							m_log->log("WebManager : " + tr("Get invalid external IP"),Qt::darkMagenta,LEVEL_VERBOSE);
						}
						else
						{
							m_log->log("WebManager : " + tr("Get external IP = ") + ExternalIP.toString(),Qt::darkMagenta,LEVEL_VERBOSE);
						}
	#endif
					}
					QDomElement xmlKey = Doc.elementsByTagName("key").at(0).toElement();
					if (!xmlKey.isNull())
					{
						m_key = xmlKey.firstChild().nodeValue();
						OnLiveUpdate();
					}
					QDomElement xmlAtc = Doc.elementsByTagName("atc").at(0).toElement();
					if (!xmlAtc.isNull())
					{
						if (xmlAtc.firstChild().nodeValue().toInt()==1)
						{
							m_ATCTimer.start();
						}
					}
					QDomElement AIManagement = Doc.elementsByTagName("AI_Management").at(0).toElement();
					if (!AIManagement.isNull())
					{
						m_AIManagement = (AIManagement.firstChild().nodeValue().toInt()==1);
					}
					m_P2P->setCallsign(m_users[m_currentUser-1].Login);
					CheckWhazzup(Doc);
					emit (FireConnected());
				}
			}

		}
		pRequest->deleteLater();
	}

	void CWebManager::LiveUpdateCallback(CHttpRequest* pRequest)
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get LiveUpdateCallback"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		if (m_bConnected)
		{
			QDomDocument Doc;
			if (pRequest->getAsXml(Doc))
			{
				CheckWhazzup(Doc);
			}
		}
		pRequest->deleteLater();
	}

	///
	/// \brief CWebManager::LogoutCallback
	/// \param Request
	///

	void CWebManager::LogoutCallback(CHttpRequest* 	pRequest)
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get LogoutCallback"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		pRequest->deleteLater();
	}

	///
	/// \brief CWebManager::OnATCUpdate
	///

	void CWebManager::OnATCUpdate()
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get ATCUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
	}

	///
	/// \brief CWebManager::OnLiveUpdate
	///

	void CWebManager::OnLiveUpdate()
	{
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("Get OnLiveUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
	#endif
		if (m_bConnected && m_simManager->isConnected())
		{
			SendLiveUpdate();
		}
	}
}
