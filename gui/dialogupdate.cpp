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
** dialogupdate.cpp is part of FF2Play project
****************************************************************************/

#include "gui/dialogupdate.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

///
/// \brief DialogUpdate::DialogUpdate
/// \param pInfo answer of the portal
/// \param pTitle window title
/// \param pMessage text above the release notes
/// \param pDestination folder where the package is downloaded
///
DialogUpdate::DialogUpdate(const SUpdateInfo& pInfo, const QString& pTitle, const QString& pMessage,
                           const QString& pDestination, QWidget* pParent) :
    QDialog(pParent),
    m_info(pInfo),
    m_destination(pDestination),
    m_choice(Choice::Later)
{
    setWindowTitle(pTitle);
    resize(560, 420);
    auto* Layout = new QVBoxLayout(this);

    auto* Message = new QLabel(pMessage, this);
    Message->setWordWrap(true);
    Layout->addWidget(Message);

    auto* Notes = new QTextBrowser(this);
    Notes->setOpenExternalLinks(true);
    Notes->setMarkdown(pInfo.Notes.isEmpty() ? tr("No release notes.") : pInfo.Notes);
    Layout->addWidget(Notes, 1);

    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    m_status->hide();
    Layout->addWidget(m_status);
    m_progress = new QProgressBar(this);
    m_progress->hide();
    Layout->addWidget(m_progress);

    m_buttons = new QDialogButtonBox(this);
    // automatic install only for a package with its checksum
    const bool CanInstall = pInfo.HasAsset && !pInfo.Asset.Sha256.isEmpty();
    m_install = nullptr;
    if (CanInstall)
    {
        m_install = m_buttons->addButton(tr("Install"), QDialogButtonBox::AcceptRole);
        m_install->setDefault(true);
        connect(m_install, &QPushButton::clicked, this, &DialogUpdate::onInstall);
    }
    if (!pInfo.PageUrl.isEmpty())
    {
        QPushButton* Page = m_buttons->addButton(CanInstall ? tr("Release page") : tr("Open the download page"),
                                                 CanInstall ? QDialogButtonBox::HelpRole : QDialogButtonBox::AcceptRole);
        connect(Page, &QPushButton::clicked, this, &DialogUpdate::onOpenPage);
    }
    if (!pInfo.Mandatory)
    {
        QPushButton* Skip = m_buttons->addButton(tr("Skip this version"), QDialogButtonBox::DestructiveRole);
        connect(Skip, &QPushButton::clicked, this, &DialogUpdate::onSkip);
    }
    QPushButton* Later = m_buttons->addButton(tr("Later"), QDialogButtonBox::RejectRole);
    connect(Later, &QPushButton::clicked, this, &DialogUpdate::reject);
    Layout->addWidget(m_buttons);
}

DialogUpdate::~DialogUpdate()
{
    if (m_reply)
    {
        m_reply->disconnect(this);
        m_reply->abort();
        m_file.remove();
    }
}

DialogUpdate::Choice DialogUpdate::choice() const
{
    return m_choice;
}

QString DialogUpdate::downloadedFile() const
{
    return m_choice == Choice::Downloaded ? m_file.fileName() : QString();
}

///
/// \brief DialogUpdate::reject
/// Later, or cancel of the download
///
void DialogUpdate::reject()
{
    if (m_reply)
    {
        m_reply->abort();
        return;
    }
    m_choice = Choice::Later;
    QDialog::reject();
}

void DialogUpdate::onOpenPage()
{
    QDesktopServices::openUrl(QUrl(m_info.PageUrl));
    m_choice = Choice::OpenedPage;
    accept();
}

void DialogUpdate::onSkip()
{
    m_choice = Choice::Skip;
    QDialog::reject();
}

///
/// \brief DialogUpdate::onInstall
/// Downloads the package in <destination>/<name>.part
///
void DialogUpdate::onInstall()
{
    QDir().mkpath(m_destination);
    // the name comes from the portal : only its file name part is used
    const QString Name = QFileInfo(m_info.Asset.Name).fileName();
    m_file.setFileName(m_destination + "/" + Name + ".part");
    if (Name.isEmpty() || !m_file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        fail(tr("Can not write in ") + m_destination);
        return;
    }
    QNetworkRequest Request{QUrl(m_info.Asset.Url)};
    Request.setRawHeader("User-Agent", "FFS2Play");
    Request.setTransferTimeout(30000);
    m_reply = m_network.get(Request);
    connect(m_reply, &QNetworkReply::readyRead, this, &DialogUpdate::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DialogUpdate::onProgress);
    connect(m_reply, &QNetworkReply::finished, this, &DialogUpdate::onFinished);
    for (QAbstractButton* Button : m_buttons->buttons())
    {
        if (m_buttons->buttonRole(Button) != QDialogButtonBox::RejectRole) Button->setEnabled(false);
    }
    m_status->setText(tr("Downloading ") + Name + "...");
    m_status->show();
    m_progress->setRange(0, 0);
    m_progress->show();
}

void DialogUpdate::onReadyRead()
{
    if (m_reply) m_file.write(m_reply->readAll());
}

void DialogUpdate::onProgress(qint64 pReceived, qint64 pTotal)
{
    if (pTotal <= 0) pTotal = m_info.Asset.Size;
    if (pTotal <= 0) return;
    m_progress->setRange(0, 1000);
    m_progress->setValue(int(pReceived * 1000 / pTotal));
}

void DialogUpdate::onFinished()
{
    QNetworkReply* Reply = m_reply;
    m_reply = nullptr;
    Reply->deleteLater();
    m_file.write(Reply->readAll());
    m_file.close();
    if (Reply->error() != QNetworkReply::NoError)
    {
        m_file.remove();
        fail(Reply->error() == QNetworkReply::OperationCanceledError ? tr("Download cancelled.")
                                                                      : tr("Download failed : ") + Reply->errorString());
        return;
    }
    if (!UpdateInfo::checkSha256(m_file.fileName(), m_info.Asset.Sha256))
    {
        m_file.remove();
        fail(tr("The downloaded file is damaged (wrong SHA-256) : it has been deleted."));
        return;
    }
    // final name without .part, replacing an older download
    const QString Final = m_file.fileName().chopped(5);
    QFile::remove(Final);
    if (!m_file.rename(Final))
    {
        m_file.remove();
        fail(tr("Can not write in ") + m_destination);
        return;
    }
    m_choice = Choice::Downloaded;
    accept();
}

void DialogUpdate::fail(const QString& pMessage)
{
    m_progress->hide();
    m_status->setText(pMessage);
    m_status->show();
    for (QAbstractButton* Button : m_buttons->buttons()) Button->setEnabled(true);
}
