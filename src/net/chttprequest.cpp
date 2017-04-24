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

CHttpRequest::CHttpRequest(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<CHttpRequest*>("CHttpRequest*");
    if (m_ptNetworkManager==NULL) m_ptNetworkManager = new QNetworkAccessManager();
    m_request.setRawHeader("User-Agent","FFS2Play");
    Log = CLogger::instance();
}

CHttpRequest::~CHttpRequest()
{
    if (m_reply) m_reply->deleteLater();
}

void CHttpRequest::execute(QString URL, QDomDocument& Doc)
{
    m_request.setUrl(QUrl(URL));
    Log->log("HttpRequest URL = " + URL,Qt::darkBlue,LEVEL_VERBOSE );
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; encoding='utf-8'");
    QByteArray content = Doc.toByteArray();
    Log->log("HttpRequest Post = " + QString(content),Qt::darkBlue,LEVEL_VERBOSE );
    m_reply = m_ptNetworkManager->post(m_request,content);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
}

void CHttpRequest::execute(QString URL)
{
    m_request.setUrl(QUrl(URL));
    m_reply = m_ptNetworkManager->get(m_request);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
}

void CHttpRequest::callback()
{
    if (m_reply!= NULL) Reponse = (QString) m_reply->readAll();
    emit(finished(this));
}

QString CHttpRequest::getAsString()
{
    return Reponse;
}
