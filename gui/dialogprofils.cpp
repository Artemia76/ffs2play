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
** dialogprofil.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "dialogprofils.h"

#include <QMessageBox>

namespace ffs2play { namespace gui {

///
/// \brief DialogBox constructor
/// \param parent
///
/// Constructor read users list previously loaded by webmanager constructor
/// Then it sync the List Widget with profil name as key
///
DialogProfils::DialogProfils(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfils)
{
    m_changed = false;
    // Init GUI Part
    ui->setupUi(this);

    // Get Logger instance
    m_log = CLogger::instance();

    // Get WebManage instance to access profils list
    m_webManager = CWebManager::instance();

    // Profil list widget is filled with each User in Profil List content
    for (const SUser& User : m_webManager->m_users)
    {
        // List item creation with profil name of current
        ui->listWidget->addItem(new QListWidgetItem(User.Profil));
    }
    // Get the default profil selected in configuration file
    m_settings.beginGroup("Web");
    int num = m_settings.value("Default",0).toInt();
    m_settings.endGroup();

    // If Default profil is defined the correspondant row is selected
    if ((num>0) && (num<=ui->listWidget->count()))
    {
         ui->listWidget->setCurrentItem(ui->listWidget->item(num-1));
    }
    // Else we ensure that lines edit are disabled
    else lineEditEnable(false);

    // Apply button is disabled at start
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    ui->lineEdit_Login->setValidator(new QRegExpValidator(QRegExp("[A-Za-z0-9_\\-\\.éèà]{0,32}"), this ));
}

///
/// \brief DialogProfils::~DialogProfils
///
/// Destructor
///
DialogProfils::~DialogProfils()
{
    delete ui;
}


///
/// \brief DialogProfils::lineEditEnable
/// \param flag
/// Enable or Disable line texts widgets for modification
///
void DialogProfils::lineEditEnable(bool pFlag)
{
    ui->lineEdit_ProfilName->setEnabled(pFlag);
    ui->lineEdit_Login->setEnabled(pFlag);
    ui->lineEdit_Password->setEnabled(pFlag);
    ui->lineEdit_URL->setEnabled(pFlag);
}

///
/// \brief DialogProfils::on_btnAdd_pressed
///
void DialogProfils::on_btnAdd_pressed()
{
    SUser User;
    User.Profil = "Profil";
    int Row = ui->listWidget->currentRow();
    m_webManager->m_users.insert(Row+1,User);
    QListWidgetItem* item = new QListWidgetItem("Profil");
    ui->listWidget->insertItem(Row+1,item);
    ui->listWidget->setCurrentRow(Row+1);
    ui->lineEdit_URL->setText("http://ffs2play.fr");
}

///
/// \brief DialogProfils::on_btnDelete_pressed
///
void DialogProfils::on_btnDelete_pressed()
{
    int row = ui->listWidget->currentRow();
    m_webManager->m_users.removeAt(row);
    QListWidgetItem *item = ui->listWidget->takeItem(row);
    delete item;
    m_webManager->SaveConfig();
}


///
/// \brief DialogProfils::on_listWidget_currentItemChanged
/// \param current
/// \param previous
///
void DialogProfils::on_listWidget_currentItemChanged(QListWidgetItem* pCurrent, QListWidgetItem*)
{
    // Let's check if changes are currently operating
    // If Yes, we ask if user want cancel current modifications
    if (pCurrent != m_currentProfil)
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
        if ((ui->listWidget->count() >0) && (pCurrent!=nullptr))
        {
            SUser user = m_webManager->m_users.at(getCurrentRow());
            ui->lineEdit_Login->setText(user.Login);
            ui->lineEdit_Password->setText(user.Password);
            ui->lineEdit_ProfilName->setText(user.Profil);
            ui->lineEdit_URL->setText(user.URL);
            lineEditEnable(true);
        }
        else
        {
            ui->lineEdit_Login->clear();
            ui->lineEdit_Password->clear();
            ui->lineEdit_ProfilName->clear();
            ui->lineEdit_URL->clear();
            lineEditEnable(false);
        }
        m_currentProfil = pCurrent;
    }
}

///
/// \brief DialogProfils::applyChangedItem
///
void DialogProfils::applyChangedItem()
{
    if (m_currentProfil != NULL)
    {
        SUser& user = m_webManager->m_users[ui->listWidget->row(m_currentProfil)];
        if (!ui->lineEdit_ProfilName->text().isEmpty())
        {
            user.Profil = ui->lineEdit_ProfilName->text();
            m_currentProfil->setText(user.Profil);
        }
        if (!ui->lineEdit_Login->text().isEmpty())
        {
            user.Login = ui->lineEdit_Login->text();
        }
        if (!ui->lineEdit_Password->text().isEmpty())
        {
            user.Password = ui->lineEdit_Password->text();
        }
        if (!ui->lineEdit_URL->text().isEmpty())
        {
            QUrl url = QUrl::fromUserInput(ui->lineEdit_URL->text());
            user.URL = url.toString();
            ui->lineEdit_URL->setText(user.URL);
        }
        m_changed = false;
        m_webManager->SaveConfig();
    }
}

///
/// \brief DialogProfils::on_buttonBox_clicked
/// \param button
///
void DialogProfils::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
    {
       applyChangedItem();
       checkIfChanged();
    }
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok))
    {
        applyChangedItem();
        m_webManager->SetCurrentUser(ui->listWidget->currentRow()+1);
        m_webManager->SaveConfig();
        this->close();
    }
    if (button == ui->buttonBox->button(QDialogButtonBox::Cancel))
    {
        this->close();
    }

}

///
/// \brief DialogProfils::getCurrentRow
/// \return
///
int DialogProfils::getCurrentRow()
{
    int row = ui->listWidget->currentRow();
    if (row >= m_webManager->m_users.count()) row = m_webManager->m_users.count()-1;
    return row;
}

///
/// \brief DialogProfils::checkIfChanged
/// Check if something is changed in current profil edition
void DialogProfils::checkIfChanged()
{
    m_changed = false;
    SUser User = m_webManager->m_users.at(getCurrentRow());
    if (ui->lineEdit_ProfilName->text() != User.Profil) m_changed =true;
    if (ui->lineEdit_Login->text() != User.Login) m_changed =true;
    if (ui->lineEdit_Password->text() != User.Password) m_changed =true;
    if (ui->lineEdit_URL->text() != User.URL) m_changed =true;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(m_changed);
}

///
/// \brief DialogProfils::on_lineEdit_ProfilName_textChanged
/// \param arg1
///
void DialogProfils::on_lineEdit_ProfilName_textChanged(const QString&)
{
    checkIfChanged();
}

///
/// \brief DialogProfils::on_lineEdit_Login_textChanged
/// \param arg1
///
void DialogProfils::on_lineEdit_Login_textChanged(const QString&)
{
    checkIfChanged();
}

///
/// \brief DialogProfils::on_lineEdit_Password_textChanged
/// \param arg1
///
void DialogProfils::on_lineEdit_Password_textChanged(const QString& )
{
    checkIfChanged();
}

///
/// \brief DialogProfils::on_lineEdit_URL_textChanged
/// \param arg1
///
void DialogProfils::on_lineEdit_URL_textChanged(const QString&)
{
    checkIfChanged();
}

}} // namespace ffs2play::gui
