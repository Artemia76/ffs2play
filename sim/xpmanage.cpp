/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#include "xpmanager.h"
#include "singleton/singleton.h"

#include <QCoreApplication>

namespace ffs2p_xplane
{
    ///
    /// \brief CXPManager::CXPManager
    /// \param parent
    ///

    CXPManager::CXPManager(QObject* parent):
        SimConnector(parent)
    {
        // Initialisation de l'état de connexion avec le simulateur
    }

    CXPManager::~CXPManager()
    {
        close();
    }

    ///
    /// \brief CXPManager::open
    ///

    bool CXPManager::open ()
    {
        if (!m_bConnected)
        {
           Log->log(tr("XPlane is not yet implemented."),Qt::darkMagenta);
        }
        return false;
    }

    ///
    /// \brief CXPManager::close
    ///

    void CXPManager::close ()
    {
        if (m_bConnected)
        {
            Log->log(tr("Disconnected from XPlane."));
            emit(closed());
        }
    }


    ///
    /// \brief CXPManager::main_loop()
    ///

    void CXPManager::main_loop()
    {
    }
}
