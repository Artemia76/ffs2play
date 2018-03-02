/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulator team.
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
 * dialogp2p.cpp is part of FF2Play project
 *
 * This class purpose a dialog interface to manage account profils
 * to connect severals FFS2Play networks servers
 * **************************************************************************/

#include "dialogp2p.h"
#include "ui_dialogp2p.h"

#include <QMessageBox>

namespace ffs2play
{

	///
	/// \brief DialogBox constructor
	/// \param parent
	///
	/// Constructor read users list previously loaded by webmanager constructor
	/// Then it sync the List Widget with profil name as key
	///

	DialogP2P::DialogP2P(QWidget *parent) :
		QDialog(parent),
		ui(new Ui::DialogP2P)
	{
		m_changed = false;
		// Init GUI Part
		ui->setupUi(this);

		// Get Logger instance
		m_log = CLogger::instance();

		// Get P2PManager instance
		m_P2PManager = CP2PManager::instance();

		// Get the default profil selected in configuration file
		m_settings.beginGroup("P2P");

		m_settings.endGroup();

		// Apply button is disabled at start
		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
	}

	///
	/// \brief DialogP2P::~DialogP2P
	///
	/// Destructor
	///

	DialogP2P::~DialogP2P()
	{
		delete ui;
	}

	///
	/// \brief DialogP2P::applyChangedItem
	///

	void DialogP2P::applyChangedItem()
	{
	}

	///
	/// \brief DialogP2P::on_buttonBox_clicked
	/// \param button
	///

	void DialogP2P::on_buttonBox_clicked(QAbstractButton *button)
	{
		if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
		{
		   applyChangedItem();
		   checkIfChanged();
		}
		if (button == ui->buttonBox->button(QDialogButtonBox::Ok))
		{
			if (m_changed)
			{
				int ret = QMessageBox::warning(this,tr("Warning"),
											   tr("There is some modifications on selected profil. Do you want apply them?"),
											   QMessageBox::Yes |QMessageBox::No);
				if (ret == QMessageBox::Yes)
				{
					applyChangedItem();
				}
			}
			this->close();
		}
		if (button == ui->buttonBox->button(QDialogButtonBox::Cancel))
		{
			this->close();
		}

	}

	///
	/// \brief DialogP2P::checkIfChanged
	/// Check if something is changed in current profil edition

	void DialogP2P::checkIfChanged()
	{
		m_changed = false;
	}
}
