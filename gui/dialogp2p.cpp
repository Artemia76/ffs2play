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
** dialogp2p.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "dialogp2p.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "ui_dialogp2p.h"
#include "web/webmanager.h"
#include "sim/simbasemanager.h"

///
/// \brief DialogBox constructor
/// \param parent
///
/// Constructor read users list previously loaded by webmanager constructor
/// Then it sync the List Widget with profil name as key
///
DialogP2P::DialogP2P(QWidget *parent)
    : QDialog(parent), ui(new Ui::DialogP2P)
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

    // Get Webmanager instance
    m_WebManager = CWebManager::instance();

    // Get the default profil selected in configuration file
    m_settings.beginGroup("P2P");
    ui->cbInfoSim->setChecked(m_settings.value("InfoEnable", true).toBool());
    ui->cbUPNP->setChecked(m_settings.value("UPNPEnable",true).toBool());
    ui->numPort->setValue(m_settings.value("Port", 54857).toInt());
    ui->cbEnforceTCP->setChecked(m_settings.value("EnforceTCP", false).toBool());
    m_settings.endGroup();
    m_settings.beginGroup("Main");
    ui->cbBeta->setChecked(m_settings.value("Beta",true).toBool());
    m_settings.endGroup();
    m_settings.beginGroup("XPlane");
    if (m_settings.value("AIMode",true).toBool())
    {
        ui->rbAI_Aircrafts->setChecked(true);
    }
    else
    {
        ui->rbCSL_Aircrafts->setChecked(true);
    }
    m_settings.endGroup();

    ui->cbEnableCallsign->setChecked(m_SimManager->getOption(DisplayCallSign));
    ui->cbDisableAltCorr->setChecked(m_SimManager->getOption(DisableAltCorr));
    ui->numAIRadius->setValue(m_P2PManager->getAIRadius());
    ui->numAILimit->setValue(m_P2PManager->getAILimit());
    ui->numP2PTx->setValue(m_P2PManager->getTxRate());
#ifndef FFS_DEBUG
    ui->cbShadow->setVisible(false);
