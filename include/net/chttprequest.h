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

#ifndef CHTTPREQUEST_H
#define CHTTPREQUEST_H

#include <QObject>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

#include "singleton/clogger.h"

class CHttpRequest : public QObject
{
    Q_OBJECT
public:
    explicit CHttpRequest(QObject *parent = 0);
    ~CHttpRequest();
    void execute(QString URL, QDomDocument& Doc);
    void execute(QString URL);
    QString getAsString();
private:
    QString Reponse;
    CLogger* Log;
    QNetworkRequest m_request;
    QNetworkReply* m_reply;
    static QNetworkAccessManager* m_ptNetworkManager;
private slots:
    void callback();
signals:
    void finished(CHttpRequest*);
};

#endif // CHTTPREQUEST_H
