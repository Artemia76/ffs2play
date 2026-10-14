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
** upnpclient.h is part of FF2Play project
**
** Minimal asynchronous UPnP IGD client (SSDP discovery + SOAP port mapping)
** built on Qt Network only.
****************************************************************************/

#ifndef UPNPCLIENT_H
#define UPNPCLIENT_H

#include <QObject>
#include <QHostAddress>
#include <QUrl>
#include <QTimer>
#include <QStringList>
#include <functional>

class QUdpSocket;
class QNetworkAccessManager;
class QNetworkReply;

class CUPnPClient : public QObject
{
    Q_OBJECT
public:
    enum class Error
    {
        NoDevice,       // No answer to SSDP discovery
        NoValidIGD      // Devices found, but none provide a WAN connection service
    };
    Q_ENUM(Error)

    struct IGDService
    {
        QUrl    controlUrl;
        QString serviceType;
        bool    isValid() const { return controlUrl.isValid() && !serviceType.isEmpty(); }
    };

    explicit                CUPnPClient(QObject* pParent = nullptr);
                            ~CUPnPClient() override;

    // Discover the gateway then map pPort for UDP and TCP. Result is notified by signals.
    void                    addPortMappings(quint16 pPort);
    // Remove the mappings previously added. Blocks at most pTimeoutMs.
    void                    removePortMappings(int pTimeoutMs = 2000);
    // Cancel any discovery or pending request
    void                    abort();
    bool                    hasMappings() const;

    // Parsers, public for unit tests
    static QUrl             parseSsdpLocation(const QByteArray& pResponse);
    static IGDService       parseDescription(const QByteArray& pXml, const QUrl& pLocation);
    static QString          soapValue(const QByteArray& pXml, const QString& pTag);
    static QByteArray       soapEnvelope(const QString& pServiceType, const QString& pAction,
                                         const QList<QPair<QString,QString>>& pArgs);

signals:
    void                    failed(CUPnPClient::Error pError);
    void                    externalAddressFound(QHostAddress pAddress);
    void                    portMapped(QString pProtocol, int pErrorCode);
    void                    portUnmapped(QString pProtocol, int pErrorCode);

private slots:
    void                    onSsdpReadyRead();
    void                    onDiscoveryTimeout();

private:
    using SoapCallback = std::function<void(int pErrorCode, const QByteArray& pBody)>;

    void                    fetchNextDescription();
    void                    onGatewayFound();
    void                    mapNextProtocol();
    void                    soapRequest(const QString& pAction, const QList<QPair<QString,QString>>& pArgs,
                                        SoapCallback pCallback);
    QHostAddress            localAddressFor(const QHostAddress& pGateway) const;

    QUdpSocket*             m_ssdp;
    QNetworkAccessManager*  m_nam;
    QTimer                  m_discoveryTimer;
    QList<QUrl>             m_locations;
    bool                    m_fetching;
    bool                    m_deviceSeen;
    IGDService              m_service;
    QHostAddress            m_lanAddress;
    quint16                 m_port;
    QStringList             m_pendingProtocols;
    QStringList             m_mappedProtocols;
    quint32                 m_generation;   // incremented by abort() to ignore stale replies
};

#endif // UPNPCLIENT_H
