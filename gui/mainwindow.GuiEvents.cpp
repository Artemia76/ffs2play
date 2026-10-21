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
** mainwindow.Events.cpp is part of FF2Play project
**
** This class define the main window of the application
****************************************************************************/

#include "mainwindow.h"

#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>

#include "dialogprofils.h"
#include "aireplace.h"
#include "tools/getappversion.h"
#include "gui/dialogp2p.h"

#include "ui_mainwindow.h"

///
/// \brief MainWindow::closeEvent
///
void MainWindow::closeEvent(QCloseEvent*)
{
    m_settings.beginGroup("MainWindow");
    m_settings.setValue("Geometry",saveGeometry());
    m_settings.setValue("State",saveState());
    m_settings.endGroup();
    m_settings.beginGroup("Log");
    m_settings.setValue("MaxLine",m_maxLine);
    m_settings.endGroup();
    if (m_webManager!=nullptr) m_webManager->Disconnect();
    if (m_P2PManager!=nullptr) m_P2PManager->init(false);
    if (m_simManager!=nullptr) m_simManager->close();
}

///
/// \brief MainWindow::on_afServeurCon_triggered
///
void MainWindow::on_afServeurCon_triggered()
{
    m_webManager->connect_disconnect();
}

///
/// \brief MainWindow::on_afProfil_triggered
///
void MainWindow::on_afProfil_triggered()
{
    DialogProfils* dialog = new DialogProfils();
    dialog->show();
}

///
/// \brief MainWindow::on_afAide_triggered
///
void MainWindow::on_afAide_triggered()
{
    sVersion Version = GetAppVersion();
    QMessageBox::about(this,tr("About"), Version.FullVersion );
}

///
/// \brief MainWindow::on_afFSCon_triggered
///
void MainWindow::on_afFSCon_triggered()
{
    m_simManager->open_close();
}

///
/// \brief MainWindow::on_afP2P_triggered
///
void MainWindow::on_afP2P_triggered()
{
    m_P2PDialogVisible=true;
    DialogP2P* dialog = new DialogP2P(this);
    dialog->show();
    m_P2PDialogVisible=false;
}
///
/// \brief MainWindow::on_rb_filterNone_toggled
/// \param checked
///
void MainWindow::on_rb_filterNone_toggled(bool checked)
{
    if (checked) m_log.setDebugLevel(LEVEL_NONE);
}

///
/// \brief MainWindow::on_rb_filterNormal_toggled
/// \param checked
///
void MainWindow::on_rb_filterNormal_toggled(bool checked)
{
    if (checked) m_log.setDebugLevel(LEVEL_NORMAL);
}

///
/// \brief MainWindow::on_rb_filterVerbose_toggled
/// \param checked
///
void MainWindow::on_rb_filterVerbose_toggled(bool checked)
{
    if (checked) m_log.setDebugLevel(LEVEL_VERBOSE);
}

///
/// \brief MainWindow::on_cb_filter_currentIndexChanged
/// \param arg1
///
void MainWindow::on_cb_filter_currentIndexChanged(const QString &arg1)
{
    m_log.setFilter(arg1);
}

///
/// \brief MainWindow::on_afFSType_triggered
///
void MainWindow::on_afFSType_triggered()
{
    if (m_simManager->getSimList().count()==1) return;
    int index = m_simManager->getSimList().indexOf(m_simManager->getType());
    if (index == (m_simManager->getSimList().count()-1))
        index=0;
    else
        index++;
    m_simManager->setSimType(m_simManager->getSimList()[index]);
}

///
/// \brief MainWindow::on_leTchatSend_returnPressed
///
void MainWindow::on_leTchatSend_returnPressed()
{
    QString Message = ui->leTchatSend->text();
    ui->leTchatSend->clear();
    if(m_webManager->isConnected())
    {
        QString Tchat_line = m_P2PManager->getCallsign() + " : " + Message;
        ui->teTChatZone->append(Tchat_line);
        m_P2PManager->sendTChat(Message);
        m_simManager->sendScrollingText(Tchat_line);
    }
}

