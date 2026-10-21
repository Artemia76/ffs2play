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
** aimapping.h is part of FF2Play project
**
** This singleton provide AI database manager and tools for aircraft
** translation
****************************************************************************/

#ifndef AIMAPPING_H
#define AIMAPPING_H

#include <QObject>
#include <QtSql>
#include <memory>
#include "tools/clogger.h"
#include "sim/simbasemanager.h"
#include "ffs2play.qpb.h"
#ifdef SIMCONNECT_BUILD
#include "scaiparser.h"
#endif
#include "xpaiparser.h"
#include "fgaiparser.h"
#include "xpcslparser.h"

class AIMapping : public QObject
{
    Q_OBJECT
    friend class FileExplorer;
private:
    explicit                AIMapping(QObject *parent = nullptr);
    static  AIMapping*      createInstance();
    CLogger&                m_log = CLogger::instance();
    simbasemanager&         m_simManager = simbasemanager::instance();
    SimConnector::SIM_VERSION m_Version;
    /// Parser of the connected simulator, null when no simulator is connected
    std::unique_ptr<AIBaseParser> m_AIParser;

public:
                            ~AIMapping();
    static	AIMapping&      instance();

    void                    startScanSimObject();
    AIResol                 SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType );
    QStringList             getAITitleDispo(
                                QString pCategory = "",
                                QString pEditor="",
                                QString pType="",
                                QString pModel="",
                                QString pEngineType="",
                                QString pNbEngine="");
    QStringList             getCategoryList();
    QStringList             getEditorList();
    QStringList             getATCTypeList();
    QStringList             getATCModelList();
    QStringList             getEngineTypeList();
    QStringList             getNbEngineList();

    QStringList             getPathList();
    QString                 getRule(const QString& pTitle);
    void                    addRule(const QString& pRemoteTitle, const QString& pLocalTitle);
    void                    delRule(const QString& pRemoteTitle);
    void                    cleanAIDB();
    QMap<QString,QString>   getRulesList();
    QString                 getAircraftPath(const QString& pTitle);
    bool                    getAircraftInfo(const QString& pTitle, QString& pModel, QString& pType);
    QString                 getFileData(const QString& pKey);

    bool                    addPath(const QString& pPath, const QString& pSim="");
    bool                    delPath(const QString& pPath);
    bool                    isInit();

signals:
    void                    fireScanFinished(int pAdded, int pUpdated, int pDeleted);
private slots:
    void                    onSimReceiveOpen();
    void                    onSimReceiveClose();
public slots:
};

#endif // AIMAPPING_H
