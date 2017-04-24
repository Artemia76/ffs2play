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
 * dialogprofil.h is part of FF2Play project
 *
 * This class purpose a dialog interface to manage account profils
 * to connect severals FFS2Play networks servers
 * **************************************************************************/

#ifndef DIALOGPROFILS_H
#define DIALOGPROFILS_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSettings>
#include <QAbstractButton>

#include "singleton/clogger.h"
#include "singleton/webmanager.h"

namespace Ui {
class DialogProfils;
}

class DialogProfils : public QDialog
{
    Q_OBJECT

public:
    // Constructor whith QT Widget inherit
    explicit DialogProfils(QWidget *parent = 0);

    // Destructor
    ~DialogProfils();

private:

    // UI Designer GUI
    Ui::DialogProfils *ui;

    // Logger singleton instance
    CLogger*        Log;

    // Web Manager singleton instance
    CWebManager*    WebManager;

    // Configuration file Access
    QSettings       s;

    // Flag to memorize if something is changed on current profil edition
    bool Changed = false;
    QListWidgetItem* CurrentProfil;
    // Method to enable or disable all texts entries
    void lineEditEnable(bool flag);

    void checkIfChanged();

    void applyChangedItem();

    int getCurrentRow();
private slots:
    // Button Add Profil Pressed Slot
    void on_btnAdd_pressed();

    // Button Delete Profil Pressed Slot
    void on_btnDelete_pressed();

    // Selection change in Profil list
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    // Button in ButtonBox is clicked
    void on_buttonBox_clicked(QAbstractButton *button);

    // Profil name line edit content changed
    void on_lineEdit_ProfilName_textChanged(const QString &arg1);
    void on_lineEdit_Login_textChanged(const QString &arg1);
    void on_lineEdit_Password_textChanged(const QString &arg1);
    void on_lineEdit_URL_textChanged(const QString &arg1);
};

#endif // DIALOGPROFILS_H
