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
** tst_core.cpp is part of FF2Play project
**
** Unit tests for crypto, protobuf wire format and UPnP parsers
****************************************************************************/

#include <QtTest>
#include <QProtobufSerializer>
#include <QTcpServer>
#include <QTcpSocket>
#include <memory>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include "ffs2play.qpb.h"
#include "net/upnpclient.h"
#include "web/caddrrequest.h"
#include "tools/crypto.h"
#include "tools/datagram.h"

class TestCore : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();

    void aesRoundTrip_data();
    void aesRoundTrip();
    void aesRejectsGarbage();
    void rsaEncrypt();

    void airDataRoundTrip();
    void airDataGoogleWireFormat();
    void stringArrayBase64();

    // Strings of the X-Plane link and peer datagrams (tools/datagram)
    void streamStringRoundTrip();
    void streamStringTruncatedUtf8();
    void streamCStringFixedField();

    void ssdpLocation();
    void igdDescription();
    void soapParsing();

    void addrRequestForcedLayer_data();
    void addrRequestForcedLayer();
    void addrRequestHttpError();
};

namespace
{
    /// Dual stack HTTP server answering like the portal : your_ip = address of the caller
    class FakePortal : public QTcpServer
    {
    public:
        QByteArray status = "200 OK";
        QByteArray lastRequest;
        explicit FakePortal()
        {
            connect(this, &QTcpServer::newConnection, this, [this]()
            {
                QTcpSocket* socket = nextPendingConnection();
                connect(socket, &QTcpSocket::readyRead, socket, [this, socket]()
                {
                    lastRequest += socket->readAll();
                    const qsizetype end = lastRequest.indexOf("\r\n\r\n");
                    if (end < 0) return;
                    const QByteArray length = lastRequest.mid(lastRequest.indexOf("Content-Length: ") + 16).split('\r').first();
                    if (lastRequest.size() < end + 4 + length.toInt()) return;
                    socket->write("HTTP/1.0 " + status + "\r\nContent-Type: text/xml\r\n\r\n"
                                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<info><your_ip>"
                                  + socket->peerAddress().toString().toLatin1() + "</your_ip></info>\n");
                    socket->disconnectFromHost();
                });
            });
        }
    };
}

void TestCore::addrRequestForcedLayer_data()
{
    QTest::addColumn<int>("protocol");
    QTest::addColumn<QString>("expected");
    QTest::newRow("IPv4") << int(QAbstractSocket::IPv4Protocol) << "127.0.0.1";
    QTest::newRow("IPv6") << int(QAbstractSocket::IPv6Protocol) << "::1";
}

void TestCore::addrRequestForcedLayer()
{
    QFETCH(int, protocol);
    QFETCH(QString, expected);
    FakePortal portal;
    QVERIFY(portal.listen(QHostAddress::Any, 0));
    if ((protocol == QAbstractSocket::IPv6Protocol) && !QTcpSocket().bind(QHostAddress::LocalHostIPv6, 0))
        QSKIP("no IPv6 loopback");

    CAddrRequest request;
    QSignalSpy finished(&request, &CAddrRequest::finished);
    const QByteArray content = "<FFS2Play><switch><data>addr</data></switch></FFS2Play>";
    request.execute(QUrl(QString("http://localhost:%1/action.php").arg(portal.serverPort())), content,
                    static_cast<QAbstractSocket::NetworkLayerProtocol>(protocol));
    QVERIFY(finished.wait(5000));
    QVERIFY2(request.errorString().isEmpty(), qPrintable(request.errorString()));
    // the layer is forced : the caller address seen by the server is of the requested family
    QVERIFY(request.yourIP().isEqual(QHostAddress(expected), QHostAddress::StrictConversion)
            || ((protocol == QAbstractSocket::IPv4Protocol) && request.yourIP().isEqual(QHostAddress(expected))));
    QVERIFY(portal.lastRequest.startsWith("POST /action.php HTTP/1.0\r\n"));
    QVERIFY(portal.lastRequest.contains(QString("Host: localhost:%1\r\n").arg(portal.serverPort()).toLatin1()));
    QVERIFY(portal.lastRequest.endsWith(content));
}

void TestCore::addrRequestHttpError()
{
    FakePortal portal;
    portal.status = "500 Internal Server Error";
    QVERIFY(portal.listen(QHostAddress::Any, 0));
    CAddrRequest request;
    QSignalSpy finished(&request, &CAddrRequest::finished);
    request.execute(QUrl(QString("http://localhost:%1/action.php").arg(portal.serverPort())), "<FFS2Play/>",
                    QAbstractSocket::IPv4Protocol);
    QVERIFY(finished.wait(5000));
    QVERIFY(request.yourIP().isNull());
    QVERIFY(!request.errorString().isEmpty());
}

