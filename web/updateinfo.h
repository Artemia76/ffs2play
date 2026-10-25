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
** updateinfo.h is part of FF2Play project
**
** Update API of the portal (suite contract docs/contracts/update-api.md) :
** request parameters, response parsing, platform detection and checksums.
** No GUI and no network here : unit tested by tst_core.
****************************************************************************/

#ifndef UPDATEINFO_H
#define UPDATEINFO_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrlQuery>
#include <QVersionNumber>

struct SUpdateAsset
{
    QString Name;
    QString Url;
    QString Sha256;     ///< empty : no checksum, never installed automatically
    QString Format;     ///< inno, dmg, appimage, deb, rpm, arch or zip
    qint64  Size = 0;
};

struct SUpdateInfo
{
    QString             App;
    bool                Available = false;
    bool                Mandatory = false;
    bool                Prerelease = false;
    QString             Version;
    QString             Notes;
    QString             PageUrl;
    QString             RequiredClient;     ///< latest.requires.ffs2play
    QString             Reason;             ///< no_matching_package, client_too_old
    bool                HasAsset = false;
    SUpdateAsset        Asset;
    QList<SUpdateAsset> Alternatives;
};

/// Fields of /etc/os-release used to choose the Linux package
struct SOsRelease
{
    QString Id;
    QString VersionId;
    QString IdLike;
};

namespace UpdateInfo
{
    /// Parses the JSON answer of /api/update ; false if it is not a valid answer
    bool            parse(const QByteArray& pJson, SUpdateInfo& pInfo);
    /// ID, VERSION_ID and ID_LIKE of an os-release file
    SOsRelease      readOsRelease(const QString& pPath = "/etc/os-release");
    /// How this copy was installed : appimage (from $APPIMAGE), else the format given at build time
    QString         packageFormat();
    /// windows, macos or linux
    QString         osName();
    /// x86_64 or arm64 ; empty for other architectures (the portal assumes x86_64)
    QString         archName();
    /// Query of /api/update for ffs2play, or for xffs2play when pClientVersion is set
    QUrlQuery       query(const QString& pApp, const QString& pVersion, bool pBeta, const QString& pClientVersion = QString());
    /// Program and arguments that install a native Linux package (run through pkexec) ; empty if unknown
    QStringList     installCommand(const QString& pFormat, const QString& pFile, bool pHasZypper);
    /// true if the SHA-256 of the file is pSha256 (hexadecimal)
    bool            checkSha256(const QString& pFile, const QString& pSha256);
    /// true if pRemote is newer than pLocal (M.m.p)
    bool            isNewer(const QString& pRemote, const QString& pLocal);
}

#endif // UPDATEINFO_H