///
/// \brief MainWindow::onPeerShowContextMenu
/// \param pos
///
void MainWindow::onPeerShowContextMenu(const QPoint&pos)
{
    QListWidgetItem* temp = ui->lwPeer->itemAt(pos);
    if (temp != nullptr)
    {
        // Handle global position
        QPoint globalPos = ui->lwPeer->mapToGlobal(pos);
        // Create menu and insert some actions
        QMenu myMenu(this);
        // If Title is available on the peer, we enable AI replacement tool
        if (m_P2PManager->getTitle(temp->text()) !=  "")
        {
            myMenu.addAction("Mapping", this, &MainWindow::openMappingBox);
        }
#ifdef FFS_DEBUG
        //Block data

        if (m_P2PManager->getBlockedData(temp->text()))
        {
            QAction* BlockMenu;
            BlockMenu = myMenu.addAction("Unblock Data", this, &MainWindow::unblockData);
            BlockMenu->setCheckable(true);
            BlockMenu->setChecked(true);
        }
        else
        {
            myMenu.addAction("Block Data", this, &MainWindow::blockData);
        }
#endif
        // Mask IA
        QAction* MaskMenu = myMenu.addAction("Mask", this, &MainWindow::maskIA);
        MaskMenu->setCheckable(true);
        MaskMenu->setChecked(m_P2PManager->isDisabled(temp->text()));
        // Show context menu at handling position
        myMenu.exec(globalPos);
    }
}

///
/// \brief MainWindow::openMappingBox
///
void MainWindow::openMappingBox()
{
    QListWidgetItem* item = ui->lwPeer->currentItem();
    AIReplace* Dialog = new AIReplace(item->text());
    Dialog->show();
}

///
/// \brief MainWindow::unblockData
///
void MainWindow::unblockData()
{
    QListWidgetItem* item = ui->lwPeer->currentItem();
    if (item != nullptr)
    {
        m_P2PManager->setBlockedData(item->text(),false);
    }
}

///
/// \brief MainWindow::blockData
///
void MainWindow::blockData()
{
    QListWidgetItem* item = ui->lwPeer->currentItem();
    if (item != nullptr)
    {
        m_P2PManager->setBlockedData(item->text(),true);
    }
}

void MainWindow::maskIA()
{
    QListWidgetItem* item = ui->lwPeer->currentItem();
    if (item != nullptr)
    {
        m_P2PManager->setDisable(item->text(),!m_P2PManager->isDisabled(item->text()));
    }
}

///
/// \brief MainWindow::on_pteLogger_customContextMenuRequested
/// \param pos
///
void MainWindow::on_pteLogger_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = ui->pteLogger->createStandardContextMenu();
    // Handle global position
    QPoint globalPos = ui->pteLogger->mapToGlobal(pos);
    menu->addAction(tr("Clear"),this, &MainWindow::clearLog);
    menu->addAction(tr("Save"),this, &MainWindow::saveLog);
    menu->exec(globalPos);
    delete menu;
}

///
/// \brief MainWindow::clearLog
///
void MainWindow::clearLog()
{
    ui->pteLogger->clear();
}

///
/// \brief MainWindow::saveLog
///
void MainWindow::saveLog()
{
    QFileDialog FileDlg(this);
    QString File;
    FileDlg.setAcceptMode(QFileDialog::AcceptSave);
    FileDlg.setDefaultSuffix("htm");
    FileDlg.setNameFilter(tr("Web File (*.htm *.html)"));
    if (FileDlg.exec())
    {
        File = FileDlg.selectedFiles().first();
        if (!File.isEmpty())
        {
            QTextDocument* Doc = ui->pteLogger->document();
            QFile document (File);
            if(!document.open(QFile::WriteOnly | QFile::Text))
            {
                m_log.log(tr("An Error occur while opening ") + document.fileName(),Qt::darkMagenta);
                return;
            }
            QTextStream writer (&document);
            writer << Doc->toHtml();
        }
    }
}

