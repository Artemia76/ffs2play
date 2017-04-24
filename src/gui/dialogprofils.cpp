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
 * dialogprofil.cpp is part of FF2Play project
 *
 * This class purpose a dialog interface to manage account profils
 * to connect severals FFS2Play networks servers
 * **************************************************************************/

#include "include/gui/dialogprofils.h"
#include "ui_dialogprofils.h"

#include <QMessageBox>

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
    // Init GUI Part
    ui->setupUi(this);

    // Get Logger instance
    Log = CLogger::instance();

    // Get WebManage instance to access profils list
    WebManager = CWebManager::instance();

    // Profil list widget is filled with each User in Profil List content
    for (const SUser& User : WebManager->Users)
    {
        // List item creation with profil name of current
        ui->listWidget->addItem(new QListWidgetItem(User.Profil));
    }
    // Get the default profil selected in configuration file
    s.beginGroup("Web");
    int num = s.value("Default",0).toInt();
    s.endGroup();

    // If Default profil is defined the correspondant row is selected
    if ((num>0) && (num<=ui->listWidget->count()))
    {
         ui->listWidget->setCurrentItem(ui->listWidget->item(num-1));
    }
    // Else we ensure that lines edit are disabled
    else lineEditEnable(false);

    // Apply button is disabled at start
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    Changed = false;
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

void DialogProfils::lineEditEnable(bool flag)
{
    ui->lineEdit_ProfilName->setEnabled(flag);
    ui->lineEdit_Login->setEnabled(flag);
    ui->lineEdit_Password->setEnabled(flag);
    ui->lineEdit_URL->setEnabled(flag);
}

///
/// \brief DialogProfils::on_btnAdd_pressed
///

void DialogProfils::on_btnAdd_pressed()
{
    SUser User;
    User.Profil = "Profil";
    int Row = ui->listWidget->currentRow();
    WebManager->Users.insert(Row+1,User);
    QListWidgetItem* item = new QListWidgetItem("Profil");
    ui->listWidget->insertItem(Row+1,item);
    ui->listWidget->setCurrentRow(Row+1);
}

///
/// \brief DialogProfils::on_btnDelete_pressed
///

void DialogProfils::on_btnDelete_pressed()
{
    int row = ui->listWidget->currentRow();
    WebManager->Users.removeAt(row);
    QListWidgetItem *item = ui->listWidget->takeItem(row);
    delete item;
    WebManager->SaveConfig();
}


///
/// \brief DialogProfils::on_listWidget_currentItemChanged
/// \param current
/// \param previous
///

void DialogProfils::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    // Let's check if changes are currently operating
    // If Yes, we ask if user want cancel current modifications
    if (current != CurrentProfil)
    {
        if (Changed)
        {
            int ret = QMessageBox::warning(this,tr("Warning"),
                                           tr("Some modifications are currently operated on selected profil. Do you want apply them?"),
                                           QMessageBox::Yes |QMessageBox::No);
            if (ret == QMessageBox::Yes)
            {
                applyChangedItem();
            }
        }
        if ((ui->listWidget->count() >0) && (current!=NULL))
        {
            SUser user = WebManager->Users.at(getCurrentRow());
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
        CurrentProfil = current;
    }
}

void DialogProfils::applyChangedItem()
{
    if (CurrentProfil != NULL)
    {
        SUser& user = WebManager->Users[ui->listWidget->row(CurrentProfil)];
        if (!ui->lineEdit_ProfilName->text().isEmpty())
        {
            user.Profil = ui->lineEdit_ProfilName->text();
            CurrentProfil->setText(user.Profil);
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
            user.URL = ui->lineEdit_URL->text();
        }
        Changed = false;
        WebManager->SaveConfig();
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
        if (Changed)
        {
            int ret = QMessageBox::warning(this,tr("Warning"),
                                           tr("Some modifications are currently operated on selected profil. Do you want apply them?"),
                                           QMessageBox::Yes |QMessageBox::No);
            if (ret == QMessageBox::Yes)
            {
                applyChangedItem();
            }
        }
        WebManager->SetCurrentUser(ui->listWidget->currentRow()+1);
        WebManager->SaveConfig();
        this->close();
    }
    if (button == ui->buttonBox->button(QDialogButtonBox::Cancel))
    {
        this->close();
    }

}

int DialogProfils::getCurrentRow()
{
    int row = ui->listWidget->currentRow();
    if (row >= WebManager->Users.count()) row = WebManager->Users.count()-1;
    return row;
}

///
/// \brief DialogProfils::checkIfChanged
/// Check if something is changed in current profil edition

void DialogProfils::checkIfChanged()
{
    Changed = false;
    SUser User = WebManager->Users.at(getCurrentRow());
    if (ui->lineEdit_ProfilName->text() != User.Profil) Changed =true;
    if (ui->lineEdit_Login->text() != User.Login) Changed =true;
    if (ui->lineEdit_Password->text() != User.Password) Changed =true;
    if (ui->lineEdit_URL->text() != User.URL) Changed =true;
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(Changed);
}

///
/// \brief DialogProfils::on_lineEdit_ProfilName_textChanged
/// \param arg1
///

void DialogProfils::on_lineEdit_ProfilName_textChanged(const QString&)
{
    checkIfChanged();
}

void DialogProfils::on_lineEdit_Login_textChanged(const QString &arg1)
{
    checkIfChanged();
}

void DialogProfils::on_lineEdit_Password_textChanged(const QString &arg1)
{
    checkIfChanged();
}

void DialogProfils::on_lineEdit_URL_textChanged(const QString &arg1)
{
    checkIfChanged();
}
