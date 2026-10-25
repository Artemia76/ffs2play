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
** dialogupdate.h is part of FF2Play project
**
** Proposes an update of FFS2Play or of the xffs2play plugin : release notes,
** download of the package with its SHA-256 checked
****************************************************************************/

#ifndef DIALOGUPDATE_H
#define DIALOGUPDATE_H

#include <QDialog>
#include <QFile>
#include <QNetworkAccessManager>
#include <QPointer>

#include "web/updateinfo.h"

class QDialogButtonBox;
class QLabel;
class QNetworkReply;
class QProgressBar;
class QPushButton;

class DialogUpdate : public QDialog
{
    Q_OBJECT
public:
    enum class Choice
    {
        Later,
        Skip,
        Downloaded,
        OpenedPage
    };

                    DialogUpdate(const SUpdateInfo& pInfo, const QString& pTitle, const QString& pMessage,
                                 const QString& pDestination, QWidget* pParent = nullptr);
                    ~DialogUpdate() override;
    Choice          choice() const;
    /// Downloaded and checked package (Choice::Downloaded)
    QString         downloadedFile() const;

protected:
    void            reject() override;

private slots:
    void            onInstall();
    void            onOpenPage();
    void            onSkip();
    void            onReadyRead();
    void            onProgress(qint64 pReceived, qint64 pTotal);
    void            onFinished();

private:
    SUpdateInfo                 m_info;
    QString                     m_destination;
    Choice                      m_choice;
    QNetworkAccessManager       m_network;
    QPointer<QNetworkReply>     m_reply;
    QFile                       m_file;
    QLabel*                     m_status;
    QProgressBar*               m_progress;
    QDialogButtonBox*           m_buttons;
    QPushButton*                m_install;
    void                        fail(const QString& pMessage);
};

#endif // DIALOGUPDATE_H
