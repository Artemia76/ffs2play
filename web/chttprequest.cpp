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
** chttprequest.cpp is part of FF2Play project
**
** This class is used to send web request in POST or GET mode
** It also provide a programmed callback is case of response needs
****************************************************************************/

#include <QMetaType>
#include "web/chttprequest.h"

///
/// \brief CHttpRequest::m_ptNetworkManager
///
/// Static Pointer to acces Qt Network Manager Layer
///
QNetworkAccessManager* CHttpRequest::m_ptNetworkManager = nullptr;

///
/// \brief CHttpRequest::m_nbInstance
///
int CHttpRequest::m_nbInstance =0;

///
/// \brief CHttpRequest::CHttpRequest
/// \param parent
///
/// Default Constructor
///
CHttpRequest::CHttpRequest(QObject *pParent) : QObject(pParent)
{
    qRegisterMetaType<CHttpRequest*>("CHttpRequest*");
    if (m_ptNetworkManager==nullptr) m_ptNetworkManager=new QNetworkAccessManager(pParent);
    //m_nbInstance++;
    m_request.setRawHeader("User-Agent","FFS2Play");
    m_reply=nullptr;
}

///
/// \brief CHttpRequest::~CHttpRequest
///
/// Default Destructor
///
CHttpRequest::~CHttpRequest()
{
}

///
/// \brief CHttpRequest::execute
/// \param URL
/// \param Doc
///
/// Exexute POST HTTP request
///
void CHttpRequest::execute(const QString& pURL, QDomDocument& pDoc)
{
    m_request.setUrl(QUrl(pURL));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; encoding='utf-8'");
    QByteArray content = pDoc.toByteArray();
    m_reply = m_ptNetworkManager->post(m_request,content);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
#ifdef FFS_DEBUG
    m_log.log("HttpRequest : URL = " + pURL,Qt::darkGreen,LEVEL_VERBOSE );
    m_log.log("HttpRequest : Post = " + QString(content),Qt::darkGreen,LEVEL_VERBOSE );
#endif
}

///
/// \brief CHttpRequest::execute
/// \param URL
///
void CHttpRequest::execute(const QString& URL)
{
    m_request.setUrl(QUrl(URL));
    m_reply = m_ptNetworkManager->get(m_request);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
#ifdef FFS_DEBUG
    m_log.log("HttpRequest : Get URL = " + URL,Qt::darkGreen,LEVEL_VERBOSE );
#endif
}

///
/// \brief CHttpRequest::callback
///
void CHttpRequest::callback()
{
    if (m_reply!= nullptr) m_response = m_reply->readAll();
#ifdef FFS_DEBUG
    m_log.log("HttpRequest : Response = " + m_response,Qt::darkRed,LEVEL_VERBOSE );
#endif
    emit finished(this);
}

///
/// \brief CHttpRequest::finalUrl
/// \return URL that answered, after redirections
///
QUrl CHttpRequest::finalUrl() const
{
    return (m_reply != nullptr) ? m_reply->url() : m_request.url();
}

///
/// \brief CHttpRequest::getAsString
/// \return
///
QString CHttpRequest::getAsString()
{
    return m_response;
}

///
/// \brief CHttpRequest::getAsXml
/// \param pDoc
/// \return
///
bool CHttpRequest::getAsXml(QDomDocument& pDoc)
{
    if (!m_response.isEmpty())
    {
        const QDomDocument::ParseResult result = pDoc.setContent(m_response);
        if (!result)
        {
            m_log.log("HttpRequest : error = " + result.errorMessage);
            return false;
        }
        return true;
    }
    return false;
}
