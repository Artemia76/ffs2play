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
** cupdatemanager.cpp is part of FF2Play project
****************************************************************************/

#include "web/cupdatemanager.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProcess>
#include <QStandardPaths>
#include <cstdio>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

#include "disk/pluginpackage.h"
#include "disk/xpcslparser.h"
#include "gui/dialogupdate.h"
#include "sim/simbasemanager.h"
#include "tools/getappversion.h"
#include "tools/singleton.h"

namespace
{
QString clientVersion()
{
    const sVersion Version = GetAppVersion();
    return QString("%1.%2.%3").arg(Version.Major).arg(Version.Minor).arg(Version.Build);
}

///
/// \brief startInstaller
/// The Inno Setup installer asks for administrator rights : CreateProcess (QProcess) refuses it,
/// ShellExecute shows the UAC prompt
///
bool startInstaller(const QString& pFile, const QStringList& pArguments)
{
#ifdef Q_OS_WIN
    const std::wstring File = QDir::toNativeSeparators(pFile).toStdWString();
    const std::wstring Arguments = pArguments.join(' ').toStdWString();
    return reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", File.c_str(), Arguments.c_str(), nullptr, SW_SHOWNORMAL)) > 32;
#else
    return QProcess::startDetached(pFile, pArguments);
#endif
}
}

///
/// \brief CUpdateManager::CUpdateManager
/// Removes the packages of the previous updates, except the plugin waiting for X-Plane to close
///
CUpdateManager::CUpdateManager(QObject* pParent) : QObject(pParent)
{
    m_busy = false;
    const QString Pending = QFileInfo(m_settings.value("Update/PendingPlugin").toString()).fileName();
    QDir Updates(downloadDir());
    for (const QString& File : Updates.entryList(QDir::Files))
    {
        if (File != Pending) Updates.remove(File);
    }
}

CUpdateManager::~CUpdateManager() = default;

CUpdateManager* CUpdateManager::createInstance()
{
    return new CUpdateManager();
}

CUpdateManager& CUpdateManager::instance()
{
    return Singleton<CUpdateManager>::instance(CUpdateManager::createInstance);
}

///
/// \brief CUpdateManager::portalUrl
/// \return portal answering the update requests (setting Update/URL, https://ffs2play.fr by default)
///
QString CUpdateManager::portalUrl()
{
    QString Url = m_settings.value("Update/URL", "https://ffs2play.fr").toString().trimmed();
    while (Url.endsWith('/')) Url.chop(1);
    return Url;
}

QString CUpdateManager::downloadDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/updates";
}

bool CUpdateManager::isSkipped(const SUpdateInfo& pInfo)
{
    return !pInfo.Mandatory && m_settings.value("Update/Skipped/" + pInfo.App).toString() == pInfo.Version;
}

///
/// \brief CUpdateManager::request
/// GET /index.php/api/update ; no answer, 503 or invalid answer : nothing is shown to the player
///
void CUpdateManager::request(const QUrlQuery& pQuery, std::function<void(const SUpdateInfo&)> pCallback)
{
    QUrl Url(portalUrl() + "/index.php/api/update");
    Url.setQuery(pQuery);
    QNetworkRequest Request(Url);
    Request.setRawHeader("User-Agent", "FFS2Play");
    Request.setTransferTimeout(15000);
    QNetworkReply* Reply = m_network.get(Request);
    connect(Reply, &QNetworkReply::finished, this, [this, Reply, pCallback]()
    {
        Reply->deleteLater();
        SUpdateInfo Info;
        if (Reply->error() != QNetworkReply::NoError || !UpdateInfo::parse(Reply->readAll(), Info))
        {
            m_log.log("UpdateManager : " + tr("Update check failed : ") + Reply->errorString(), Qt::darkBlue, LEVEL_NORMAL);
            return;
        }
        pCallback(Info);
    });
}

