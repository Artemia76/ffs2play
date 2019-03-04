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

/****************************************************************************
 * webmanager.Requests.cpp is part of FF2Play project
 *
 * This singleton provide Web Manager for web server communication
 * **************************************************************************/

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
        XmlBuild(SendHello, XmlCode::hello);
        CHttpRequest* Request = new CHttpRequest(this);
        m_log->log(tr("Connecting to server ..."));
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
        XmlBuild(SendLogout, XmlCode::logout);
        CHttpRequest* Request = new CHttpRequest(this);
        m_log->log(tr("Disconnecting from server ..."));
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
        XmlBuild(SendVerify, XmlCode::verify);
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
        XmlBuild(SendLiveUpdate, XmlCode::liveupdate);
        CHttpRequest* Request = new CHttpRequest(this);
        connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( LiveUpdateCallback (CHttpRequest*)));
        Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendLiveUpdate);
    }

    ///
    /// \brief CWebManager::SendATCUpdate
    ///
    void CWebManager::SendATCUpdate()
    {
        QDomDocument SendAtcUpdate;
        XmlBuild(SendAtcUpdate, XmlCode::atc);
        CHttpRequest* Request = new CHttpRequest(this);
        connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( ATCCallback (CHttpRequest*)));
        Request->execute(m_users[m_currentUser-1].URL+"/action.php",SendAtcUpdate);
    }

    ///
    /// \brief CWebManager::CheckNewVersion
    ///
    void CWebManager::CheckNewVersion()
    {
        // Get the default profil selected in configuration file
        QString CheckVersionURL = "http://download.ffsimulateur2.fr/download.php?soft=ffs2play";
        if (m_settings.value("Main/Beta",false).toBool())
        {
            CheckVersionURL +="&beta=1";
        }
#ifdef Q_OS_MAC
        CheckVersionURL +="&os=mac";
#elif defined Q_OS_LINUX
        CheckVersionURL +="&os=lin";
#elif defined Q_OS_WIN32
        CheckVersionURL +="&os=win";
#endif
        CHttpRequest* Request =new CHttpRequest(this);
        connect(Request,SIGNAL(finished(CHttpRequest*)),this, SLOT( CheckNewVersionCallback (CHttpRequest*)));
        Request->execute(CheckVersionURL);
    }

    ///
    /// \brief CWebManager::CheckNewVersion
    ///
    void CWebManager::CheckXPPluginVersion()
    {
        QString CheckVersionURL = "http://download.ffsimulateur2.fr/download.php?soft=xffs2play&beta=1";
        CHttpRequest* Request =new CHttpRequest(this);
        connect(Request,SIGNAL(finished(CHttpRequest*)),this, SLOT( CheckXPPluginVersionCallback (CHttpRequest*)));
        Request->execute(CheckVersionURL);
    }
}
