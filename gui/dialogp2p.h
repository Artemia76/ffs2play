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
 * dialogp2p.h is part of FF2Play project
 *
 * This class purpose a dialog interface to manage account profils
 * to connect severals FFS2Play networks servers
 * **************************************************************************/

#ifndef DIALOGP2P_H
#define DIALOGP2P_H

#include <QDialog>
#include <QSettings>
#include <QPushButton>

#include "singleton/clogger.h"
#include "singleton/p2pmanager.h"
#include "singleton/simbasemanager.h"
#include "singleton/webmanager.h"

namespace Ui {
class DialogP2P;
}

namespace ffs2play
{
    class DialogP2P : public QDialog
    {
        Q_OBJECT

    public:
        // Constructor whith QT Widget inherit
        explicit            DialogP2P(QWidget *parent = 0);

        // Destructor
        ~DialogP2P();

    private:

        // UI Designer GUI
        Ui::DialogP2P*      ui;

        // Logger singleton instance
        CLogger*            m_log;

        // Web Manager singleton instance
        CP2PManager*        m_P2PManager;

        // Simulator Manager singleton instance
        simbasemanager*     m_SimManager;

        // Web manager singleton instance
        CWebManager*        m_WebManager;

        // Configuration file Access
        QSettings           m_settings;

        AIMapping*          m_AIMapping;

        // Flag to memorize if something is changed on current profil edition
        bool                m_changed;
        bool                m_needP2PRestart;
        bool                m_needDBReload;

        QColor              m_textColor;

        void                checkIfChanged();
        void                applyChangedItem();

    private slots:
        void                on_buttonBox_clicked(QAbstractButton* button);
        void                on_cbShadow_stateChanged(int);
        void                on_cbInfoSim_stateChanged(int);
        void                on_numP2PTx_valueChanged(int);
        void                on_cbUPNP_stateChanged(int);
        void                on_cbBeta_stateChanged(int);
        void                on_btnAddItem_released();
        void                on_btnDelItem_released();
        void                on_numPort_valueChanged(int);
        void                on_numAIRadius_valueChanged(int);
        void                on_numAILimit_valueChanged(int);
        void                on_cbEnableCallsign_stateChanged(int arg1);
        void                on_cbDropRateDisp_stateChanged(int arg1);
        void                on_cbSendDropRate_stateChanged(int arg1);
        void                on_pbTextColor_released();
        void on_pushButton_released();
        void on_cbDisableAltCorr_stateChanged(int arg1);
    };
}
#endif // DIALOGP2P_H