void CUpdateManager::checkClient()
{
    const QString Installed = clientVersion();
    const bool Beta = m_settings.value("Main/Beta", false).toBool();
    request(UpdateInfo::query("ffs2play", Installed, Beta), [this, Installed](const SUpdateInfo& pInfo)
    {
        offerClient(pInfo, Installed);
    });
}

void CUpdateManager::checkPlugin(const QString& pPluginVersion)
{
    const bool Beta = m_settings.value("Main/Beta", false).toBool();
    request(UpdateInfo::query("xffs2play", pPluginVersion, Beta, clientVersion()), [this, pPluginVersion](const SUpdateInfo& pInfo)
    {
        offerPlugin(pInfo, pPluginVersion);
    });
}

///
/// \brief CUpdateManager::offerClient
/// Proposes the new version ; the downloaded package is installed according to its format
///
void CUpdateManager::offerClient(const SUpdateInfo& pInfo, const QString& pInstalled)
{
    if (!pInfo.Available || isSkipped(pInfo) || m_busy) return;
    m_log.log("UpdateManager : " + tr("FFS2Play %1 is available").arg(pInfo.Version), Qt::darkBlue);
    QString Message = tr("FFS2Play %1 is available (installed version : %2).").arg(pInfo.Version, pInstalled);
    if (pInfo.Prerelease) Message += " " + tr("This is a beta version.");
    if (pInfo.Mandatory) Message += "\n" + tr("Your version is no longer supported : please update it.");
    if (!pInfo.HasAsset && !pInfo.Alternatives.isEmpty())
        Message += "\n" + tr("No package matches your system : the AppImage version, on the download page, works on every Linux distribution.");
    else if (pInfo.HasAsset && pInfo.Asset.Sha256.isEmpty())
        Message += "\n" + tr("This package can not be checked : download it from the release page.");
    QString Destination = downloadDir();
    if (pInfo.Asset.Format == "appimage")
    {
        // same folder as the AppImage : replaced by a rename
        const QString Folder = QFileInfo(qEnvironmentVariable("APPIMAGE")).absolutePath();
        if (QFileInfo(Folder).isWritable()) Destination = Folder;
    }
    m_busy = true;
    DialogUpdate Dialog(pInfo, tr("FFS2Play Update"), Message, Destination);
    Dialog.exec();
    m_busy = false;
    switch (Dialog.choice())
    {
    case DialogUpdate::Choice::Skip:
        m_settings.setValue("Update/Skipped/" + pInfo.App, pInfo.Version);
        break;
    case DialogUpdate::Choice::Downloaded:
        installClient(Dialog.downloadedFile(), pInfo.Asset);
        break;
    default:
        break;
    }
}

