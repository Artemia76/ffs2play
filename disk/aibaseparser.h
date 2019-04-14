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
** aibaseparser.h is part of FF2Play project
**
** This class is an abstract interface to adapt parsing method to scan SIM
** AI Files
****************************************************************************/

#ifndef AIBASEPARSER_H
#define AIBASEPARSER_H

#include <QObject>
#include <QThread>
#include <QtSql>
#include "tools/clogger.h"
#include "sim/simbasemanager.h"

#define AIDB_AIRCRAFTS_VERSION 3
#define AIDB_SIMPATHS_VERSION 1
#define AIDB_REPLACEMENT_VERSION 1

class AIBaseParser : public QObject
{
    Q_OBJECT
    friend  class   AIMapping;
public:
    explicit        AIBaseParser(QObject *parent = nullptr);
    virtual         ~AIBaseParser();

protected:
    bool            m_Initialized;
    QThread*        m_thread;
    QSqlDatabase    m_Thread_DB; //Since QT5.11, we need to separate DB connexion between threads
    QSqlDatabase    m_DB;
    QString         m_Table;
    QAtomicInt      m_NbAdded;
    QAtomicInt      m_NbUpdated;
    QAtomicInt      m_NbDeleted;
    CLogger*        m_log;
    QStringList     m_AircraftPaths;
    QString         m_BaseFile;
    simbasemanager* m_simManager;
    QStringList     m_AIFileFilter;

    void            startScanSimObject();
    QString         getAircraftPath(const QString& pTitle);
    AIResol         SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType );
    QStringList     getAITitreDispo(QString pCategorie = "", QString pEditeur="", QString pType="", QString pModel="");
    QString         getRule(const QString& pTitre);
    void            addRule(const QString& pRemoteTitle, const QString& pLocalTitle);
    void            delRule(const QString& pRemoteTitle);
    void            cleanAIDB();
    bool            addPath(const QString& pPath, const QString& pSim="");
    bool            delPath(const QString& pPath);
    virtual void    searchSimPath(){}
    virtual QString findInstallationPath (const QString&,QSettings::Format){return "";}
    QStringList     getPathList();
    QString         MD5Hash(const QString& pFileName);
    void            initSQLite();
    void            DBTableVersionCheck(const QString& pTable, int pVersion, bool pForceClean = false);

virtual void        DBFileUpdate(const QString& pKey,const QString& pPath)=0;
    void            DBFilePurge ();

signals:
    void            fireScanFinished(int pAdded, int pUpdated, int pDeleted);
private slots:
    void            scanSimObject();
};

#endif // AIBASEPARSER_H
