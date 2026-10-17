/****************************************************************************
**
** Copyright (C) 2026 FSFranceSimulateur team.
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
** caddrrequest.h is part of FF2Play project
**
** POST of an XML request to the portal over a forced IP layer (IPv4 or IPv6) :
** QNetworkAccessManager lets the system choose the layer, so this class uses
** a plain HTTP/1.0 exchange on a QTcpSocket / QSslSocket instead
****************************************************************************/

#ifndef CADDRREQUEST_H
#define CADDRREQUEST_H

#include <QObject>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTimer>
#include <QUrl>

class QTcpSocket;

class CAddrRequest : public QObject
{
    Q_OBJECT
public:
    explicit                    CAddrRequest(QObject* pParent = nullptr);
    void                        execute(const QUrl& pURL, const QByteArray& pContent,
                                        QAbstractSocket::NetworkLayerProtocol pProtocol);
    QAbstractSocket::NetworkLayerProtocol protocol() const { return m_protocol; }
    QHostAddress                yourIP() const { return m_yourIP; }
    QString                     errorString() const { return m_error; }

signals:
    void                        finished(CAddrRequest*);

private:
    QTcpSocket*                 m_socket;
    QTimer                      m_timeout;
    QByteArray                  m_request;
    QByteArray                  m_response;
    QAbstractSocket::NetworkLayerProtocol m_protocol;
    QHostAddress                m_yourIP;
    QString                     m_error;
    bool                        m_done;
    void                        sendRequest();
    void                        finish(const QString& pError = QString());
    void                        parseResponse();
};

#endif // CADDRREQUEST_H
