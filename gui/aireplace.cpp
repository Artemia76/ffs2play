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
** aireplace.cpp is part of FF2Play project
**
** This class purpose a dialog interface to manage account profils
** to connect severals FFS2Play networks servers
****************************************************************************/

#include "aireplace.h"
#include "web/webmanager.h"
#include "sim/simbasemanager.h"

#include <QFileDialog>
#include <QMessageBox>

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
    m_dialog_initialized=false;
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
    m_UI->leOriginal->setText(m_P2PManager->getTitle(m_Peer));

    // Fill Category List
    m_UI->cbCategory->addItems(m_AIMapping->getCategoryList());
    m_UI->cbCategory->insertItem(0,"*");
    m_UI->cbCategory->setCurrentIndex(0);

    // Fill Editor List
    m_UI->cbEditor->addItems(m_AIMapping->getEditorList());
    m_UI->cbEditor->insertItem(0,"*");
    m_UI->cbEditor->setCurrentIndex(0);

    // Fill Type List
    m_UI->cbType->addItems(m_AIMapping->getATCTypeList());
    m_UI->cbType->insertItem(0,"*");
    m_UI->cbType->setCurrentIndex(0);

    // Fill Model List
    m_UI->cbModel->addItems(m_AIMapping->getATCModelList());
    m_UI->cbModel->insertItem(0,"*");
    m_UI->cbModel->setCurrentIndex(0);

    // Fill Engine Type
    m_UI->cbEngineType->addItems(m_AIMapping->getEngineTypeList());
    m_UI->cbEngineType->insertItem(0,"*");
    m_UI->cbEngineType->setCurrentIndex(0);

    // Fill Number of Engine
    m_UI->cbEngineNB->addItems(m_AIMapping->getNbEngineList());
    m_UI->cbEngineNB->insertItem(0,"*");
    m_UI->cbEngineNB->setCurrentIndex(0);
    m_dialog_initialized=true;
    updateRulesTable();
    refreshReplacelist();
    m_UI->twRules->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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

void AIReplace::refreshReplacelist()
{
    if (!m_dialog_initialized) return;
    QString Category = m_UI->cbCategory->currentText();
    QString Type = m_UI->cbType->currentText();
    QString Model = m_UI->cbModel->currentText();
    QString Editor = m_UI->cbEditor->currentText();
    QString EngineType = m_UI->cbEngineType->currentText();
    QString NbEngine = m_UI->cbEngineNB->currentText();
    m_UI->cbReplacement->clear();
    m_UI->cbReplacement->addItems(m_AIMapping->getAITitleDispo(
                                      Category,
                                      Editor,
                                      Type,
                                      Model,
                                      EngineType,
                                      NbEngine));
    QString RemoteAI = m_P2PManager->getTitle(m_Peer);
    m_ExistingRule = m_AIMapping->getRule(RemoteAI);
    if (m_ExistingRule != "")
    {
        m_UI->cbReplacement->setCurrentIndex(m_UI->cbReplacement->findText(m_ExistingRule));
        QList<QTableWidgetItem*> Item = m_UI->twRules->findItems(RemoteAI,Qt::MatchFixedString);
        if (Item.count()>0)
        {
            m_UI->twRules->selectRow(Item[0]->row());
            m_UI->btnDeleteRule->setVisible(true);
        }
    }
    else
    {
        AIResol Solve =m_AIMapping->SolveTitle(
                    m_P2PManager->getTitle(m_Peer),
                    m_P2PManager->getModel(m_Peer),
                    m_P2PManager->getType(m_Peer));
        int index =m_UI->cbReplacement->findText(Solve.title());
        if (index>-1)
        {
            m_UI->cbReplacement->setCurrentIndex(index);
        }
        m_UI->btnDeleteRule->setVisible(false);
        m_UI->twRules->clearSelection();
    }
}

///
/// \brief AIReplace::on_btnDeleteRule_clicked
///
void AIReplace::on_btnDeleteRule_clicked()
{
    QList<QTableWidgetItem*> Selection = m_UI->twRules->selectedItems();
    if (Selection.count()>0)
    {
        QString Source = Selection[0]->text();
        m_AIMapping->delRule(Source);
        updateRulesTable();
        m_P2PManager->ResetAI(m_Peer);
    }
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

///
/// \brief AIReplace::on_cbCategory_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbCategory_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

///
/// \brief AIReplace::on_cbEngineNB_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbEngineNB_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

///
/// \brief AIReplace::on_cbEngineType_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbEngineType_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

///
/// \brief AIReplace::on_cbType_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbType_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

///
/// \brief AIReplace::on_cbModel_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbModel_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

///
/// \brief AIReplace::on_cbEditor_currentIndexChanged
/// \param arg1
///
void AIReplace::on_cbEditor_currentIndexChanged(const QString&)
{
    refreshReplacelist();
}

void AIReplace::updateRulesTable()
{
    QMap<QString,QString> RulesList;
    RulesList = m_AIMapping->getRulesList();
    for (int i=m_UI->twRules->rowCount(); i>0;i--)
    {
        m_UI->twRules->removeRow(i-1);
    }
    for(const auto& rules : RulesList.toStdMap())
    {
      m_UI->twRules->insertRow(m_UI->twRules->rowCount());
      QTableWidgetItem* Source = new QTableWidgetItem();
      Source->setText(rules.first);
      QTableWidgetItem* Replace = new QTableWidgetItem();
      Replace->setText(rules.second);
      m_UI->twRules->setItem(m_UI->twRules->rowCount()-1, 0,Source);
      m_UI->twRules->setItem(m_UI->twRules->rowCount()-1, 1,Replace);
    }
}

void AIReplace::on_twRules_itemSelectionChanged()
{
    if (m_UI->twRules->selectedItems().count()>0)
        m_UI->btnDeleteRule->setVisible(true);
    else
        m_UI->btnDeleteRule->setVisible(false);
}
