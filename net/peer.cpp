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

#include "peer.h"

#ifdef Q_OS_WIN
	#include <windows.h>
#endif

namespace ffs2play
{
	CPeer::CPeer(
			UDPServer* pServer,
			QHostAddress pExternalIP,
			int pPort,
			QString CallSign,
			bool pDisabled,
			bool pLocal,
			const QList<QHostAddress>* pInternalIP,
			QObject *parent
			) : QObject(parent),
				CallSign (m_CallSign)
	{
		m_log = CLogger::instance();
		m_AIMapping = AIMapping::instance();
		m_simManager = simbasemanager::instance();
		m_CallSign = CallSign;
		m_server = pServer;
		m_ExternalIP = pExternalIP;
		if (pInternalIP != NULL)
		{
			m_InternalIP = *pInternalIP;
		}
		m_Port = pPort;
		m_bLocal = pLocal;
		m_bDisabled = true;
		m_sel_iplocal = 0;
		m_TrySimpleAI = false;
		m_bDisabled = pDisabled;
		m_Visible = false;
		m_bSpawnable = false;
	}

	CPeer::~CPeer()
	{
	}
}
