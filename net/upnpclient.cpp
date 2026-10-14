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
** upnpclient.cpp is part of FF2Play project
**
** Minimal asynchronous UPnP IGD client (SSDP discovery + SOAP port mapping)
** built on Qt Network only.
****************************************************************************/

#include "net/upnpclient.h"

#include <QDomDocument>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUdpSocket>

#include "tools/clogger.h"

namespace
{
    const QHostAddress  SSDP_ADDRESS(QStringLiteral("239.255.255.250"));
    const quint16       SSDP_PORT = 1900;
    const int           DISCOVERY_TIMEOUT_MS = 2500;
    const int           HTTP_TIMEOUT_MS = 3000;

    const char* const   SEARCH_TARGETS[] = {
        "urn:schemas-upnp-org:device:InternetGatewayDevice:1",
        "urn:schemas-upnp-org:device:InternetGatewayDevice:2",
        "urn:schemas-upnp-org:service:WANIPConnection:1",
        "urn:schemas-upnp-org:service:WANPPPConnection:1"
    };

    // Same meaning as miniupnpc UPNPCOMMAND_HTTP_ERROR
    const int           HTTP_ERROR = -3;

    ///
    /// \brief localName strip the namespace prefix of a tag name
    ///
    QString localName(const QDomElement& pElement)
    {
        const QString tag = pElement.tagName();
        const qsizetype pos = tag.indexOf(':');
        return pos < 0 ? tag : tag.mid(pos + 1);
    }

    ///
    /// \brief findElements depth first search of elements by local name
    ///
    void findElements(const QDomElement& pRoot, const QString& pName, QList<QDomElement>& pResult)
    {
        for (QDomElement child = pRoot.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
        {
            if (localName(child) == pName) pResult.append(child);
            findElements(child, pName, pResult);
        }
    }

    QString childText(const QDomElement& pParent, const QString& pName)
    {
        for (QDomElement child = pParent.firstChildElement(); !child.isNull(); child = child.nextSiblingElement())
        {
            if (localName(child) == pName) return child.text().trimmed();
        }
        return QString();
    }
}

///
/// \brief CUPnPClient::CUPnPClient
/// \param pParent
///
CUPnPClient::CUPnPClient(QObject* pParent) :
    QObject(pParent),
    m_ssdp(nullptr),
    m_nam(new QNetworkAccessManager(this)),
    m_fetching(false),
    m_deviceSeen(false),
    m_port(0),
    m_generation(0)
{
    m_discoveryTimer.setSingleShot(true);
    connect(&m_discoveryTimer, &QTimer::timeout, this, &CUPnPClient::onDiscoveryTimeout);
}

///
/// \brief CUPnPClient::~CUPnPClient
///
CUPnPClient::~CUPnPClient()
{
    abort();
}

///
/// \brief CUPnPClient::hasMappings
/// \return
///
bool CUPnPClient::hasMappings() const
{
    return !m_mappedProtocols.isEmpty();
}

///
/// \brief CUPnPClient::addPortMappings
/// \param pPort
///
void CUPnPClient::addPortMappings(quint16 pPort)
{
    abort();
    m_port = pPort;
    m_pendingProtocols = QStringList{QStringLiteral("UDP"), QStringLiteral("TCP")};
    m_locations.clear();
    m_deviceSeen = false;
    m_service = IGDService();

    m_ssdp = new QUdpSocket(this);
    connect(m_ssdp, &QUdpSocket::readyRead, this, &CUPnPClient::onSsdpReadyRead);
    if (!m_ssdp->bind(QHostAddress(QHostAddress::AnyIPv4), 0))
    {
        emit failed(Error::NoDevice);
        return;
    }
    // Send the search on every multicast capable interface
    QList<QNetworkInterface> interfaces;
    const QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : all)
    {
        const auto flags = iface.flags();
        if ((flags & QNetworkInterface::IsUp) && (flags & QNetworkInterface::IsRunning)
            && (flags & QNetworkInterface::CanMulticast) && !(flags & QNetworkInterface::IsLoopBack))
            interfaces.append(iface);
    }
    if (interfaces.isEmpty()) interfaces.append(QNetworkInterface());
    for (const QNetworkInterface& iface : std::as_const(interfaces))
    {
        if (iface.isValid()) m_ssdp->setMulticastInterface(iface);
        for (const char* target : SEARCH_TARGETS)
        {
            const QByteArray search =
                "M-SEARCH * HTTP/1.1\r\n"
                "HOST: 239.255.255.250:1900\r\n"
                "MAN: \"ssdp:discover\"\r\n"
                "MX: 2\r\n"
                "ST: " + QByteArray(target) + "\r\n\r\n";
            m_ssdp->writeDatagram(search, SSDP_ADDRESS, SSDP_PORT);
        }
    }
    m_discoveryTimer.start(DISCOVERY_TIMEOUT_MS);
}

