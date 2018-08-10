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
#include "singleton/webmanager.h"
#include "singleton/simbasemanager.h"

#include <QFileDialog>
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

		// Get AIMapping instance
		m_AIMapping = AIMapping::instance();

		// Get SimManager instance
		m_SimManager = simbasemanager::instance();

		// Get the default profil selected in configuration file
		m_settings.beginGroup("P2P");
		ui->cbInfoSim->setChecked(m_settings.value("InfoEnable", true).toBool());
		ui->cbUPNP->setChecked(m_settings.value("UPNPEnable",true).toBool());
		ui->numPort->setValue(m_settings.value("Port", 54857).toInt());
		m_settings.endGroup();
        ui->cbEnableCallsign->setChecked(m_SimManager->getOption(DisplayCallSign));
		ui->numAIRadius->setValue(m_P2PManager->getAIRadius());
		ui->numAILimit->setValue(m_P2PManager->getAILimit());
		ui->numP2PTx->setValue(m_P2PManager->getTxRate());
#ifdef FFS_DEBUG
        ui->cbShadow->setChecked(m_P2PManager->getShadowMode());
#else
        ui->cbShadow->setVisible(false);
#endif
		ui->cbShadow->setChecked(m_P2PManager->getShadowMode());
		m_needP2PRestart=false;
		m_needDBReload=false;
		for (QString Item : m_AIMapping->getPathList())
		{
			ui->lvItems->addItem(Item);
		}
		if (CWebManager::instance()->isConnected()) ui->numPort->setEnabled(false);

		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
		if (m_SimManager->getVersion()==SimConnector::SIM_VERSION::UNKNOWN)
		{
			ui->btnAddItem->setEnabled(false);
			ui->btnDelItem->setEnabled(false);
			ui->lvItems->setEnabled(false);
		}
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
		if (!m_changed) return;
		m_settings.beginGroup("P2P");
		// is SimInfo changed?
		if (ui->cbInfoSim->isChecked() != m_settings.value("InfoEnable", true).toBool())
		{
			m_settings.setValue("InfoEnable",ui->cbInfoSim->isChecked());
		}
		if (ui->cbUPNP->isChecked() != m_settings.value("UPNPEnable",true).toBool())
		{
			m_settings.setValue("UPNPEnable",ui->cbUPNP->isChecked());
		}
		if (ui->numPort->value() != m_settings.value("Port", 54857).toInt())
		{
			m_settings.setValue("Port",ui->numPort->value());
		}
		m_settings.endGroup();
		if (ui->cbShadow->isChecked() != m_P2PManager->getShadowMode())
		{
			m_P2PManager->setShadowMode(ui->cbShadow->isChecked());
		}
		if (ui->numP2PTx->value() != m_P2PManager->getTxRate())
		{
			m_P2PManager->setTxRate(ui->numP2PTx->value());
		}
		if (ui->numAIRadius->value() != m_P2PManager->getAIRadius())
		{
			m_P2PManager->setAIRadius(ui->numAIRadius->value());
		}
		if (ui->numAILimit->value() != m_P2PManager->getAILimit())
		{
			m_P2PManager->setAILimit(ui->numAILimit->value());
		}
		if (m_needP2PRestart)
		{
			m_P2PManager->init(false);
			m_P2PManager->init(true);
			m_needP2PRestart=false;
		}
		if (m_needDBReload)
		{
			m_needDBReload=false;
		}
        if (ui->cbEnableCallsign->isChecked() != m_SimManager->getOption(DisplayCallSign))
        {
            m_SimManager->setOptionChange(DisplayCallSign, ui->cbEnableCallsign->isChecked());
        }
		m_changed=false;
		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
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
		m_needP2PRestart = false;
		// Get the default profil selected in configuration file
		m_settings.beginGroup("P2P");
		// is SimInfo changed?
		if (ui->cbInfoSim->isChecked() != m_settings.value("InfoEnable", true).toBool())
		{
			m_changed = true;
		}
		if (ui->cbUPNP->isChecked() != m_settings.value("UPNPEnable",true).toBool())
		{
			m_changed = true;
			m_needP2PRestart =true;
		}
		if (ui->numPort->value() != m_settings.value("Port", 54857).toInt())
		{
			m_changed = true;
		}
		m_settings.endGroup();
		// is Shadow mode changed?
		if (ui->cbShadow->isChecked() != m_P2PManager->getShadowMode())
		{
			m_changed = true;
		}
		if (ui->numP2PTx->value() != m_P2PManager->getTxRate())
		{
			m_changed = true;
		}
		if (ui->numAIRadius->value() != m_P2PManager->getAIRadius())
		{
			m_changed = true;
		}
		if (ui->numAILimit->value() != m_P2PManager->getAILimit())
		{
			m_changed = true;
		}
        if (ui->cbEnableCallsign->isChecked() != m_SimManager->getOption(DisplayCallSign))
        {
            m_changed = true;
        }
		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(m_changed);
	}

	void DialogP2P::on_cbShadow_stateChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_cbInfoSim_stateChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_numP2PTx_valueChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_cbUPNP_stateChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_cbBeta_stateChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_btnAddItem_released()
	{
		QFileDialog FileDlg(this);
		QString Path;
		FileDlg.setFileMode(QFileDialog::Directory);
		if (FileDlg.exec())
		{
			Path = FileDlg.selectedFiles().first();
			if (!Path.isEmpty())
			{
				m_AIMapping->addPath(Path);
				ui->lvItems->addItem(Path);
				m_AIMapping->startScanSimObject();
			}
		}
	}

	void DialogP2P::on_btnDelItem_released()
	{
		QListWidgetItem* item = ui->lvItems->currentItem();
		if (item!=nullptr)
		{
			m_AIMapping->delPath(item->text());
			//ui->lvItems->removeItemWidget(item);
			delete item;
		}
	}

	void DialogP2P::on_numPort_valueChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_numAIRadius_valueChanged(int)
	{
		checkIfChanged();
	}

	void DialogP2P::on_numAILimit_valueChanged(int)
	{
		checkIfChanged();
	}

    void DialogP2P::on_cbEnableCallsign_stateChanged(int arg1)
    {
        checkIfChanged();
    }
}
