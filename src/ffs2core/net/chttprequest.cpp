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

#include "net/chttprequest.h"

QNetworkAccessManager* CHttpRequest::m_ptNetworkManager = NULL;
int CHttpRequest::m_nbInstance =0;
///
/// \brief CHttpRequest::CHttpRequest
/// \param parent
///

CHttpRequest::CHttpRequest(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<CHttpRequest*>("CHttpRequest*");
    if (m_ptNetworkManager==NULL) m_ptNetworkManager=new QNetworkAccessManager(parent);
    //m_nbInstance++;
    m_request.setRawHeader("User-Agent","FFS2Play");
    Log = CLogger::instance();
    m_reply=NULL;
}

///
/// \brief CHttpRequest::~CHttpRequest
///

CHttpRequest::~CHttpRequest()
{

}

///
/// \brief CHttpRequest::execute
/// \param URL
/// \param Doc
///

void CHttpRequest::execute(QString URL, QDomDocument& Doc)
{
    m_request.setUrl(QUrl(URL));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; encoding='utf-8'");
    QByteArray content = Doc.toByteArray();
    m_reply = m_ptNetworkManager->post(m_request,content);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
#ifdef QT_DEBUG
    Log->log("HttpRequest : URL = " + URL,Qt::darkGreen,LEVEL_VERBOSE );
    Log->log("HttpRequest Post = " + QString(content),Qt::darkGreen,LEVEL_VERBOSE );
#endif
}

///
/// \brief CHttpRequest::execute
/// \param URL
///

void CHttpRequest::execute(QString URL)
{
    m_request.setUrl(QUrl(URL));
    m_reply = m_ptNetworkManager->get(m_request);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
#ifdef QT_DEBUG
    Log->log("HttpRequest Get URL = " + URL,Qt::darkGreen,LEVEL_VERBOSE );
#endif
}

///
/// \brief CHttpRequest::callback
///

void CHttpRequest::callback()
{
    if (m_reply!= NULL) Reponse = (QString) m_reply->readAll();
#ifdef QT_DEBUG
    Log->log("HttpRequest Response = " + Reponse,Qt::darkRed,LEVEL_VERBOSE );
#endif
    emit(finished(this));
}

///
/// \brief CHttpRequest::getAsString
/// \return
///

QString CHttpRequest::getAsString()
{
    return Reponse;
}

bool CHttpRequest::getAsXml(QDomDocument& Doc)
{
    if (!Reponse.isEmpty())
    {
        QString ErrorMessage;
        Doc.setContent(Reponse,&ErrorMessage);
        if (!ErrorMessage.isEmpty())
        {
            Log->log("HttpRequest error : " + ErrorMessage);
            return false;
        }
        return true;
    }
    return false;
}