///
/// \brief CUPnPClient::abort
///
void CUPnPClient::abort()
{
    ++m_generation;
    m_discoveryTimer.stop();
    m_fetching = false;
    if (m_ssdp != nullptr)
    {
        m_ssdp->close();
        m_ssdp->deleteLater();
        m_ssdp = nullptr;
    }
    const QList<QNetworkReply*> replies = m_nam->findChildren<QNetworkReply*>();
    for (QNetworkReply* reply : replies) reply->abort();
}

///
/// \brief CUPnPClient::onSsdpReadyRead
///
void CUPnPClient::onSsdpReadyRead()
{
    while ((m_ssdp != nullptr) && m_ssdp->hasPendingDatagrams())
    {
        const QNetworkDatagram datagram = m_ssdp->receiveDatagram();
        const QUrl location = parseSsdpLocation(datagram.data());
        if (location.isValid() && !m_locations.contains(location))
        {
#ifdef FFS_DEBUG
            CLogger::instance()->log("UPnP : IGD Desc = " + location.toString(),Qt::darkBlue,LEVEL_NORMAL);
#endif
            m_locations.append(location);
            m_deviceSeen = true;
        }
    }
    // Start with the first answer, keep listening for alternatives
    if (!m_fetching && !m_service.isValid() && !m_locations.isEmpty()) fetchNextDescription();
}

///
/// \brief CUPnPClient::onDiscoveryTimeout
///
void CUPnPClient::onDiscoveryTimeout()
{
    if (m_ssdp != nullptr)
    {
        m_ssdp->close();
        m_ssdp->deleteLater();
        m_ssdp = nullptr;
    }
    if (!m_fetching && !m_service.isValid())
        emit failed(m_deviceSeen ? Error::NoValidIGD : Error::NoDevice);
}

///
/// \brief CUPnPClient::fetchNextDescription
///
void CUPnPClient::fetchNextDescription()
{
    if (m_locations.isEmpty())
    {
        m_fetching = false;
        // Wait for other answers while discovery is still running
        if (!m_discoveryTimer.isActive()) emit failed(Error::NoValidIGD);
        return;
    }
    m_fetching = true;
    const QUrl location = m_locations.takeFirst();
    QNetworkRequest request(location);
    request.setTransferTimeout(HTTP_TIMEOUT_MS);
    QNetworkReply* reply = m_nam->get(request);
    const quint32 generation = m_generation;
    connect(reply, &QNetworkReply::finished, this, [this, reply, location, generation]()
    {
        reply->deleteLater();
        if (generation != m_generation) return;
        IGDService service;
        if (reply->error() == QNetworkReply::NoError)
            service = parseDescription(reply->readAll(), location);
        if (service.isValid())
        {
            m_fetching = false;
            m_service = service;
            onGatewayFound();
        }
        else fetchNextDescription();
    });
}

///
/// \brief CUPnPClient::onGatewayFound
///
void CUPnPClient::onGatewayFound()
{
    m_discoveryTimer.stop();
    if (m_ssdp != nullptr)
    {
        m_ssdp->close();
        m_ssdp->deleteLater();
        m_ssdp = nullptr;
    }
    m_lanAddress = localAddressFor(QHostAddress(m_service.controlUrl.host()));
#ifdef FFS_DEBUG
    CLogger::instance()->log("UPnP : Found valid IGD = " + m_service.controlUrl.toString()
                             + " service = " + m_service.serviceType
                             + " LAN address = " + m_lanAddress.toString(),Qt::darkBlue,LEVEL_NORMAL);
#endif
    soapRequest(QStringLiteral("GetExternalIPAddress"), {}, [this](int pError, const QByteArray& pBody)
    {
        if (pError == 0)
        {
            const QHostAddress external(soapValue(pBody, QStringLiteral("NewExternalIPAddress")));
            if (!external.isNull()) emit externalAddressFound(external);
        }
        mapNextProtocol();
    });
}

