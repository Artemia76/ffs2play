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
** caddrrequest.cpp is part of FF2Play project
**
** POST of an XML request to the portal over a forced IP layer (IPv4 or IPv6),
** used by the addr action so that the portal records the address of each layer
****************************************************************************/

#include "web/caddrrequest.h"

#include <QDomDocument>
#include <QSslSocket>
#include <QTcpSocket>

namespace
{
    constexpr int TIMEOUT_MS = 5000;
    constexpr qsizetype MAX_RESPONSE = 256 * 1024;
}

///
/// \brief CAddrRequest::CAddrRequest
/// \param pParent
///
CAddrRequest::CAddrRequest(QObject* pParent) : QObject(pParent)
{
    m_socket = nullptr;
    m_protocol = QAbstractSocket::IPv4Protocol;
    m_done = false;
    m_timeout.setSingleShot(true);
    connect(&m_timeout, &QTimer::timeout, this, [this]() { finish(QStringLiteral("timeout")); });
}

///
/// \brief CAddrRequest::execute
/// \param pURL action.php URL (http or https)
/// \param pContent XML request
/// \param pProtocol IPv4Protocol or IPv6Protocol
///
void CAddrRequest::execute(const QUrl& pURL, const QByteArray& pContent,
                           QAbstractSocket::NetworkLayerProtocol pProtocol)
{
    m_protocol = pProtocol;
    const bool secure = pURL.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) == 0;
    const QString host = pURL.host();
    const quint16 port = static_cast<quint16>(pURL.port(secure ? 443 : 80));
    QString path = pURL.path(QUrl::FullyEncoded);
    if (path.isEmpty()) path = QStringLiteral("/");

    // HTTP/1.0 : no chunked response, the server closes the connection at the end
    m_request = "POST " + path.toLatin1() + " HTTP/1.0\r\n"
              + "Host: " + pURL.host(QUrl::FullyEncoded).toLatin1()
              + (pURL.port() > 0 ? ":" + QByteArray::number(port) : QByteArray()) + "\r\n"
              + "User-Agent: FFS2Play\r\n"
              + "Content-Type: text/xml; encoding='utf-8'\r\n"
              + "Content-Length: " + QByteArray::number(pContent.size()) + "\r\n"
              + "Connection: close\r\n\r\n"
              + pContent;

    m_timeout.start(TIMEOUT_MS);
    if (secure)
    {
        QSslSocket* ssl = new QSslSocket(this);
        m_socket = ssl;
        connect(ssl, &QSslSocket::encrypted, this, &CAddrRequest::sendRequest);
        connect(ssl, &QSslSocket::sslErrors, this, [this](const QList<QSslError>& pErrors)
        {
            finish(pErrors.isEmpty() ? QStringLiteral("TLS error") : pErrors.first().errorString());
        });
        ssl->connectToHostEncrypted(host, port, QIODevice::ReadWrite, pProtocol);
    }
    else
    {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::connected, this, &CAddrRequest::sendRequest);
        m_socket->connectToHost(host, port, QIODevice::ReadWrite, pProtocol);
    }
    connect(m_socket, &QTcpSocket::readyRead, this, [this]()
    {
        m_response += m_socket->readAll();
        if (m_response.size() > MAX_RESPONSE) finish(QStringLiteral("response too large"));
    });
    connect(m_socket, &QTcpSocket::disconnected, this, &CAddrRequest::parseResponse);
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError pError)
    {
        // the server closing the connection after the response is the normal end
        if (pError == QAbstractSocket::RemoteHostClosedError) parseResponse();
        else finish(m_socket->errorString());
    });
}

///
/// \brief CAddrRequest::sendRequest
///
void CAddrRequest::sendRequest()
{
    m_socket->write(m_request);
}

///
/// \brief CAddrRequest::parseResponse
/// Read your_ip in the XML body of a 200 response
///
void CAddrRequest::parseResponse()
{
    if (m_done) return;
    m_response += m_socket->readAll();
    const qsizetype bodyStart = m_response.indexOf("\r\n\r\n");
    if (bodyStart < 0)
    {
        finish(QStringLiteral("no HTTP response"));
        return;
    }
    const QByteArray statusLine = m_response.left(m_response.indexOf("\r\n"));
    if (statusLine.split(' ').value(1) != "200")
    {
        finish(QString::fromLatin1(statusLine));
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(m_response.mid(bodyStart + 4)))
    {
        finish(QStringLiteral("invalid XML response"));
        return;
    }
    m_yourIP = QHostAddress(doc.elementsByTagName("your_ip").at(0).toElement().text().trimmed());
    finish(m_yourIP.isNull() ? QStringLiteral("no address in response") : QString());
}

///
/// \brief CAddrRequest::finish
/// \param pError empty on success
///
void CAddrRequest::finish(const QString& pError)
{
    if (m_done) return;
    m_done = true;
    m_timeout.stop();
    m_error = pError;
    if (!pError.isEmpty()) m_yourIP.clear();
    if (m_socket != nullptr) m_socket->abort();
    emit finished(this);
}
