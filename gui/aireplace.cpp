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
 * aireplace.cpp is part of FF2Play project
 *
 * This class purpose a dialog interface to manage account profils
 * to connect severals FFS2Play networks servers
 * **************************************************************************/

#include "aireplace.h"
#include "ui_aireplace.h"
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
    AIReplace::AIReplace(const QString& pPeer, QWidget *parent) :
        QDialog(parent),
        m_UI(new Ui::AIReplace)
    {
        // Init GUI Part
        m_UI->setupUi(this);

        // Get Logger instance
        m_Log = CLogger::instance();

        // Get P2PManager instance
        m_P2PManager = CP2PManager::instance();

        // Get AIMapping instance
        m_AIMapping = AIMapping::instance();

        // Get SimManager instance
        m_SimManager = simbasemanager::instance();

        m_Peer = pPeer;
        QString RemoteAI = m_P2PManager->getTitle(m_Peer);
        m_UI->leOriginal->setText(RemoteAI);
        m_UI->cbReplacement->addItems(m_AIMapping->getAITitreDispo("*","*","*","*"));
        m_ExistingRule = m_AIMapping->getRule(RemoteAI);
        if (m_ExistingRule != "")
        {
            m_UI->cbReplacement->setCurrentIndex(m_UI->cbReplacement->findText(m_ExistingRule));
            m_UI->btnDeleteRule->setVisible(true);
        }
        else
        {
            int index =m_UI->cbReplacement->findText(RemoteAI);
            if (index>-1)
            {
                m_UI->cbReplacement->setCurrentIndex(index);
            }
            m_UI->btnDeleteRule->setVisible(false);
        }
    }

    ///
    /// \brief AIReplace::~AIReplace
    ///
    /// Destructor
    ///
    AIReplace::~AIReplace()
    {
        delete m_UI;
    }

    ///
    /// \brief AIReplace::on_btnDeleteRule_clicked
    ///
    void AIReplace::on_btnDeleteRule_clicked()
    {
        m_AIMapping->delRule(m_UI->leOriginal->text());
        m_UI->btnDeleteRule->setVisible(false);
        m_P2PManager->ResetAI(m_Peer);
    }

    ///
    /// \brief AIReplace::on_btnBox_accepted
    ///
    void AIReplace::on_btnBox_accepted()
    {
        if (m_ExistingRule != m_UI->cbReplacement->currentText())
        {
            m_AIMapping->addRule(m_UI->leOriginal->text(),m_UI->cbReplacement->currentText());
            m_P2PManager->ResetAI(m_Peer);
        }
        close();
    }

    ///
    /// \brief AIReplace::on_btnBox_rejected
    ///
    void AIReplace::on_btnBox_rejected()
    {
        close();
    }
}
