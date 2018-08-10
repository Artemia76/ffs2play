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

#ifndef FGMANAGER_H
#define FGMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QTimer>

#include "sim/simconnector.h"

namespace ffs2play
{
	///
	/// \brief The CFGManager class
	///

	class CFGManager : public SimConnector
	{
		Q_OBJECT
	public:
		explicit CFGManager(QObject *parent = 0);
		~CFGManager();
	private:
		QTimer			m_PingTimer;
		datetime		m_LastPong;

	protected:
		void			open();
		void			close();
		void			main_loop();
		SIM_VERSION		getVersion();
		void			sendScrollingText(const QString&);
		bool			createAI(AIResol&, const QString&, const CAircraftState&, bool);
		bool			removeAI(uint&);
		bool			updateAI(uint, const CAircraftState&);
		void			freezeAI(uint, bool);

	private slots:
		void			OnPingTimer();
	};
}
#endif // FGMANAGER_H