#endif
    ui->cbShadow->setChecked(m_P2PManager->getShadowMode());

    ui->cbDropRateDisp->setChecked(m_P2PManager->getDropRateDisplay());
    ui->cbSendDropRate->setChecked(m_P2PManager->getSendDropRate());

    m_needP2PRestart=false;
    m_needDBReload=false;
    ui->pbTextColor->setVisible(false);
    ui->cbEnableCallsign->setVisible(false);

    m_textColor = m_SimManager->getCallSignColor();
    ui->pbTextColor->setStyleSheet(QString("QPushButton {" "background-color : %1" "}").arg(m_textColor.name()));
    ui->pbTextColor->update();

    if (m_SimManager->isConnected())
    {
        for (QString Item : m_AIMapping->getPathList())
        {
            ui->lvItems->addItem(Item);
        }
        if (m_SimManager->getType()==SIM_TYPE::XPLANE)
        {
            ui->pbTextColor->setVisible(true);
            ui->cbEnableCallsign->setVisible(true);
        }
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
    if (ui->cbEnforceTCP->isChecked() != m_settings.value("EnforceTCP", false).toBool())
    {
        m_settings.setValue("EnforceTCP", ui->cbEnforceTCP->isChecked());
        m_needP2PRestart = true;
    }
    m_settings.endGroup();
    m_settings.beginGroup("XPlane");
    if (ui->rbCSL_Aircrafts->isChecked()) m_settings.setValue("AIMode", false);
    else if (ui->rbAI_Aircrafts->isChecked()) m_settings.setValue("AIMode", true);
    m_SimManager->setOptionChange(UseAI,ui->rbAI_Aircrafts->isChecked());
    m_SimManager->setOptionChange(UseCSL,ui->rbCSL_Aircrafts->isChecked());
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
        if (m_WebManager->isConnected())
        {
            m_WebManager->Disconnect();
            m_WebManager->Connect();
        }
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
    if (ui->cbDisableAltCorr->isChecked() != m_SimManager->getOption(DisableAltCorr))
    {
        m_SimManager->setOptionChange(DisableAltCorr, ui->cbDisableAltCorr->isChecked());
    }
    if (ui->cbDropRateDisp->isChecked() != m_P2PManager->getDropRateDisplay())
    {
        m_P2PManager->setDropRateDisplay(ui->cbDropRateDisp->isChecked());
    }
    if (ui->cbSendDropRate->isChecked() != m_P2PManager->getSendDropRate())
    {
        m_P2PManager->setSendDropRate(ui->cbSendDropRate->isChecked());
    }
    if (ui->cbBeta->isChecked() != m_settings.value("Main/Beta",true).toBool())
    {
        m_settings.setValue("Main/Beta",ui->cbBeta->isChecked());
        m_WebManager->CheckNewVersion();
    }
    if (m_textColor != m_SimManager->getCallSignColor())
    {
        m_SimManager->setCallSignColor(m_textColor.name());
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
            applyChangedItem();
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
    if (ui->cbEnforceTCP->isChecked() != m_settings.value("EnforceTCP", false).toBool())
    {
        m_changed = true;
    }
    m_settings.endGroup();
    m_settings.beginGroup("XPlane");
    if (
            (m_settings.value("AIMode",true).toBool() && ui->rbCSL_Aircrafts->isChecked()) ||
            ((!m_settings.value("AIMode",true).toBool()) && ui->rbAI_Aircrafts->isChecked()))
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
    if (ui->cbDisableAltCorr->isChecked() != m_SimManager->getOption(DisableAltCorr))
    {
        m_changed = true;
    }
    if (ui->cbDropRateDisp->isChecked() != m_P2PManager->getDropRateDisplay())
    {
        m_changed = true;
    }
    if (ui->cbSendDropRate->isChecked() != m_P2PManager->getSendDropRate())
    {
        m_changed = true;
    }
    if (ui->cbBeta->isChecked() != m_settings.value("Main/Beta",true).toBool())
    {
        m_changed = true;
    }
    if (m_textColor != m_SimManager->getCallSignColor())
    {
        m_changed = true;
    }
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(m_changed);
}

///
/// \brief DialogP2P::on_cbShadow_stateChanged
///
void DialogP2P::on_cbShadow_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbInfoSim_stateChanged
///
void DialogP2P::on_cbInfoSim_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_numP2PTx_valueChanged
///
void DialogP2P::on_numP2PTx_valueChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbUPNP_stateChanged
///
void DialogP2P::on_cbUPNP_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbBeta_stateChanged
///
void DialogP2P::on_cbBeta_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_btnAddItem_released
///
void DialogP2P::on_btnAddItem_released()
{
    QFileDialog FileDlg(this);
    QDir Path;
    FileDlg.setFileMode(QFileDialog::Directory);
    if (FileDlg.exec())
    {
        Path = FileDlg.selectedFiles().first();
        m_log->log(" Path = " + Path.absolutePath() );
        if (!Path.isEmpty())
        {
            m_AIMapping->addPath(Path.absolutePath());
            ui->lvItems->addItem(Path.absolutePath());
            m_AIMapping->startScanSimObject();
        }
    }
}

///
/// \brief DialogP2P::on_btnDelItem_released
///
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

///
/// \brief DialogP2P::on_numPort_valueChanged
///
void DialogP2P::on_numPort_valueChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_numAIRadius_valueChanged
///
void DialogP2P::on_numAIRadius_valueChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_numAILimit_valueChanged
///
void DialogP2P::on_numAILimit_valueChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbEnableCallsign_stateChanged
///
void DialogP2P::on_cbEnableCallsign_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbDropRateDisp_stateChanged
///
void DialogP2P::on_cbDropRateDisp_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbSendDropRate_stateChanged
///
void DialogP2P::on_cbSendDropRate_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_pbTextColor_released
///
void DialogP2P::on_pbTextColor_released()
{
    QColorDialog dlgColor(m_textColor,this);
    dlgColor.exec();
    if (dlgColor.result()==QDialog::Accepted)
    {
        m_textColor = dlgColor.selectedColor();
        checkIfChanged();
    }
    ui->pbTextColor->setStyleSheet(QString("QPushButton {" "background-color : %1" "}").arg(m_textColor.name()));
}

///
/// \brief DialogP2P::on_pushButton_released
///
void DialogP2P::on_pushButton_released()
{
    int ret = QMessageBox::warning(this,tr("Warning"),
        tr("This function reset all AI Database for all simulator, are you sure to continue?"),
        QMessageBox::Yes |QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        m_AIMapping->cleanAIDB();
    }
}

///
/// \brief DialogP2P::on_cbDisableAltCorr_stateChanged
///
void DialogP2P::on_cbDisableAltCorr_stateChanged(int)
{
    checkIfChanged();
}

///
/// \brief DialogP2P::on_cbEnforceTCP_stateChanged
///
void DialogP2P::on_cbEnforceTCP_stateChanged(int)
{
    checkIfChanged();
}

void DialogP2P::on_rbAI_Aircrafts_toggled(bool)
{
    checkIfChanged();
}

void DialogP2P::on_rbCSL_Aircrafts_toggled(bool)
{
    checkIfChanged();
}
