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
** pluginpackage.h is part of FF2Play project
**
** Installs the xffs2play plugin package (xffs2play_M_m_p.zip, folder xffs2play/)
** in the Resources/plugins folder of an X-Plane installation
****************************************************************************/

#ifndef PLUGINPACKAGE_H
#define PLUGINPACKAGE_H

#include <QString>

namespace PluginPackage
{
    /// Plugin folder name in Resources/plugins and at the root of the zip
    inline constexpr const char* FolderName = "xffs2play";

    ///
    /// Extracts the zip in <pPluginsDir>/xffs2play.new, then swaps it with <pPluginsDir>/xffs2play
    /// (the previous plugin is kept as xffs2play.old until the swap succeeds, and restored on failure).
    /// Fails without touching the installed plugin if X-Plane still holds its files (Windows).
    ///
    bool install(const QString& pZip, const QString& pPluginsDir, QString& pError);
}

#endif // PLUGINPACKAGE_H