///
/// \brief CUPnPClient::mapNextProtocol
///
void CUPnPClient::mapNextProtocol()
{
    if (m_pendingProtocols.isEmpty()) return;
    const QString protocol = m_pendingProtocols.takeFirst();
    const QString port = QString::number(m_port);
    soapRequest(QStringLiteral("AddPortMapping"),
                {
                    {QStringLiteral("NewRemoteHost"), QString()},
                    {QStringLiteral("NewExternalPort"), port},
                    {QStringLiteral("NewProtocol"), protocol},
                    {QStringLiteral("NewInternalPort"), port},
                    {QStringLiteral("NewInternalClient"), m_lanAddress.toString()},
                    {QStringLiteral("NewEnabled"), QStringLiteral("1")},
                    {QStringLiteral("NewPortMappingDescription"), "FFS2P_" + protocol + "_" + port},
                    {QStringLiteral("NewLeaseDuration"), QStringLiteral("0")}
                },
                [this, protocol](int pError, const QByteArray&)
    {
        if (pError == 0) m_mappedProtocols.append(protocol);
        emit portMapped(protocol, pError);
        mapNextProtocol();
    });
}

///
/// \brief CUPnPClient::removePortMappings
/// \param pTimeoutMs
///
void CUPnPClient::removePortMappings(int pTimeoutMs)
{
    abort();
    if (m_mappedProtocols.isEmpty() || !m_service.isValid()) return;
    const QStringList protocols = m_mappedProtocols;
    m_mappedProtocols.clear();
    int pending = static_cast<int>(protocols.size());
    QEventLoop loop;
    for (const QString& protocol : protocols)
    {
        soapRequest(QStringLiteral("DeletePortMapping"),
                    {
                        {QStringLiteral("NewRemoteHost"), QString()},
                        {QStringLiteral("NewExternalPort"), QString::number(m_port)},
                        {QStringLiteral("NewProtocol"), protocol}
                    },
                    [this, protocol, &pending, &loop](int pError, const QByteArray&)
        {
            emit portUnmapped(protocol, pError);
            if (--pending == 0) loop.quit();
        });
    }
    QTimer::singleShot(pTimeoutMs, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    // Late replies must not touch the destroyed loop
    abort();
}

///
/// \brief CUPnPClient::soapRequest
/// \param pAction
/// \param pArgs
/// \param pCallback called with 0 on success, UPnP error code or HTTP_ERROR on failure
///
void CUPnPClient::soapRequest(const QString& pAction, const QList<QPair<QString,QString>>& pArgs,
                              SoapCallback pCallback)
{
    QNetworkRequest request(m_service.controlUrl);
    request.setTransferTimeout(HTTP_TIMEOUT_MS);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/xml; charset=\"utf-8\""));
    request.setRawHeader("SOAPAction", "\"" + m_service.serviceType.toUtf8() + "#" + pAction.toUtf8() + "\"");
    QNetworkReply* reply = m_nam->post(request, soapEnvelope(m_service.serviceType, pAction, pArgs));
    const quint32 generation = m_generation;
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation, pCallback]()
    {
        reply->deleteLater();
        if (generation != m_generation) return;
        const QByteArray body = reply->readAll();
        int error = 0;
        if (reply->error() != QNetworkReply::NoError)
        {
            bool ok = false;
            error = soapValue(body, QStringLiteral("errorCode")).toInt(&ok);
            if (!ok || error == 0) error = HTTP_ERROR;
        }
        pCallback(error, body);
    });
}

