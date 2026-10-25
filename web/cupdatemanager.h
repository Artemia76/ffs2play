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
** cupdatemanager.h is part of FF2Play project
**
** Updates of FFS2Play and of the xffs2play X-Plane plugin, from the packages
** published on GitHub and chosen by the portal (suite contract update-api.md)
****************************************************************************/

#ifndef CUPDATEMANAGER_H
#define CUPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QSettings>
#include <functional>

#include "tools/clogger.h"
#include "web/updateinfo.h"

class CUpdateManager : public QObject
{
    Q_OBJECT
private:
                        CUpdateManager(QObject* pParent = nullptr);
    static CUpdateManager* createInstance();
    QSettings           m_settings;
    CLogger&            m_log = CLogger::instance();
    QNetworkAccessManager m_network;
    bool                m_busy;

    QString             portalUrl();
    QString             downloadDir() const;
    bool                isSkipped(const SUpdateInfo& pInfo);
    void                request(const QUrlQuery& pQuery, std::function<void(const SUpdateInfo&)> pCallback);
    void                offerClient(const SUpdateInfo& pInfo, const QString& pInstalled);
    void                offerPlugin(const SUpdateInfo& pInfo, const QString& pInstalled);
    void                installClient(const QString& pFile, const SUpdateAsset& pAsset);
    void                installLinuxPackage(const QString& pFile, const QString& pFormat);
    void                openFile(const QString& pFile);

public:
                        ~CUpdateManager() override;
    static CUpdateManager& instance();
    /// Asks the portal for a new FFS2Play (start, change of the beta channel)
    void                checkClient();
    /// Asks the portal for a new xffs2play (plugin connected)
    void                checkPlugin(const QString& pPluginVersion);
    /// Installs the downloaded plugin once X-Plane is closed (start, X-Plane disconnected)
    void                installPendingPlugin();

signals:
    /// FFS2Play must quit : the installer replaces it
    void                fireExit();
};

#endif // CUPDATEMANAGER_H