void TestCore::initTestCase()
{
    QCoreApplication::setOrganizationName("FFS2P_Tests");
    QCoreApplication::setApplicationName("tst_core");
}

void TestCore::aesRoundTrip_data()
{
    QTest::addColumn<QString>("plain");
    QTest::newRow("empty") << QString();
    QTest::newRow("ascii") << QString("192.168.1.10");
    QTest::newRow("utf8") << QString::fromUtf8("Mot de passe éèà ✈");
    // Long input : the old implementation truncated the cipher at the first zero byte
    QTest::newRow("long") << QString("0123456789abcdef").repeated(64);
}

void TestCore::aesRoundTrip()
{
    QFETCH(QString, plain);
    CCrypto crypto;
    QCOMPARE(crypto.EncryptionKey().size(), 32);
    for (int i = 0; i < 50; ++i)
    {
        const QString cipher = crypto.EncryptToBase64(plain);
        QVERIFY(!cipher.isEmpty());
        QCOMPARE(crypto.DecryptFromBase64(cipher), plain);
    }
}

void TestCore::aesRejectsGarbage()
{
    CCrypto crypto;
    QCOMPARE(crypto.DecryptFromBase64(QString()), QString());
    QCOMPARE(crypto.DecryptFromBase64(QString("bm90IGVuY3J5cHRlZA==")), QString());
}

namespace
{
    struct PkeyDeleter { void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); } };
    struct X509Deleter { void operator()(X509* p) const { X509_free(p); } };
    struct BioDeleter { void operator()(BIO* p) const { BIO_free(p); } };
    struct CtxDeleter { void operator()(EVP_PKEY_CTX* p) const { EVP_PKEY_CTX_free(p); } };
}

void TestCore::rsaEncrypt()
{
    // Self signed certificate standing for the server one
    std::unique_ptr<EVP_PKEY, PkeyDeleter> key(EVP_RSA_gen(2048));
    QVERIFY(key);
    std::unique_ptr<X509, X509Deleter> cert(X509_new());
    X509_set_version(cert.get(), 2);
    ASN1_INTEGER_set(X509_get_serialNumber(cert.get()), 1);
    X509_gmtime_adj(X509_getm_notBefore(cert.get()), 0);
    X509_gmtime_adj(X509_getm_notAfter(cert.get()), 3600);
    X509_set_pubkey(cert.get(), key.get());
    X509_NAME* name = X509_NAME_new();
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>("ffs2play-test"), -1, -1, 0);
    X509_set_subject_name(cert.get(), name);
    X509_set_issuer_name(cert.get(), name);
    X509_NAME_free(name);
    QVERIFY(X509_sign(cert.get(), key.get(), EVP_sha256()) > 0);
    std::unique_ptr<BIO, BioDeleter> bio(BIO_new(BIO_s_mem()));
    QVERIFY(PEM_write_bio_X509(bio.get(), cert.get()) == 1);
    char* pemData = nullptr;
    const long pemLen = BIO_get_mem_data(bio.get(), &pemData);
    const QString pem = QString::fromLatin1(pemData, pemLen);

    CCrypto crypto;
    const QString encrypted = crypto.RSA_EncryptToBase64(pem, crypto.EncryptionKey());
    const QByteArray raw = QByteArray::fromBase64(encrypted.toLatin1());
    QCOMPARE(raw.size(), 256);

    // Decrypt with the private key, as the server does
    std::unique_ptr<EVP_PKEY_CTX, CtxDeleter> ctx(EVP_PKEY_CTX_new(key.get(), nullptr));
    QVERIFY(EVP_PKEY_decrypt_init(ctx.get()) > 0);
    QVERIFY(EVP_PKEY_CTX_set_rsa_padding(ctx.get(), RSA_PKCS1_PADDING) > 0);
    size_t outLen = 512;
    QByteArray out(512, '\0');
    QVERIFY(EVP_PKEY_decrypt(ctx.get(), reinterpret_cast<unsigned char*>(out.data()), &outLen,
                             reinterpret_cast<const unsigned char*>(raw.constData()), raw.size()) > 0);
    out.resize(static_cast<qsizetype>(outLen));
    QCOMPARE(out, crypto.EncryptionKey());

    QCOMPARE(crypto.RSA_EncryptToBase64(QString("not a certificate"), QByteArray("x")), QString());
}

