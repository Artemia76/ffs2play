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
** aireplace.h is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#ifndef AIREPLACE_H
#define AIREPLACE_H

#include <QDialog>
#include <memory>
#include <QSettings>
#include <QPushButton>

#include "ui_aireplace.h"
#include "tools/clogger.h"
#include "net/p2pmanager.h"
#include "sim/simbasemanager.h"

namespace Ui {
class AIReplace;
}

class AIReplace : public QDialog
{
    Q_OBJECT

public:
    // Constructor whith QT Widget inherit
    explicit            AIReplace(const QString& pPeer, QWidget *parent = nullptr);

    // Destructor
    ~AIReplace();

private:
    // UI Designer GUI
    std::unique_ptr<Ui::AIReplace> m_UI;

    // Logger singleton instance
    CLogger&            m_Log = CLogger::instance();

    // Web Manager singleton instance
    CP2PManager&        m_P2PManager = CP2PManager::instance();

    // Simulator Manager singleton instance
    simbasemanager&     m_SimManager = simbasemanager::instance();

    // Configuration file Access
    QSettings           m_Settings;

    AIMapping&          m_AIMapping = AIMapping::instance();

    QString             m_Peer;
    QString             m_ExistingRule;
    bool                m_dialog_initialized;

    void                refreshReplacelist();
    void                updateRulesTable();
private slots:
    void                on_btnDeleteRule_clicked();
    void                on_btnBox_accepted();
    void                on_btnBox_rejected();
    void                on_cbCategory_currentTextChanged(const QString&);
    void                on_cbEngineNB_currentTextChanged(const QString&);
    void                on_cbEngineType_currentTextChanged(const QString&);
    void                on_cbType_currentTextChanged(const QString&);
    void                on_cbModel_currentTextChanged(const QString&);
    void                on_cbEditor_currentTextChanged(const QString&);
    void on_twRules_itemSelectionChanged();
};

#endif // DIALOGP2P_H
