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
** updateinfo.cpp is part of FF2Play project
****************************************************************************/

#include "web/updateinfo.h"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTextStream>

#ifndef FFS2PLAY_PACKAGE_FORMAT
#define FFS2PLAY_PACKAGE_FORMAT ""
#endif

namespace
{
SUpdateAsset assetFromJson(const QJsonObject& pObject)
{
    SUpdateAsset Asset;
    Asset.Name = pObject.value("name").toString();
    Asset.Url = pObject.value("url").toString();
    Asset.Sha256 = pObject.value("sha256").toString().toLower();
    Asset.Format = pObject.value("format").toString();
    Asset.Size = pObject.value("size").toInteger();
    return Asset;
}
}

///
/// \brief UpdateInfo::parse
/// \param pJson answer of /api/update
/// \param pInfo filled on success
/// \return false if the answer is not a valid update answer (error, 503...)
///
bool UpdateInfo::parse(const QByteArray& pJson, SUpdateInfo& pInfo)
{
    const QJsonDocument Doc = QJsonDocument::fromJson(pJson);
    if (!Doc.isObject()) return false;
    const QJsonObject Root = Doc.object();
    if (Root.contains("error") || !Root.value("update_available").isBool()) return false;
    pInfo = SUpdateInfo();
    pInfo.App = Root.value("app").toString();
    pInfo.Available = Root.value("update_available").toBool();
    pInfo.Mandatory = Root.value("mandatory").toBool();
    pInfo.Reason = Root.value("reason").toString();
    const QJsonObject Latest = Root.value("latest").toObject();
    pInfo.Version = Latest.value("version").toString();
    pInfo.Prerelease = Latest.value("prerelease").toBool();
    pInfo.Notes = Latest.value("notes").toString();
    pInfo.PageUrl = Latest.value("page_url").toString();
    pInfo.RequiredClient = Latest.value("requires").toObject().value("ffs2play").toString();
    if (Root.value("asset").isObject())
    {
        pInfo.Asset = assetFromJson(Root.value("asset").toObject());
        pInfo.HasAsset = !pInfo.Asset.Url.isEmpty();
    }
    for (const QJsonValue& Value : Root.value("alternatives").toArray())
    {
        pInfo.Alternatives << assetFromJson(Value.toObject());
    }
    // available without a version is not usable
    if (pInfo.Available && pInfo.Version.isEmpty()) pInfo.Available = false;
    return true;
}

///
/// \brief UpdateInfo::readOsRelease
/// \param pPath
/// \return ID, VERSION_ID and ID_LIKE (values without quotes)
///
SOsRelease UpdateInfo::readOsRelease(const QString& pPath)
{
    SOsRelease Release;
    QFile File(pPath);
    if (!File.open(QIODevice::ReadOnly | QIODevice::Text)) return Release;
    QTextStream Stream(&File);
    while (!Stream.atEnd())
    {
        const QString Line = Stream.readLine().trimmed();
        const qsizetype Equal = Line.indexOf('=');
        if (Line.startsWith('#') || Equal <= 0) continue;
        const QString Key = Line.left(Equal);
        QString Value = Line.mid(Equal + 1).trimmed();
        if (Value.size() >= 2 && (Value.front() == '"' || Value.front() == '\'') && Value.back() == Value.front())
            Value = Value.mid(1, Value.size() - 2);
        if (Key == "ID") Release.Id = Value.toLower();
        else if (Key == "VERSION_ID") Release.VersionId = Value;
        else if (Key == "ID_LIKE") Release.IdLike = Value.toLower();
    }
    return Release;
}

///
/// \brief UpdateInfo::packageFormat
/// \return appimage, the format given at build time (FFS2PLAY_PACKAGE_FORMAT), or empty for a development build
///
QString UpdateInfo::packageFormat()
{
#ifdef Q_OS_LINUX
    if (!qEnvironmentVariable("APPIMAGE").isEmpty()) return "appimage";
#endif
    return QString(FFS2PLAY_PACKAGE_FORMAT);
}

QString UpdateInfo::osName()
{
#if defined(Q_OS_WIN)
    return "windows";
#elif defined(Q_OS_MACOS)
    return "macos";
#else
    return "linux";
#endif
}

QString UpdateInfo::archName()
{
    const QString Arch = QSysInfo::currentCpuArchitecture();
    if (Arch == "x86_64") return Arch;
    if (Arch == "arm64" || Arch == "aarch64") return "arm64";
    return QString();
}

///
/// \brief UpdateInfo::query
/// \param pApp ffs2play or xffs2play
/// \param pVersion installed version of pApp (M.m.p)
/// \param pBeta beta channel
/// \param pClientVersion version of this client, sent with the xffs2play requests
///
QUrlQuery UpdateInfo::query(const QString& pApp, const QString& pVersion, bool pBeta, const QString& pClientVersion)
{
    QUrlQuery Query;
    Query.addQueryItem("app", pApp);
    Query.addQueryItem("version", pVersion);
    Query.addQueryItem("channel", pBeta ? "beta" : "stable");
    Query.addQueryItem("os", osName());
    const QString Arch = archName();
    if (!Arch.isEmpty()) Query.addQueryItem("arch", Arch);
    if (!pClientVersion.isEmpty())
    {
        Query.addQueryItem("client_version", pClientVersion);
        return Query;
    }
    Query.addQueryItem("format", packageFormat());
#ifdef Q_OS_LINUX
    const SOsRelease Release = readOsRelease();
    Query.addQueryItem("distro", Release.Id);
    Query.addQueryItem("distro_version", Release.VersionId);
    Query.addQueryItem("distro_like", Release.IdLike);
#endif
    return Query;
}

///
/// \brief UpdateInfo::installCommand
/// \param pFormat deb, rpm or arch
/// \param pFile downloaded package
/// \param pHasZypper openSUSE : zypper instead of dnf
/// \return program and arguments, to be run by pkexec
///
QStringList UpdateInfo::installCommand(const QString& pFormat, const QString& pFile, bool pHasZypper)
{
    if (pFormat == "deb") return {"apt-get", "install", "-y", pFile};
    if (pFormat == "rpm")
    {
        // packages are not signed : zypper refuses them without --allow-unsigned-rpm
        if (pHasZypper) return {"zypper", "--non-interactive", "install", "--allow-unsigned-rpm", pFile};
        return {"dnf", "install", "-y", pFile};
    }
    if (pFormat == "arch") return {"pacman", "-U", "--noconfirm", pFile};
    return {};
}

bool UpdateInfo::checkSha256(const QString& pFile, const QString& pSha256)
{
    QFile File(pFile);
    if (pSha256.size() != 64 || !File.open(QIODevice::ReadOnly)) return false;
    QCryptographicHash Hash(QCryptographicHash::Sha256);
    if (!Hash.addData(&File)) return false;
    return Hash.result().toHex() == pSha256.toLower().toLatin1();
}

bool UpdateInfo::isNewer(const QString& pRemote, const QString& pLocal)
{
    return QVersionNumber::compare(QVersionNumber::fromString(pRemote), QVersionNumber::fromString(pLocal)) > 0;
}
