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
#include "tools/fgpropertylist.h"
#include "sim/fgdatagram.h"
#include "tools/calculation.h"

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

    void fgPropertyList();
    void fgPropertyListInclude();
    void fgPropertyListInvalid();
    void fgPropertyListRealAircraft();
    void fgDatagramPosition();
    void fgOrientationRoundTrip_data();
    void fgOrientationRoundTrip();
    void fgDatagramWriteRead();
    void fgPropertyListAliases();
    void fgDatagramProperties();
    void fgDatagramVelocity();
    void fgDatagramAirDataRoundTrip();
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

namespace
{
bool writeTextFile(const QString& pPath, const QByteArray& pContent)
{
    QDir().mkpath(QFileInfo(pPath).absolutePath());
    QFile File(pPath);
    return File.open(QIODevice::WriteOnly) && File.write(pContent) == pContent.size();
}
}

void TestCore::fgPropertyList()
{
    QTemporaryDir Dir;
    QVERIFY(Dir.isValid());
    const QString SetFile = Dir.filePath("Aircraft/dr400/dr400-set.xml");
    QVERIFY(writeTextFile(SetFile, R"(<?xml version="1.0"?>
<PropertyList>
  <sim>
    <description>Robin DR400</description>
    <author>Someone</author>
    <model><path>Aircraft/dr400/Models/dr400.xml</path></model>
    <tags>
      <tag>ga</tag>
      <tag>piston</tag>
      <tag>single-engine</tag>
    </tags>
  </sim>
  <aircraft><icao><type>DR40</type></icao></aircraft>
</PropertyList>
)"));
    CFGPropertyList Props;
    QVERIFY(Props.read(SetFile));
    QCOMPARE(Props.value("sim/model/path"), QStringLiteral("Aircraft/dr400/Models/dr400.xml"));
    QCOMPARE(Props.value("aircraft/icao/type"), QStringLiteral("DR40"));
    QCOMPARE(Props.value("sim/description"), QStringLiteral("Robin DR400"));
    QCOMPARE(Props.values("sim/tags/tag"), QStringList({"ga", "piston", "single-engine"}));
    QVERIFY(Props.value("sim/missing").isEmpty());
}

void TestCore::fgPropertyListInclude()
{
    QTemporaryDir Dir;
    QVERIFY(Dir.isValid());
    // Root include relative to the file, node include relative to a parent folder
    const QString SetFile = Dir.filePath("Aircraft/c172p/c172p-set.xml");
    QVERIFY(writeTextFile(SetFile, R"(<?xml version="1.0"?>
<PropertyList include="c172p-common.xml">
  <sim>
    <description>Cessna 172P floats</description>
    <tags include="Aircraft/Generic/tags.xml"/>
  </sim>
</PropertyList>
)"));
    QVERIFY(writeTextFile(Dir.filePath("Aircraft/c172p/c172p-common.xml"), R"(<?xml version="1.0"?>
<PropertyList>
  <sim>
    <description>Cessna 172P</description>
    <model><path>Aircraft/c172p/Models/c172p.xml</path></model>
  </sim>
  <aircraft><icao><type>C172</type></icao></aircraft>
</PropertyList>
)"));
    QVERIFY(writeTextFile(Dir.filePath("Aircraft/Generic/tags.xml"), R"(<?xml version="1.0"?>
<PropertyList><tag>seaplane</tag></PropertyList>
)"));
    CFGPropertyList Props;
    QVERIFY(Props.read(SetFile));
    QCOMPARE(Props.value("sim/model/path"), QStringLiteral("Aircraft/c172p/Models/c172p.xml"));
    QCOMPARE(Props.value("aircraft/icao/type"), QStringLiteral("C172"));
    // The including file overrides the included one
    QCOMPARE(Props.value("sim/description"), QStringLiteral("Cessna 172P floats"));
    QCOMPARE(Props.values("sim/tags/tag"), QStringList({"seaplane"}));
}

void TestCore::fgPropertyListInvalid()
{
    QTemporaryDir Dir;
    QVERIFY(Dir.isValid());
    CFGPropertyList Props;
    QVERIFY(!Props.read(Dir.filePath("missing-set.xml")));
    const QString Other = Dir.filePath("other-set.xml");
    QVERIFY(writeTextFile(Other, "<?xml version=\"1.0\"?><root><a>1</a></root>"));
    QVERIFY(!Props.read(Other));
    // A file including itself stops at the maximum depth
    const QString Loop = Dir.filePath("loop-set.xml");
    QVERIFY(writeTextFile(Loop, R"(<?xml version="1.0"?>
<PropertyList include="loop-set.xml"><sim><model><path>Loop.xml</path></model></sim></PropertyList>
)"));
    QVERIFY(Props.read(Loop));
    QCOMPARE(Props.value("sim/model/path"), QStringLiteral("Loop.xml"));
    QCOMPARE(Props.values("sim/model/path").size(), CFGPropertyList::MaxIncludeDepth + 1);
}

void TestCore::fgPropertyListRealAircraft()
{
    // Only with a FlightGear installation : FG_ROOT=<...>/fgdata_2024_1
    const QString Root = qEnvironmentVariable("FG_ROOT");
    if (Root.isEmpty()) QSKIP("FG_ROOT not set");
    CFGPropertyList Props;
    QVERIFY(Props.read(Root + "/Aircraft/c172p/c172p-set.xml", {Root}));
    QCOMPARE(Props.value("sim/model/path"), QStringLiteral("Aircraft/c172p/Models/c172p.xml"));
    QCOMPARE(Props.value("aircraft/icao/type"), QStringLiteral("C172"));
    QVERIFY(!Props.value("sim/description").isEmpty());
    qInfo() << "description =" << Props.value("sim/description") << ", author =" << Props.value("sim/author")
            << ", tags =" << Props.values("sim/tags/tag");
    // c172p-main.xml : lights in generic int, doors in generic float
    const simgear::FGGenericMap Map = simgear::FGGenericMap::fromPropertyList(Props);
    QCOMPARE(Map.LandingLight, quint16(10305));
    QCOMPARE(Map.BeaconLight, quint16(10312));
    QCOMPARE(Map.StrobeLight, quint16(10313));
    QCOMPARE(Map.NavLight, quint16(10314));
    QCOMPARE(Map.Doors[0], quint16(10200));
    QCOMPARE(Map.Doors[1], quint16(10201));
    QCOMPARE(Map.Doors[2], quint16(10202));
    QCOMPARE(Map.Doors[3], quint16(10218));
    // Propeller : engine[2]/rpm, not engine[3] to [7] (gear data)
    QCOMPARE(Map.RpmRelays[0], quint16(322));
    QCOMPARE(Map.RpmRelays[1], quint16(0));
}

void TestCore::fgDatagramPosition()
{
    // Multiplayer position message as sent by FlightGear (mpmessages.hxx), XDR big endian
    QByteArray Data;
    QDataStream Out(&Data, QIODevice::WriteOnly);
    Out.setByteOrder(QDataStream::BigEndian);
    Out << quint32(simgear::MSG_MAGIC) << quint32(simgear::PROTO_VER) << quint32(7) << quint32(0) << quint32(100) << quint32(5001);
    Out.writeRawData("TEST\0\0\0\0", 8);
    QByteArray Model("Aircraft/c172p/Models/c172p.xml");
    Model.resize(96, '\0');
    Out.writeRawData(Model.constData(), Model.size());
    Out.setFloatingPointPrecision(QDataStream::DoublePrecision);
    Out << 12.5 << 0.25 << 4201000.0 << 187000.0 << 4790000.0;
    Out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (int i = 1; i <= 15; i++) Out << static_cast<float>(i) / 10.0f;
    Out << quint32(0); // pad
    QCOMPARE(Data.size(), 232);
    // A V1 property (4 bytes id) : surface-positions/elevator-pos-norm
    Out << quint16(0) << quint16(102) << 0.5f;

    QDataStream In(&Data, QIODevice::ReadOnly);
    simgear::FGDatagram Datagram;
    Datagram.Read(In);
    QVERIFY(Datagram.IsSane());
    simgear::FGPos Pos;
    QVERIFY(Datagram.GetPos(Pos));
    QCOMPARE(QString(Pos.ModelName), QStringLiteral("Aircraft/c172p/Models/c172p.xml"));
    QCOMPARE(Pos.Time, 12.5);
    QCOMPARE(Pos.Position[2], 4790000.0);
    QCOMPARE(Pos.Orientation[0], 0.1f);
    QCOMPARE(Pos.AngularAccel[2], 1.5f);
    QVERIFY(In.atEnd());
    QCOMPARE(In.status(), QDataStream::Ok);
}

void TestCore::fgOrientationRoundTrip_data()
{
    QTest::addColumn<double>("Lon");
    QTest::addColumn<double>("Lat");
    QTest::addColumn<double>("Heading");
    QTest::addColumn<double>("Pitch");
    QTest::addColumn<double>("Bank");
    QTest::newRow("LFPG level north") << 2.55 << 49.01 << 0.0 << 0.0 << 0.0;
    QTest::newRow("LFPG climbing turn") << 2.55 << 49.01 << 265.0 << -8.0 << 25.0;
    QTest::newRow("southern hemisphere") << -70.6 << -33.4 << 123.0 << 4.0 << -30.0;
    QTest::newRow("date line") << 179.5 << 10.0 << 359.0 << -15.0 << 60.0;
}

void TestCore::fgOrientationRoundTrip()
{
    QFETCH(double, Lon);
    QFETCH(double, Lat);
    QFETCH(double, Heading);
    QFETCH(double, Pitch);
    QFETCH(double, Bank);
    float AngleAxis[3];
    EulerToFGOrientation(Lon, Lat, Heading, Pitch, Bank, AngleAxis);
    double H, P, B;
    FGOrientationToEuler(Lon, Lat, AngleAxis, H, P, B);
    double DeltaH = std::fmod(H - Heading + 540.0, 360.0) - 180.0;
    QVERIFY2(std::abs(DeltaH) < 0.01, qPrintable(QString::number(H)));
    QVERIFY2(std::abs(P - Pitch) < 0.01, qPrintable(QString::number(P)));
    QVERIFY2(std::abs(B - Bank) < 0.01, qPrintable(QString::number(B)));
}

void TestCore::fgDatagramWriteRead()
{
    CAircraftState State;
    State.Latitude = 49.0097;
    State.Longitude = 2.5479;
    State.Altitude = 1500.0;
    State.Heading = 90.0;
    State.Pitch = -5.0;  // nose up
    State.Bank = -20.0;  // right wing down
    State.TASSpeed = 110.0;
    simgear::FGDatagram Out;
    Out.SetIdentity("fgp76-long-callsign", "Aircraft/c172p/Models/c172p.xml");
    QVERIFY(Out.SetAirData(State));
    QByteArray Data;
    QDataStream OutStream(&Data, QIODevice::WriteOnly);
    Out.Write(OutStream);

    QDataStream InStream(&Data, QIODevice::ReadOnly);
    simgear::FGDatagram In;
    In.Read(InStream);
    QVERIFY(In.IsSane());
    simgear::FGHeader Header;
    QVERIFY(In.GetHeader(Header));
    QCOMPARE(Header.MsgId, simgear::POS_DATA_ID);
    QCOMPARE(static_cast<qsizetype>(Header.MsgLen), Data.size());
    QCOMPARE(QString(Header.CallSign), QStringLiteral("fgp76-l"));
    CAircraftState Result;
    QVERIFY(In.GetAirData(Result));
    QCOMPARE(Result.Title, QStringLiteral("Aircraft/c172p/Models/c172p.xml"));
    QVERIFY(std::abs(Result.Latitude - State.Latitude) < 1e-6);
    QVERIFY(std::abs(Result.Longitude - State.Longitude) < 1e-6);
    QVERIFY(std::abs(Result.Altitude - State.Altitude) < 0.1);
    QVERIFY(std::abs(Result.Heading - State.Heading) < 0.01);
    QVERIFY(std::abs(Result.Pitch - State.Pitch) < 0.01);
    QVERIFY(std::abs(Result.Bank - State.Bank) < 0.01);
    QVERIFY(std::abs(Result.TASSpeed - State.TASSpeed) < 0.01);
}

void TestCore::fgPropertyListAliases()
{
    QTemporaryDir Dir;
    QVERIFY(Dir.isValid());
    // As the c172p : generic properties aliased to the aircraft properties
    const QString SetFile = Dir.filePath("Aircraft/c172p/c172p-set.xml");
    QVERIFY(writeTextFile(SetFile, R"(<?xml version="1.0"?>
<PropertyList>
  <sim>
    <multiplay>
      <generic>
        <float n="0" alias="/sim/model/door-positions/rightDoor/position-norm-effective"/>
        <float n="1" alias="/sim/model/door-positions/leftDoor/position-norm-effective"/>
        <float n="3" alias="/sim/model/door-positions/leftWindow/position-norm"/>
        <int n="4" alias="/sim/model/c172p/lighting/taxi"/>
        <int n="5" alias="/sim/model/c172p/lighting/landing"/>
        <int n="12" alias="/sim/model/c172p/lighting/beacon/enabled"/>
        <int n="13" alias="/sim/model/c172p/lighting/strobes/enabled"/>
        <bool alias="/controls/lighting/nav-lights"/>
        <bool alias="/controls/panel/glass"/>
        <string n="0" alias="/sim/model/immat"/>
      </generic>
    </multiplay>
  </sim>
  <engines><engine n="2"><rpm alias="/engines/active-engine/rpm"/></engine></engines>
</PropertyList>
)"));
    CFGPropertyList Props;
    QVERIFY(Props.read(SetFile));
    const QHash<QString, QString> Aliases = Props.aliases();
    QCOMPARE(Aliases.value("sim/multiplay/generic/float"), QStringLiteral("sim/model/door-positions/rightDoor/position-norm-effective"));
    QCOMPARE(Aliases.value("sim/multiplay/generic/int[12]"), QStringLiteral("sim/model/c172p/lighting/beacon/enabled"));
    // Without n, the rank among the elements of the same name
    QCOMPARE(Aliases.value("sim/multiplay/generic/bool"), QStringLiteral("controls/lighting/nav-lights"));
    QCOMPARE(Aliases.value("sim/multiplay/generic/bool[1]"), QStringLiteral("controls/panel/glass"));
    QCOMPARE(Aliases.value("engines/engine[2]/rpm"), QStringLiteral("engines/active-engine/rpm"));

    const simgear::FGGenericMap Map = simgear::FGGenericMap::fromPropertyList(Props);
    QCOMPARE(Map.LandingLight, quint16(10305));
    QCOMPARE(Map.BeaconLight, quint16(10312));
    QCOMPARE(Map.StrobeLight, quint16(10313));
    QCOMPARE(Map.NavLight, quint16(simgear::BOOLARRAY_BASE_1));
    QCOMPARE(Map.RecoLight, quint16(0));
    // The window is not a door
    QCOMPARE(Map.Doors[0], quint16(10200));
    QCOMPARE(Map.Doors[1], quint16(10201));
    QCOMPARE(Map.Doors[2], quint16(0));
    // engine[2]/rpm copies the engine rpm : the model reads it
    QCOMPARE(Map.RpmRelays[0], quint16(322));
    QCOMPARE(Map.RpmRelays[1], quint16(0));
    QVERIFY(!Map.isEmpty());
    QVERIFY(simgear::FGGenericMap().isEmpty());
}

namespace
{
/// Position message of FlightGear without properties : level, at 0 N 0 E
QByteArray fgPositionMessage(quint32 pPad, const float pVelocity[3] = nullptr, const float pOrientation[3] = nullptr)
{
    QByteArray Data;
    QDataStream Out(&Data, QIODevice::WriteOnly);
    Out.setByteOrder(QDataStream::BigEndian);
    Out << quint32(simgear::MSG_MAGIC) << quint32(simgear::PROTO_VER) << quint32(7) << quint32(0) << quint32(100) << quint32(5001);
    Out.writeRawData("TEST\0\0\0\0", 8);
    QByteArray Model("Aircraft/c172p/Models/c172p.xml");
    Model.resize(96, '\0');
    Out.writeRawData(Model.constData(), Model.size());
    Out.setFloatingPointPrecision(QDataStream::DoublePrecision);
    Out << 12.5 << 0.1;
    double X, Y, Z;
    GeodToCart(0.0, 0.0, 1000.0, X, Y, Z);
    Out << X << Y << Z;
    Out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (int i = 0; i < 3; i++) Out << (pOrientation ? pOrientation[i] : 0.0f);
    for (int i = 0; i < 3; i++) Out << (pVelocity ? pVelocity[i] : 0.0f);
    for (int i = 0; i < 9; i++) Out << 0.0f;
    Out << pPad;
    return Data;
}

quint32 fgShortInt(quint16 pId, qint16 pValue)
{
    return (quint32(pId) << 16) | quint16(pValue);
}
}

void TestCore::fgDatagramProperties()
{
    // As FlightGear 2024.1 sends them (captured from fgfs, c172p) : 2017.2 protocol
    QByteArray Data = fgPositionMessage(simgear::V2_PAD_MAGIC);
    QDataStream Out(&Data, QIODevice::WriteOnly | QIODevice::Append);
    Out.setByteOrder(QDataStream::BigEndian);
    Out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    Out << fgShortInt(10, 2);                                   // protocol-version
    Out << fgShortInt(100, 720);                                // left aileron 0.022 (norm)
    Out << fgShortInt(102, -16384);                             // elevator -0.5 : nose up
    Out << fgShortInt(104, 21845);                              // flaps 2/3
    Out << fgShortInt(201, 32767);                              // gear[0] down
    Out << fgShortInt(210, 5110);                               // gear[1] compressed
    Out << fgShortInt(302, 23000);                              // engine[0] 2300 rpm
    Out << quint32(904) << qint32(0);                           // V1 bool (not short encoded)
    Out << quint32(906) << 12345.0f;                            // V1 float
    Out << fgShortInt(1005, 1000);                              // nav-lights 1.0
    Out << fgShortInt(1500, 7000);                              // transponder code
    Out << fgShortInt(1503, 5);                                 // transponder altitude mode
    Out << quint32(10200) << 0.82f;                             // generic float[0] : right door
    Out << quint32(10312) << qint32(1);                         // generic int[12] : beacon
    Out << fgShortInt(1101, 5);                                 // livery : 5 characters, no padding
    Out.writeRawData("4X-CH", 5);
    Out << quint32(10100) << quint32(2);                        // V1 string : one word per character
    Out << qint32('F') << qint32('G') << qint32(0) << qint32(0);
    Out << quint32(simgear::BOOLARRAY_BASE_1) << quint32(0x5);  // bool[0] and bool[2]
    Out << fgShortInt(13000, 1);                                // fallback-model-index

    QDataStream In(&Data, QIODevice::ReadOnly);
    simgear::FGDatagram Datagram;
    Datagram.Read(In);
    QVERIFY(Datagram.IsSane());
    QVERIFY(In.atEnd());
    QCOMPARE(Datagram.PropertyValue(10), 2.0);
    QVERIFY(std::abs(Datagram.PropertyValue(100) - 720.0 / 32767.0) < 1e-6);
    QCOMPARE(Datagram.PropertyValue(906), 12345.0);
    QVERIFY(Datagram.HasProperty(904));
    QCOMPARE(Datagram.Properties().value(1101).string_value, QByteArray("4X-CH"));
    QCOMPARE(Datagram.Properties().value(10100).string_value, QByteArray("FG"));
    QCOMPARE(Datagram.PropertyValue(simgear::BOOLARRAY_BASE_1), 1.0);
    QCOMPARE(Datagram.PropertyValue(simgear::BOOLARRAY_BASE_1 + 1), 0.0);
    QCOMPARE(Datagram.PropertyValue(simgear::BOOLARRAY_BASE_1 + 2), 1.0);
    // Read after the unpadded string
    QCOMPARE(Datagram.PropertyValue(13000), 1.0);

    simgear::FGGenericMap Map;
    Map.BeaconLight = 10312;
    Map.Doors[0] = 10200;
    CAircraftState State;
    State.NumEngine = 1;
    QVERIFY(Datagram.GetAirData(State, Map));
    QVERIFY(std::abs(State.AileronPos - 0.022) < 0.001);
    QVERIFY(std::abs(State.ElevatorPos - 0.5) < 0.001);
    QCOMPARE(State.FlapsIndex, 2);
    QCOMPARE(State.GearPos, 1);
    QVERIFY(State.OnGround);
    QCOMPARE(State.StateEng1, 1);
    QCOMPARE(State.NavLight, 1);
    QCOMPARE(State.BeaconLight, 1);
    QCOMPARE(State.LandingLight, 0);
    QCOMPARE(State.Door1Pos, 82);
    QCOMPARE(State.Squawk, 7000);
    QCOMPARE(State.SquawkMode, 3);

    // An unknown property ends the reading, as in FlightGear
    QByteArray Bad = fgPositionMessage(0);
    QDataStream BadOut(&Bad, QIODevice::WriteOnly | QIODevice::Append);
    BadOut.setByteOrder(QDataStream::BigEndian);
    BadOut.setFloatingPointPrecision(QDataStream::SinglePrecision);
    BadOut << quint32(102) << 0.25f << quint32(9999) << 1.0f << quint32(103) << 0.5f;
    QDataStream BadIn(&Bad, QIODevice::ReadOnly);
    simgear::FGDatagram BadDatagram;
    BadDatagram.Read(BadIn);
    QVERIFY(BadDatagram.IsSane());
    QCOMPARE(BadDatagram.PropertyValue(102), 0.25);
    QVERIFY(!BadDatagram.HasProperty(103));
}

void TestCore::fgDatagramVelocity()
{
    // At 0 N 0 E, heading north, nose up 10 degrees, 50 m/s forward in the body frame
    float Orientation[3];
    EulerToFGOrientation(0.0, 0.0, 0.0, -10.0, 0.0, Orientation);
    const float Velocity[3] = {50.0f, 0.0f, 0.0f};
    QByteArray Data = fgPositionMessage(0, Velocity, Orientation);
    QDataStream In(&Data, QIODevice::ReadOnly);
    simgear::FGDatagram Datagram;
    Datagram.Read(In);
    CAircraftState State;
    QVERIFY(Datagram.GetAirData(State));
    QVERIFY2(std::abs(State.Pitch + 10.0) < 0.01, qPrintable(QString::number(State.Pitch)));
    const double Climb = 50.0 * std::sin(10.0 * M_PI / 180.0) * 196.8503937;
    QVERIFY2(std::abs(State.Vario - Climb) < 1.0, qPrintable(QString::number(State.Vario)));
    const double Ground = 50.0 * std::cos(10.0 * M_PI / 180.0) / SG_KT_TO_MPS;
    QVERIFY2(std::abs(State.GroundSpeed - Ground) < 0.1, qPrintable(QString::number(State.GroundSpeed)));
    QVERIFY(std::abs(State.TASSpeed - 50.0 / SG_KT_TO_MPS) < 0.1);
}

void TestCore::fgDatagramAirDataRoundTrip()
{
    CAircraftState State;
    State.Latitude = 49.0097;
    State.Longitude = 2.5479;
    State.Altitude = 2500.0;
    State.Heading = 135.0;
    State.Pitch = -3.0;
    State.Bank = 15.0;
    State.GroundSpeed = 120.0;
    State.Vario = -700.0;
    State.IASSpeed = 115.0;
    State.AileronPos = -0.3;
    State.ElevatorPos = 0.2;
    State.RudderPos = 0.1;
    State.SpoilerPos = 0.5;
    State.FlapsIndex = 1;
    State.GearPos = 1;
    State.OnGround = false;
    State.LandingLight = 1;
    State.BeaconLight = 1;
    State.StrobeLight = 0;
    State.NavLight = 1;
    State.Door1Pos = 0;
    State.Door2Pos = 100;
    State.Squawk = 4521;
    State.SquawkMode = 3;
    simgear::FGGenericMap Map;
    Map.LandingLight = 10305;
    Map.BeaconLight = 10312;
    Map.StrobeLight = 10313;
    Map.NavLight = simgear::BOOLARRAY_BASE_1 + 3;
    Map.Doors[0] = 10200;
    Map.Doors[1] = 10201;

    simgear::FGDatagram Out;
    Out.SetIdentity("F-GP76", "Aircraft/c172p/Models/c172p.xml");
    QVERIFY(Out.SetAirData(State, Map));
    QByteArray Data;
    QDataStream OutStream(&Data, QIODevice::WriteOnly);
    Out.Write(OutStream);
    QVERIFY(Data.size() > static_cast<qsizetype>(simgear::POS_MSG_LEN));

    QDataStream InStream(&Data, QIODevice::ReadOnly);
    simgear::FGDatagram In;
    In.Read(InStream);
    QVERIFY(In.IsSane());
    QVERIFY(InStream.atEnd());
    simgear::FGHeader Header;
    QVERIFY(In.GetHeader(Header));
    // FlightGear checks the length of the message
    QCOMPARE(static_cast<qsizetype>(Header.MsgLen), Data.size());
    for (auto it = Out.Properties().constBegin(); it != Out.Properties().constEnd(); ++it)
        QVERIFY2(std::abs(In.PropertyValue(it.key(), -1.0) - Out.PropertyValue(it.key())) < 1e-6, qPrintable(QString::number(it.key())));
    // FlightGear elevator positive nose down
    QVERIFY(std::abs(In.PropertyValue(simgear::FGProp::Elevator) + 0.2) < 1e-6);
    QCOMPARE(In.PropertyValue(simgear::FGProp::TransponderAltitude), 2500.0);

    CAircraftState Result;
    QVERIFY(In.GetAirData(Result, Map));
    QVERIFY(std::abs(Result.GroundSpeed - State.GroundSpeed) < 0.01);
    QVERIFY(std::abs(Result.Vario - State.Vario) < 0.5);
    QVERIFY(std::abs(Result.AileronPos - State.AileronPos) < 1e-6);
    QVERIFY(std::abs(Result.ElevatorPos - State.ElevatorPos) < 1e-6);
    QVERIFY(std::abs(Result.RudderPos - State.RudderPos) < 1e-6);
    QVERIFY(std::abs(Result.SpoilerPos - State.SpoilerPos) < 1e-6);
    QCOMPARE(Result.FlapsIndex, State.FlapsIndex);
    QCOMPARE(Result.GearPos, State.GearPos);
    QCOMPARE(Result.LandingLight, 1);
    QCOMPARE(Result.BeaconLight, 1);
    QCOMPARE(Result.StrobeLight, 0);
    QCOMPARE(Result.NavLight, 1);
    QCOMPARE(Result.Door1Pos, 0);
    QCOMPARE(Result.Door2Pos, 100);
    QCOMPARE(Result.Squawk, 4521);
    QCOMPARE(Result.SquawkMode, 3);
    QCOMPARE(Result.IASSpeed, 115.0);

    // Engines : only the engines of the model, and the rpm relays
    State.StateEng1 = 1;
    State.ThrottleEng1 = 50;
    Map.EngineCount = 1;
    Map.EngineType = 0;
    Map.RpmRelays[0] = 322;
    simgear::FGDatagram Piston;
    Piston.SetIdentity("F-GP76", "Aircraft/c172p/Models/c172p.xml");
    QVERIFY(Piston.SetAirData(State, Map));
    QCOMPARE(Piston.PropertyValue(simgear::FGProp::EngineRpm), 1550.0);
    QCOMPARE(Piston.PropertyValue(322), 1550.0);
    QVERIFY(!Piston.HasProperty(312));
    QVERIFY(!Piston.HasProperty(332));
    State.StateEng1 = 0;
    QVERIFY(Piston.SetAirData(State, Map));
    QCOMPARE(Piston.PropertyValue(322), 0.0);
    State.StateEng1 = 1;
    State.StateEng2 = 1;
    State.ThrottleEng2 = 100;
    simgear::FGGenericMap JetMap;
    JetMap.EngineCount = 2;
    JetMap.EngineType = 1;
    simgear::FGDatagram Jet;
    Jet.SetIdentity("F-GP76", "Aircraft/A320/Models/a320.xml");
    QVERIFY(Jet.SetAirData(State, JetMap));
    QCOMPARE(Jet.PropertyValue(simgear::FGProp::EngineN1), 60.0);
    QCOMPARE(Jet.PropertyValue(simgear::FGProp::EngineN1 + 10), 100.0);
    QVERIFY(!Jet.HasProperty(simgear::FGProp::EngineRpm));

    // Transponder in standby : no code, no altitude
    State.SquawkMode = 1;
    simgear::FGDatagram Standby;
    Standby.SetIdentity("F-GP76", "Aircraft/c172p/Models/c172p.xml");
    QVERIFY(Standby.SetAirData(State));
    QCOMPARE(Standby.PropertyValue(simgear::FGProp::TransponderId), double(simgear::FGProp::TransponderNone));
    QCOMPARE(Standby.PropertyValue(simgear::FGProp::TransponderAltitude), double(simgear::FGProp::TransponderNone));
}

QTEST_GUILESS_MAIN(TestCore)
#include "tst_core.moc"