///
/// \brief CUpdateManager::installClient
/// \param pFile downloaded package, checksum verified
///
void CUpdateManager::installClient(const QString& pFile, const SUpdateAsset& pAsset)
{
    m_log.log("UpdateManager : " + tr("Installing ") + pFile, Qt::darkBlue);
    if (pAsset.Format == "inno")
    {
        // the installer closes FFS2Play if needed, then starts the new version
        if (startInstaller(pFile, {"/SILENT", "/SP-", "/NOCANCEL", "/NORESTART", "/CLOSEAPPLICATIONS"}))
        {
            emit fireExit();
            return;
        }
    }
    else if (pAsset.Format == "dmg")
    {
        openFile(pFile);
        QMessageBox::information(nullptr, tr("FFS2Play Update"),
            tr("Drag FFS2Play into the Applications folder to replace the current version, then start it again. FFS2Play will now quit."));
        emit fireExit();
        return;
    }
    else if (pAsset.Format == "appimage")
    {
        const QString Target = qEnvironmentVariable("APPIMAGE");
        QFile::setPermissions(pFile, QFile::permissions(pFile) | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
        // rename() replaces the running AppImage atomically (same folder)
        bool Replaced = std::rename(QFile::encodeName(pFile).constData(), QFile::encodeName(Target).constData()) == 0;
        if (!Replaced && QFile::remove(Target)) Replaced = QFile::copy(pFile, Target) && QFile::remove(pFile);
        if (Replaced)
        {
            QFile::setPermissions(Target, QFile::permissions(Target) | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther);
            QProcess::startDetached(Target, {});
            emit fireExit();
            return;
        }
        m_log.log("UpdateManager : " + tr("Can not replace ") + Target, Qt::darkRed);
    }
    else if (pAsset.Format == "deb" || pAsset.Format == "rpm" || pAsset.Format == "arch")
    {
        installLinuxPackage(pFile, pAsset.Format);
        return;
    }
    QMessageBox::warning(nullptr, tr("FFS2Play Update"), tr("The update could not be started. The package is here :\n") + pFile);
    openFile(QFileInfo(pFile).absolutePath());
}

///
/// \brief CUpdateManager::installLinuxPackage
/// Installs the package with the distribution package manager through pkexec (administrator password),
/// or hands it to the software center
///
void CUpdateManager::installLinuxPackage(const QString& pFile, const QString& pFormat)
{
    const QString Pkexec = QStandardPaths::findExecutable("pkexec");
    QStringList Command = UpdateInfo::installCommand(pFormat, pFile, !QStandardPaths::findExecutable("zypper").isEmpty());
    const QString Program = Command.isEmpty() ? QString() : QStandardPaths::findExecutable(Command.first());
    if (Pkexec.isEmpty() || Program.isEmpty())
    {
        openFile(pFile);
        return;
    }
    Command.first() = Program;
    auto* Process = new QProcess(this);
    Process->setProcessChannelMode(QProcess::MergedChannels);
    connect(Process, &QProcess::finished, this, [this, Process, pFile](int pExitCode, QProcess::ExitStatus pStatus)
    {
        Process->deleteLater();
        const QString Output = QString::fromLocal8Bit(Process->readAll()).trimmed();
        if (pStatus == QProcess::NormalExit && pExitCode == 0)
        {
            QFile::remove(pFile);
            if (QMessageBox::question(nullptr, tr("FFS2Play Update"), tr("The update is installed. Restart FFS2Play now ?"),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
            {
                QProcess::startDetached(QCoreApplication::applicationFilePath(), {});
                emit fireExit();
            }
            return;
        }
        m_log.log("UpdateManager : " + tr("Package installation failed : ") + Output.right(500), Qt::darkRed);
        // 126 / 127 : authentication refused or cancelled
        if (pExitCode == 126 || pExitCode == 127)
        {
            openFile(pFile);
            return;
        }
        QMessageBox::warning(nullptr, tr("FFS2Play Update"), tr("The package could not be installed :\n") + Output.right(500)
                             + "\n\n" + tr("The package is here :\n") + pFile);
    });
    Process->start(Pkexec, Command);
}

void CUpdateManager::openFile(const QString& pFile)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(pFile));
}

///
/// \brief CUpdateManager::offerPlugin
/// The plugin is downloaded now and installed once X-Plane is closed (installPendingPlugin)
///
void CUpdateManager::offerPlugin(const SUpdateInfo& pInfo, const QString& pInstalled)
{
    if (!pInfo.Available || isSkipped(pInfo) || m_busy) return;
    if (pInfo.Reason == "client_too_old")
    {
        m_log.log("UpdateManager : " + tr("xffs2play %1 needs FFS2Play %2 or newer : update FFS2Play first")
                  .arg(pInfo.Version, pInfo.RequiredClient), Qt::darkMagenta);
        return;
    }
    QString Message = tr("The X-Plane plugin xffs2play %1 is available (installed version : %2).").arg(pInfo.Version, pInstalled);
    if (pInfo.Prerelease) Message += " " + tr("This is a beta version.");
    Message += "\n" + tr("FFS2Play installs it in X-Plane as soon as X-Plane is closed.");
    m_busy = true;
    DialogUpdate Dialog(pInfo, tr("XFFS2Play Plugin Update"), Message, downloadDir());
    Dialog.exec();
    m_busy = false;
    if (Dialog.choice() == DialogUpdate::Choice::Skip)
    {
        m_settings.setValue("Update/Skipped/" + pInfo.App, pInfo.Version);
        return;
    }
    if (Dialog.choice() != DialogUpdate::Choice::Downloaded) return;
    m_settings.setValue("Update/PendingPlugin", Dialog.downloadedFile());
    simbasemanager& Sim = simbasemanager::instance();
    if (Sim.getType() == SIM_TYPE::XPLANE && Sim.isConnected())
    {
        QMessageBox::information(nullptr, tr("XFFS2Play Plugin Update"),
            tr("Close X-Plane : the new plugin will be installed when X-Plane is closed, and loaded at its next start."));
        return;
    }
    installPendingPlugin();
}

///
/// \brief CUpdateManager::installPendingPlugin
/// Installs the downloaded plugin in every X-Plane installation that contains xffs2play.
/// Kept for later while X-Plane is connected or holds the plugin files.
///
void CUpdateManager::installPendingPlugin()
{
    const QString Zip = m_settings.value("Update/PendingPlugin").toString();
    if (Zip.isEmpty()) return;
    if (!QFile::exists(Zip))
    {
        m_settings.remove("Update/PendingPlugin");
        return;
    }
    simbasemanager& Sim = simbasemanager::instance();
    if (Sim.getType() == SIM_TYPE::XPLANE && Sim.isConnected()) return;

    QStringList Targets;
    for (const QString& Root : XPCSLParser::findXPlaneRoots())
    {
        if (QFileInfo::exists(Root + "/Resources/plugins/" + PluginPackage::FolderName)) Targets << Root + "/Resources/plugins";
    }
    if (Targets.isEmpty())
    {
        const QString Root = QFileDialog::getExistingDirectory(nullptr, tr("X-Plane folder where xffs2play is installed"));
        if (Root.isEmpty() || !QFileInfo::exists(Root + "/Resources/plugins"))
        {
            m_settings.remove("Update/PendingPlugin");
            QMessageBox::warning(nullptr, tr("XFFS2Play Plugin Update"),
                tr("No X-Plane folder : unzip the plugin yourself in X-Plane/Resources/plugins. The package is here :\n") + Zip);
            openFile(QFileInfo(Zip).absolutePath());
            return;
        }
        Targets << Root + "/Resources/plugins";
    }

    QStringList Done;
    QStringList Errors;
    for (const QString& Plugins : std::as_const(Targets))
    {
        QString Error;
        if (PluginPackage::install(Zip, Plugins, Error)) Done << QDir::toNativeSeparators(Plugins);
        else Errors << QDir::toNativeSeparators(Plugins) + " : " + Error;
    }
    for (const QString& Folder : std::as_const(Done))
        m_log.log("UpdateManager : " + tr("xffs2play plugin installed in ") + Folder, Qt::darkGreen);
    for (const QString& Error : std::as_const(Errors))
        m_log.log("UpdateManager : " + Error, Qt::darkRed);
    if (!Errors.isEmpty())
    {
        // tried again at the next start or when X-Plane disconnects
        QMessageBox::warning(nullptr, tr("XFFS2Play Plugin Update"),
            tr("The plugin could not be installed everywhere :\n") + Errors.join("\n")
            + "\n\n" + tr("FFS2Play will try again when X-Plane is closed."));
        return;
    }
    m_settings.remove("Update/PendingPlugin");
    QFile::remove(Zip);
    QMessageBox::information(nullptr, tr("XFFS2Play Plugin Update"),
        tr("The xffs2play plugin is updated in :\n") + Done.join("\n"));
}