///
/// \brief CUPnPClient::localAddressFor
/// \param pGateway
/// \return the local IPv4 address on the gateway's subnet
///
QHostAddress CUPnPClient::localAddressFor(const QHostAddress& pGateway) const
{
    const QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : all)
    {
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry& entry : entries)
        {
            if ((entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                && pGateway.isInSubnet(entry.ip(), entry.prefixLength()))
                return entry.ip();
        }
    }
    // Fallback : let the routing table choose
    QUdpSocket probe;
    probe.connectToHost(pGateway, SSDP_PORT);
    return probe.localAddress();
}

///
/// \brief CUPnPClient::parseSsdpLocation
/// \param pResponse SSDP HTTP over UDP answer
/// \return the LOCATION header
///
QUrl CUPnPClient::parseSsdpLocation(const QByteArray& pResponse)
{
    const QList<QByteArray> lines = pResponse.split('\n');
    if (lines.isEmpty() || !lines.first().trimmed().toUpper().startsWith("HTTP/1.1 200")) return QUrl();
    for (const QByteArray& line : lines)
    {
        const qsizetype pos = line.indexOf(':');
        if (pos < 0) continue;
        if (line.left(pos).trimmed().toUpper() == "LOCATION")
        {
            const QUrl url(QString::fromLatin1(line.mid(pos + 1).trimmed()));
            if (url.isValid() && url.scheme().startsWith(QLatin1String("http"))) return url;
        }
    }
    return QUrl();
}

///
/// \brief CUPnPClient::parseDescription
/// \param pXml device description
/// \param pLocation URL of the description
/// \return the WAN connection service, WANIPConnection preferred
///
CUPnPClient::IGDService CUPnPClient::parseDescription(const QByteArray& pXml, const QUrl& pLocation)
{
    IGDService result;
    QDomDocument doc;
    if (!doc.setContent(pXml)) return result;
    const QDomElement root = doc.documentElement();
    QUrl base = pLocation;
    const QString urlBase = childText(root, QStringLiteral("URLBase"));
    if (!urlBase.isEmpty()) base = QUrl(urlBase);

    QList<QDomElement> services;
    findElements(root, QStringLiteral("service"), services);
    for (const QDomElement& service : std::as_const(services))
    {
        const QString type = childText(service, QStringLiteral("serviceType"));
        const QString control = childText(service, QStringLiteral("controlURL"));
        if (control.isEmpty()) continue;
        const bool ip = type.startsWith(QLatin1String("urn:schemas-upnp-org:service:WANIPConnection:"));
        const bool ppp = type.startsWith(QLatin1String("urn:schemas-upnp-org:service:WANPPPConnection:"));
        if (!ip && !ppp) continue;
        if (ip || !result.isValid())
        {
            result.serviceType = type;
            result.controlUrl = base.resolved(QUrl(control));
        }
        if (ip) break;
    }
    return result;
}

///
/// \brief CUPnPClient::soapValue
/// \param pXml SOAP answer
/// \param pTag local name of the element
/// \return text of the first element named pTag
///
QString CUPnPClient::soapValue(const QByteArray& pXml, const QString& pTag)
{
    QDomDocument doc;
    if (!doc.setContent(pXml)) return QString();
    QList<QDomElement> found;
    findElements(doc.documentElement(), pTag, found);
    return found.isEmpty() ? QString() : found.first().text().trimmed();
}

///
/// \brief CUPnPClient::soapEnvelope
/// \param pServiceType
/// \param pAction
/// \param pArgs
/// \return
///
QByteArray CUPnPClient::soapEnvelope(const QString& pServiceType, const QString& pAction,
                                     const QList<QPair<QString,QString>>& pArgs)
{
    QByteArray body;
    body += "<?xml version=\"1.0\"?>\r\n"
            "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
            "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
            "<s:Body><u:" + pAction.toUtf8() + " xmlns:u=\"" + pServiceType.toUtf8() + "\">";
    for (const auto& arg : pArgs)
    {
        body += "<" + arg.first.toUtf8() + ">" + arg.second.toHtmlEscaped().toUtf8() + "</" + arg.first.toUtf8() + ">";
    }
    body += "</u:" + pAction.toUtf8() + "></s:Body></s:Envelope>\r\n";
    return body;
}