void TestCore::airDataRoundTrip()
{
    AirData data;
    data.setTimestamp(1234567890123LL);
    data.setTitle(QString::fromUtf8("Airbus A320 Air France ✈"));
    data.setLatitude(49.4567);
    data.setLongitude(-1.2345);
    data.setIasSpeed(142.5);
    data.setDoor1Pos(100);
    data.setOnGround(true);
    data.setSpoilerPos(0.25);
    data.setSquawkMode(2);

    const QByteArray wire = ProtobufSerialize(data);
    QVERIFY(!wire.isEmpty());
    AirData parsed;
    QVERIFY(ProtobufParse(parsed, wire));
    QCOMPARE(parsed, data);

    // ProtobufToStream writes the same bytes after any header
    QByteArray buffer;
    QDataStream stream(&buffer, QIODevice::WriteOnly);
    stream << static_cast<quint8>(42);
    ProtobufToStream(stream, data);
    QCOMPARE(buffer.mid(1), wire);
}

void TestCore::airDataGoogleWireFormat()
{
    // Bytes as produced by google protobuf (field number order), i.e. by the
    // X-Plane plugin and by FFS2Play Qt5 clients
    const QByteArray wire = QByteArray::fromHex(
        "0801"                  // 1  timestamp = 1
        "120141"                // 2  title = "A"
        "31000000000000f03f"    // 6  altitude = 1.0
        "7064"                  // 14 squawk = 100
        "e00101"                // 28 on_ground = true
        "f902000000000000e03f"  // 47 spoiler_pos = 0.5
    );
    AirData data;
    {
        QProtobufSerializer serializer;
        const bool ok = data.deserialize(&serializer, wire);
        QVERIFY2(ok, qPrintable(serializer.lastErrorString()));
    }
    QCOMPARE(data.timestamp(), 1);
    QCOMPARE(data.title(), QString("A"));
    QCOMPARE(data.altitude(), 1.0);
    QCOMPARE(data.squawk(), 100);
    QCOMPARE(data.onGround(), true);
    QCOMPARE(data.spoilerPos(), 0.5);

    // Field order may differ, but a re-encode must parse back identically
    AirData reparsed;
    QVERIFY(ProtobufParse(reparsed, ProtobufSerialize(data)));
    QCOMPARE(reparsed, data);
}

void TestCore::stringArrayBase64()
{
    StringArray ips;
    ips.setValues({"192.168.1.10", "10.0.0.2"});
    const QByteArray encoded = ProtobufSerialize(ips).toBase64();
    // Same bytes as google protobuf : two length delimited strings with tag 1
    QCOMPARE(QByteArray::fromBase64(encoded),
             QByteArray::fromHex("0a0c3139322e3136382e312e3130") + QByteArray::fromHex("0a0831302e302e302e32"));
    StringArray decoded;
    QVERIFY(ProtobufParse(decoded, QByteArray::fromBase64(encoded)));
    QCOMPARE(decoded.values(), ips.values());
}

void TestCore::streamStringRoundTrip()
{
    const QString Text = QStringLiteral("F-GABC é 飞机");
    QByteArray Data;
    {
        QDataStream Out(&Data, QIODevice::WriteOnly);
        StringToStream(Out, Text);
    }
    QCOMPARE(static_cast<quint8>(Data.at(0)), static_cast<quint8>(Text.toUtf8().size()));
    QDataStream In(&Data, QIODevice::ReadOnly);
    QCOMPARE(StringFromStream(In), Text);
    QVERIFY(In.atEnd());
}

void TestCore::streamStringTruncatedUtf8()
{
    // 254 ASCII bytes + a 2 bytes character : 256 bytes, beyond the one byte length
    const QString Text = QString(254, QLatin1Char('a')) + QStringLiteral("é");
    QByteArray Data;
    {
        QDataStream Out(&Data, QIODevice::WriteOnly);
        StringToStream(Out, Text);
    }
    // The length byte matches the written bytes and the UTF-8 sequence is not cut
    QCOMPARE(static_cast<quint8>(Data.at(0)), quint8(254));
    QCOMPARE(Data.size(), 1 + 254);
    QDataStream In(&Data, QIODevice::ReadOnly);
    QCOMPARE(StringFromStream(In), QString(254, QLatin1Char('a')));
}

