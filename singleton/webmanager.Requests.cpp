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

namespace ffs2play
{
	///
	/// \brief CWebManager::Connect
	///

	void CWebManager::Connect()
	{
		if (m_bConnected) return;
		if (m_currentUser == 0) return;
		QDomDocument SendHello;
		XmlBuild(SendHello, hello);
		CHttpRequest* Request = new CHttpRequest(this);
		connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( HelloCallback (CHttpRequest*)));
		Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendHello);
	}

	///
	/// \brief CWebManager::Disconnect
	///

	void CWebManager::Disconnect()
	{
		if (!m_bConnected) return;
		m_ATCTimer.stop();
		OnLiveUpdate();
		QDomDocument SendLogout;
		XmlBuild(SendLogout, logout);
		CHttpRequest* Request = new CHttpRequest(this);
		connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( LogoutCallback(CHttpRequest*)));
		Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendLogout);
		m_P2P->clear();
		m_bConnected = false;
		m_ATCTimer.stop();
		emit (FireDisconnected());
	}

	///
	/// \brief CWebManager::SendVerify
	///

	void CWebManager::SendVerify()
	{
		QDomDocument SendVerify;
		XmlBuild(SendVerify, verify);
		CHttpRequest* Request = new CHttpRequest(this);
		connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( ConnectCallback (CHttpRequest*)));
		Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendVerify);
	}

	///
	/// \brief CWebManager::SendLiveUpdate
	///

	void CWebManager::SendLiveUpdate()
	{
		QDomDocument SendLiveUpdate;
		XmlBuild(SendLiveUpdate, liveupdate);
		CHttpRequest* Request = new CHttpRequest(this);
		connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( LiveUpdateCallback (CHttpRequest*)));
		Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendLiveUpdate);
	}

	void CWebManager::SendATCUpdate()
	{
		QDomDocument SendAtcUpdate;
		XmlBuild(SendAtcUpdate, atc);
		CHttpRequest* Request = new CHttpRequest(this);
		connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( ATCCallback (CHttpRequest*)));
		Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendAtcUpdate);
	}
}
