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
 * webmanager.Callback.cpp is part of FF2Play project
 *
 * This singleton provide Web Manager for web server communication
 * **************************************************************************/

#include "singleton/webmanager.h"
#include "singleton/singleton.h"
#include "tools/getappversion.h"

#include <QCryptographicHash>
#include <QMessageBox>
#include <QDesktopServices>

namespace ffs2play
{

    ///
    /// \brief CWebManager::HelloCallback
    /// \param Request
    ///
    void CWebManager::HelloCallback (CHttpRequest* Request)
    {
    #ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get HelloCallback"),Qt::darkBlue,LEVEL_VERBOSE);
    #endif
        QDomDocument Doc;
        if (Request->getAsXml(Doc))
        {
            QDomNode NodeCert = Doc.elementsByTagName("certificat").at(0);
            QString SCert = NodeCert.firstChild().nodeValue();
            m_sCrypted_AESKey = m_crypto.RSA_EncryptToBase64(SCert, m_crypto.EncryptionKey());
    #ifdef FFS_DEBUG
            m_log->log("WEBManager: AES Key encrypted with RSA = " + m_sCrypted_AESKey,Qt::darkBlue,LEVEL_NORMAL);
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
        m_log->log("WEBManager : " + tr("Get ConnectCallback"),Qt::darkBlue,LEVEL_VERBOSE);
    #endif
        QDomDocument Doc;
        try
        {
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
                                m_log->log("WEBManager : " + tr("Get invalid external IP"),Qt::darkMagenta,LEVEL_VERBOSE);
                            }
                            else
                            {
                                m_log->log("WEBManager : " + tr("Get external IP = ") + ExternalIP.toString(),Qt::darkMagenta,LEVEL_VERBOSE);
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
                        m_ATCTimer.start();
                        CheckWhazzup(Doc);
                        emit (FireConnected());
                    }
                }
                else
                {
                    throw(std::logic_error("Invalid Xml Response"));
                }
            }
            else
            {
                throw(std::logic_error("TimeOut"));
            }
        }
        catch (std::exception& /*e*/)
        {
            m_log->log(tr("Connexion to server failed."),Qt::darkMagenta);
        }
        pRequest->deleteLater();
    }

    ///
    /// \brief CWebManager::LiveUpdateCallback
    /// \param pRequest
    ///
    void CWebManager::LiveUpdateCallback(CHttpRequest* pRequest)
    {
    #ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get LiveUpdateCallback"),Qt::darkBlue,LEVEL_VERBOSE);
    #endif
        if (m_bConnected)
        {
            QDomDocument Doc;
            if (pRequest->getAsXml(Doc))
            {
                if (CheckError(Doc))
                {
                    Disconnect();
                    return;
                }
                else
                {
                    CheckWhazzup(Doc);
                    QDomElement Element = Doc.elementsByTagName("new_key").at(0).toElement();
                    if (!Element.isNull())
                    {
                        m_key = Element.text();
                    }
                }
            }
        }
        pRequest->deleteLater();
    }

    ///
    /// \brief CWebManager::ATCCallback
    /// \param pRequest
    ///
    void CWebManager::ATCCallback(CHttpRequest *pRequest)
    {
#ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get ATCCallback"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
        pRequest->deleteLater();
    }

    ///
    /// \brief CWebManager::LogoutCallback
    /// \param Request
    ///
    void CWebManager::LogoutCallback(CHttpRequest* 	pRequest)
    {
    #ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get LogoutCallback"),Qt::darkBlue,LEVEL_VERBOSE);
    #endif
        pRequest->deleteLater();
    }

    ///
    /// \brief CWebManager::OnATCUpdate
    ///
    void CWebManager::OnATCUpdate()
    {
    #ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get ATCUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
    #endif
        if (isConnected() && m_simManager->isConnected())
        {
            int Transponder = m_simManager->getAircraftState().Squawk;
            int TransponderMode = m_simManager->getAircraftState().SquawkMode;
            switch (m_simManager->getType())
            {
                case SIM_TYPE::FSX:
                {
                    if ((Transponder == 1200) || (Transponder == 7000) || (Transponder == 4700)) return;
                }
                case SIM_TYPE::XPLANE:
                {
                    if (TransponderMode<2) return;
                }
            }
            SendATCUpdate();
        }
    }

    ///
    /// \brief CWebManager::OnLiveUpdate
    ///
    void CWebManager::OnLiveUpdate()
    {
#ifdef FFS_DEBUG
        m_log->log("WEBManager : " + tr("Get OnLiveUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
        if (m_bConnected && m_simManager->isConnected())
        {
            SendLiveUpdate();
        }
    }

    ///
    /// \brief CWebManager::CheckNewVersionCallback
    /// \param pRequest
    ///
    void CWebManager::CheckNewVersionCallback(CHttpRequest* pRequest)
    {
        sVersion Actual = GetAppVersion();
        QDomDocument Doc;
        if (pRequest->getAsXml(Doc))
        {
            QDomElement xmlError = Doc.elementsByTagName("error").at(0).toElement();
            if (xmlError.isNull())
            {
                QDomElement xmlVersion = Doc.elementsByTagName("version").at(0).toElement();
                QDomElement xmlURL = Doc.elementsByTagName("url").at(0).toElement();
                if (!xmlVersion.isNull() && !xmlURL.isNull())
                {
                    int SiteMajor = xmlVersion.attribute("major").toInt();
                    int SiteMinor = xmlVersion.attribute("minor").toInt();
                    int SiteBuild = xmlVersion.attribute("build").toInt();
                    bool maj = false;
                    bool compat = true;
                    QString URL = xmlURL.text();
                    QString Message;
                    if (SiteMajor > Actual.Major)
                    {
                        Message = tr("This version of FFS2Play is legacy and does not suppported. Do you want update it ?");
                        maj = true;
                        compat = false;
                    }
                    else if ((SiteMinor > Actual.Minor)||((SiteMinor == Actual.Minor) && (SiteBuild> Actual.Build)))
                    {
                        Message = tr("This version of FFS2Play is not up to date. Do you want update it?");
                        maj =true;
                    }
                    if (maj)
                    {
                        QMessageBox::StandardButton reply;
                        reply = QMessageBox::question(nullptr, tr("FFS2Play Update"),Message,QMessageBox::Yes|QMessageBox::No);
                        if (reply == QMessageBox::Yes)
                        {
                            QDesktopServices::openUrl(URL);
                            compat=false;
                        }
                        if (!compat) emit(FireExit());
                    }
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log->log("WEBManager : " + tr("Error on Version checking : ") + xmlError.text(),Qt::darkBlue,LEVEL_NORMAL);
#endif
            }
        }
        pRequest->deleteLater();
    }

    ///
    /// \brief CWebManager::CheckNewVersionCallback
    /// \param pRequest
    ///
    void CWebManager::CheckXPPluginVersionCallback(CHttpRequest* pRequest)
    {
        sVersion Actual = m_simManager->getPlugVersion();
        QDomDocument Doc;
        if (pRequest->getAsXml(Doc))
        {
            QDomElement xmlVersion = Doc.elementsByTagName("version").at(0).toElement();
            QDomElement xmlURL = Doc.elementsByTagName("url").at(0).toElement();

            if (!xmlVersion.isNull() && !xmlURL.isNull())
            {
                int SiteMajor = xmlVersion.attribute("major").toInt();
                int SiteMinor = xmlVersion.attribute("minor").toInt();
                int SiteBuild = xmlVersion.attribute("build").toInt();
                QString URL = xmlURL.text();
                QString Message;
                if ((SiteMajor > Actual.Major)||
                        ((SiteMajor == Actual.Major) && (SiteMinor > Actual.Minor)) ||
                        ((SiteMajor == Actual.Major) && (SiteMinor == Actual.Minor) && (SiteBuild> Actual.Build)))
                {
                    Message = tr("The X-Plane XFFS2Play plugin is not up to date. Do you want update it?");
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(nullptr, tr("XFFS2Play Plugin Update"),Message,QMessageBox::Yes|QMessageBox::No);
                    if (reply == QMessageBox::Yes)
                    {
                        QDesktopServices::openUrl(URL);
                    }
                }
            }
        }
        pRequest->deleteLater();
    }
}