void TestCore::streamCStringFixedField()
{
    // Null padded field and full field without terminator
    QByteArray Data("ABC\0\0\0\0\0WXYZ", 12);
    QDataStream In(&Data, QIODevice::ReadOnly);
    QCOMPARE(CStringFromStream(In, 8), QStringLiteral("ABC"));
    QCOMPARE(CStringFromStream(In, 4), QStringLiteral("WXYZ"));
}

void TestCore::ssdpLocation()
{
    const QByteArray answer =
        "HTTP/1.1 200 OK\r\n"
        "CACHE-CONTROL: max-age=120\r\n"
        "ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
        "USN: uuid:1234::urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
        "Location: http://192.168.1.1:5000/rootDesc.xml\r\n"
        "SERVER: Linux UPnP/1.1 MiniUPnPd/2.2\r\n\r\n";
    QCOMPARE(CUPnPClient::parseSsdpLocation(answer), QUrl("http://192.168.1.1:5000/rootDesc.xml"));
    QVERIFY(!CUPnPClient::parseSsdpLocation("NOTIFY * HTTP/1.1\r\nLOCATION: http://x/\r\n\r\n").isValid());
    QVERIFY(!CUPnPClient::parseSsdpLocation("HTTP/1.1 200 OK\r\n\r\n").isValid());
}

void TestCore::igdDescription()
{
    const QByteArray xml = R"(<?xml version="1.0"?>
<root xmlns="urn:schemas-upnp-org:device-1-0">
  <device>
    <deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>
    <serviceList>
      <service>
        <serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType>
        <controlURL>/ctl/L3F</controlURL>
      </service>
    </serviceList>
    <deviceList>
      <device>
        <deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>
        <deviceList>
          <device>
            <deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>
            <serviceList>
              <service>
                <serviceType>urn:schemas-upnp-org:service:WANPPPConnection:1</serviceType>
                <controlURL>/ctl/PPP</controlURL>
              </service>
              <service>
                <serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>
                <controlURL>/ctl/IPConn</controlURL>
              </service>
            </serviceList>
          </device>
        </deviceList>
      </device>
    </deviceList>
  </device>
</root>)";
    const CUPnPClient::IGDService service =
        CUPnPClient::parseDescription(xml, QUrl("http://192.168.1.1:5000/rootDesc.xml"));
    QVERIFY(service.isValid());
    QCOMPARE(service.serviceType, QString("urn:schemas-upnp-org:service:WANIPConnection:1"));
    QCOMPARE(service.controlUrl, QUrl("http://192.168.1.1:5000/ctl/IPConn"));

    // URLBase takes precedence over the description location
    QByteArray withBase = xml;
    withBase.replace("<device>", "<URLBase>http://10.0.0.1:49000/</URLBase><device>");
    QCOMPARE(CUPnPClient::parseDescription(withBase, QUrl("http://192.168.1.1:5000/rootDesc.xml")).controlUrl,
             QUrl("http://10.0.0.1:49000/ctl/IPConn"));

    QVERIFY(!CUPnPClient::parseDescription("<root/>", QUrl("http://192.168.1.1/")).isValid());
}

void TestCore::soapParsing()
{
    const QByteArray envelope = CUPnPClient::soapEnvelope(
        "urn:schemas-upnp-org:service:WANIPConnection:1", "AddPortMapping",
        {{"NewExternalPort", "54857"}, {"NewPortMappingDescription", "FFS2P_UDP_54857"}});
    QCOMPARE(CUPnPClient::soapValue(envelope, "NewExternalPort"), QString("54857"));
    QVERIFY(envelope.contains("<u:AddPortMapping xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"));

    const QByteArray answer = R"(<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
<s:Body><u:GetExternalIPAddressResponse xmlns:u="urn:schemas-upnp-org:service:WANIPConnection:1">
<NewExternalIPAddress>82.64.12.34</NewExternalIPAddress>
</u:GetExternalIPAddressResponse></s:Body></s:Envelope>)";
    QCOMPARE(CUPnPClient::soapValue(answer, "NewExternalIPAddress"), QString("82.64.12.34"));

    const QByteArray fault = R"(<?xml version="1.0"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><s:Fault>
<faultcode>s:Client</faultcode><faultstring>UPnPError</faultstring>
<detail><UPnPError xmlns="urn:schemas-upnp-org:control-1-0">
<errorCode>718</errorCode><errorDescription>ConflictInMappingEntry</errorDescription>
</UPnPError></detail></s:Fault></s:Body></s:Envelope>)";
    QCOMPARE(CUPnPClient::soapValue(fault, "errorCode").toInt(), 718);
}

QTEST_GUILESS_MAIN(TestCore)
#include "tst_core.moc"
