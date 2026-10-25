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
** pluginpackage.cpp is part of FF2Play project
****************************************************************************/

#include "disk/pluginpackage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

// Qt private zip reader : QtCore since Qt 6.9, QtGui before
#if __has_include(<QtCore/private/qzipreader_p.h>)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

namespace
{
QString tr(const char* pText)
{
    return QCoreApplication::translate("PluginPackage", pText);
}

///
/// \brief extract
/// Writes the entries under xffs2play/ of the zip in pTarget ; refuses absolute paths and '..'
///
bool extract(QZipReader& pZip, const QString& pTarget, QString& pError)
{
    const QString Prefix = QString(PluginPackage::FolderName) + "/";
    int Files = 0;
    for (const QZipReader::FileInfo& Entry : pZip.fileInfoList())
    {
        const QString Path = QDir::fromNativeSeparators(Entry.filePath);
        if (!Path.startsWith(Prefix, Qt::CaseInsensitive)) continue;
        const QString Relative = QDir::cleanPath(Path.mid(Prefix.size()));
        if (Relative.isEmpty() || Relative == ".") continue;
        if (QDir::isAbsolutePath(Relative) || Relative == ".." || Relative.startsWith("../") || Relative.contains(':'))
        {
            pError = QCoreApplication::translate("PluginPackage", "Invalid path in the plugin package : ") + Path;
            return false;
        }
        const QString Destination = pTarget + "/" + Relative;
        if (Entry.isDir)
        {
            if (!QDir().mkpath(Destination))
            {
                pError = QCoreApplication::translate("PluginPackage", "Can not create the folder ") + Destination;
                return false;
            }
            continue;
        }
        if (!Entry.isFile) continue;
        QDir().mkpath(QFileInfo(Destination).absolutePath());
        QFile File(Destination);
        if (!File.open(QIODevice::WriteOnly) || File.write(pZip.fileData(Entry.filePath)) != Entry.size)
        {
            pError = QCoreApplication::translate("PluginPackage", "Can not write the file ") + Destination;
            return false;
        }
        Files++;
    }
    if (Files == 0)
    {
        pError = QCoreApplication::translate("PluginPackage", "The package does not contain the xffs2play folder");
        return false;
    }
    return true;
}
}

///
/// \brief PluginPackage::install
/// \param pZip plugin package
/// \param pPluginsDir X-Plane Resources/plugins folder
/// \param pError reason of the failure
/// \return true when <pPluginsDir>/xffs2play is the new plugin
///
bool PluginPackage::install(const QString& pZip, const QString& pPluginsDir, QString& pError)
{
    QZipReader Zip(pZip);
    if (!Zip.isReadable() || Zip.status() != QZipReader::NoError)
    {
        pError = tr("Can not read the plugin package ") + pZip;
        return false;
    }
    const QString Current = pPluginsDir + "/" + FolderName;
    const QString New = Current + ".new";
    const QString Old = Current + ".old";

    QDir(New).removeRecursively();
    if (!extract(Zip, New, pError))
    {
        QDir(New).removeRecursively();
        return false;
    }
    // remains of an interrupted update
    if (QFileInfo::exists(Old) && !QDir(Old).removeRecursively())
    {
        QDir(New).removeRecursively();
        pError = tr("Can not remove the folder ") + Old;
        return false;
    }
    QDir Plugins(pPluginsDir);
    const bool HadPlugin = QFileInfo::exists(Current);
    // X-Plane holds the loaded plugin on Windows : the rename fails and nothing changes
    if (HadPlugin && !Plugins.rename(FolderName, QString(FolderName) + ".old"))
    {
        QDir(New).removeRecursively();
        pError = tr("The plugin is in use : close X-Plane and try again");
        return false;
    }
    if (!Plugins.rename(QString(FolderName) + ".new", FolderName))
    {
        if (HadPlugin) Plugins.rename(QString(FolderName) + ".old", FolderName);
        QDir(New).removeRecursively();
        pError = tr("Can not install the new plugin in ") + pPluginsDir;
        return false;
    }
    if (HadPlugin) QDir(Old).removeRecursively();
    return true;
}
