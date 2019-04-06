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
** aibaseparser.cpp is part of FF2Play project
**
** This class is an abstract interface to adapt parsing method to scan SIM
** AI Files
****************************************************************************/

#include "aibaseparser.h"

namespace ffs2play { namespace disk {

AIBaseParser::AIBaseParser(QObject *parent) : QObject(parent)
{
    m_Initialized =false;
    m_log = CLogger::instance();
    m_simManager = simbasemanager::instance();
    m_thread = nullptr;
    QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
    m_Table = version.valueToKey(m_simManager->getVersion());
#ifdef FFS_DEBUG
    m_log->log(tr("AIMapping : aibaseparser constructor : ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
    // AI Database initialisation
    initSQLite();
}

AIBaseParser::~AIBaseParser()
{
    m_DB.close();
#ifdef FFS_DEBUG
    m_log->log(tr("AIMapping : aibaseparser destructor : ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
}

///
/// \brief AIMapping::initSQLite
///
void AIBaseParser::initSQLite()
{
    if (m_Initialized) return;
    m_DB=QSqlDatabase::addDatabase("QSQLITE");
    QString PathDatabase = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir(PathDatabase).exists())
    {
        QDir().mkpath(PathDatabase);
    }
    m_BaseFile = PathDatabase + QString("/AIDB.sqlite");
#ifdef FFS_DEBUG
    m_log->log(tr("AIMapping : open database : ") + m_BaseFile, Qt::darkYellow,LEVEL_NORMAL);
#endif
    m_DB.setDatabaseName(m_BaseFile);
    m_DB.open();
    QSqlQuery query(m_DB);

    //create if not exist the Version Table
    query.prepare("create table if not exists Version (TableName varchar(256) UNIQUE, Version int)");
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to create Version Table : ") + query.lastError().text(), Qt::darkYellow,LEVEL_NORMAL);
#endif
        return;
    }

    //Check if a new version is needed
    DBTableVersionCheck("SimPaths",AIDB_SIMPATHS_VERSION);
    // create if not exist the SimPath Table
    query.prepare("create table if not exists SimPaths (Path varchar(256) UNIQUE, Sim varchar(16))");
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to create SimPath Table"), Qt::darkYellow,LEVEL_NORMAL);
#endif
        return;
    }

    //Check if a new version is needed
    DBTableVersionCheck("Aircrafts",AIDB_AIRCRAFTS_VERSION,true);
    // create if not exist Aircrafts Table
    query.prepare("create table if not exists Aircrafts ( checksum varchar(32), Sim varchar(16) collate nocase, Title varchar(256) collate nocase, Editor varchar(100) collate nocase, Type varchar(30) collate nocase, Model varchar(10) collate nocase, AirLine varchar(10), Path varchar(256) collate nocase, Category varchar(20) collate nocase, NbEngine int, EngineType int, CGHeight real, Pitch real);");
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to create Aircrafts Table"), Qt::darkYellow,LEVEL_NORMAL);
#endif
        return;
    }

    //Check if a new version is needed
    DBTableVersionCheck("Replacement",AIDB_REPLACEMENT_VERSION);
    // create if not exist Replacement Table
    query.prepare("create table if not exists Replacement (RemoteTitle varchar(256) UNIQUE, LocalTitle varchar(256), Sim varchar(16) collate nocase)");
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to create Replacement Table"), Qt::darkYellow,LEVEL_NORMAL);
#endif
        return;
    }
    m_Initialized =true;
}

///
/// \brief AIMapping::DBTableVersionCheck
/// \param pTable
/// \param pVersion
/// \param pForceClean
///
void AIBaseParser::DBTableVersionCheck(const QString& pTable, int pVersion, bool pForceClean)
{
    if (!m_DB.isOpen()) return;
    QSqlQuery query(m_DB);
    bool NeedUpdate = false;
    query.prepare("SELECT Version FROM Version WHERE TableName = :TableName");
    query.bindValue(":TableName", pTable);
    if (query.exec())
    {
        if (query.next())
        {
            if (query.value(0).toInt()<pVersion)
            {
                NeedUpdate=true;
                query.prepare("UPDATE Version SET Version = :Version WHERE TableName = :TableName");
                query.bindValue(":Version", pVersion);
                query.bindValue(":TableName", pTable);
                query.exec();
            }
        }
        else
        {
            NeedUpdate=pForceClean;
            query.prepare("INSERT INTO Version (TableName, Version) VALUES(:TableName , :Version)");
            query.bindValue(":Version", pVersion);
            query.bindValue(":TableName", pTable);
            query.exec();
        }
    }
    else
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to query Version Table : ") + query.lastError().text(), Qt::darkYellow,LEVEL_NORMAL);
#endif
    }
    if (NeedUpdate)
    {
        query.prepare("DROP TABLE IF EXISTS " + pTable);
        query.exec();
    }
}

///
/// \brief AIBaseParser::startScanSimObject
///
void AIBaseParser::startScanSimObject()
{
    m_AircraftPaths = getPathList();
    //if (m_AircraftPaths.count() == 0) return;
    if (m_thread != nullptr)
    {
        if (m_thread->isRunning()) m_thread->exit();
    }
    m_thread = new QThread();
    connect(m_thread, SIGNAL(started()), this, SLOT(scanSimObject()),Qt::DirectConnection);
    m_thread->start();
    m_thread->setPriority(QThread::IdlePriority);
}

///
/// \brief AIBaseParser::getPathList
/// \return
///
QStringList AIBaseParser::getPathList()
{
    QStringList Result;
    if (m_Initialized && (!m_Table.isEmpty()))
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Path FROM SimPaths WHERE Sim = :Sim;");
        query.bindValue(":Sim",m_Table);
        if (query.exec())
        {
            while (query.next())
            {
                Result.append(query.value(0).toString());
            }
        }
        else
        {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to query path list for simulator ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        }
    }
    return Result;
}

///
/// \brief AIBaseParser::scanSimObject
///
void AIBaseParser::scanSimObject()
{
    m_NbAdded=0;
    m_NbUpdated=0;
    m_NbDeleted=0;
    m_log->log(tr("Start updating aircraft database..."));
    //Since QT5.11, create new connexion for thread
    const QString thread_address = QLatin1String("0x") + QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()), 16);
    QSqlDatabase Thread_DB=QSqlDatabase::database(thread_address);
    Thread_DB.setDatabaseName(m_BaseFile);
    if ((!Thread_DB.isOpen()) || (!Thread_DB.isValid()))
    {
        Thread_DB = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), thread_address);
    }
    m_Thread_DB = Thread_DB;
    m_Thread_DB.setDatabaseName(m_BaseFile);
    if (m_Thread_DB.open())
    {
        for (QString Directory : m_AircraftPaths)
        {
            QDirIterator it(QDir(Directory).canonicalPath(), m_AIFileFilter, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QString FileName = it.next();
                QString Hash = MD5Hash(FileName);
                DBFileUpdate(Hash,FileName);
            }
        }
        DBFilePurge();
    }
    else
    {
#if FFS_DEBUG
        m_log->log(tr("AIMapping : Failed to start thread scan Object Path du to SQLite DB connexion"),Qt::darkMagenta);
#endif
    }
    //Close DB Connexion
    m_Thread_DB.close();
    m_log->log(tr("End of update : ") + QString::number(m_NbAdded) +
               tr(" aircrafts added, ") + QString::number(m_NbUpdated) +
               tr(" updated and ") + QString::number(m_NbDeleted) + tr(" deleted."));
    emit(fireScanFinished(m_NbAdded, m_NbUpdated, m_NbDeleted));
}

///
/// \brief AIMapping::DBFilePurge
///
void AIBaseParser::DBFilePurge()
{
    if (!m_Initialized) return;
    int m_NbDeleted=0;
    int total =0;
#ifdef FFS_DEBUG
    m_log->log(tr("AIMapping : Start of automatic cleaning..."),Qt::darkYellow,LEVEL_NORMAL);
#endif
    QSqlQuery query(m_DB);
    query.prepare("SELECT Path FROM Aircrafts WHERE Sim =:Sim");
    query.bindValue(":Sim",m_Table);
    if (query.exec())
    {
        while (query.next())
        {
            total++;
            QFile Acft(query.value(0).toString());
            if (!Acft.exists())
            {
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : deleting file ") + Acft.fileName() + tr(" from DB"),Qt::darkYellow,LEVEL_NORMAL);
#endif
                m_NbDeleted++;
                QSqlQuery query2(m_DB);
                query2.prepare("DELETE FROM Aircrafts WHERE (Path =:Path) and (Sim =:Sim)");
                query2.bindValue(":Sim",m_Table);
                query2.bindValue(":Path",Acft.fileName());
                query2.exec();
            }
        }
    }
#ifdef FFS_DEBUG
    m_log->log(tr("AIMapping : End of automatic cleaning.: ") + QString::number(m_NbDeleted)  + tr(" over ") + QString::number(total) + tr(" entry(ies) removed"),Qt::darkYellow,LEVEL_NORMAL);
#endif
}

///
/// \brief AIBaseParser::MD5Hash
/// \param pFileName
/// \return
///
QString AIBaseParser::MD5Hash(const QString& pFileName)
{
    QFile f(pFileName);
    if (f.open(QFile::ReadOnly))
    {
       QCryptographicHash hash(QCryptographicHash::Md5);
       if (hash.addData(&f))
       {
           return QString(hash.result().toHex());
       }
    }
    return "";
}

///
/// \brief AIMapping::getAircraftPath
/// \param pTitle
/// \return
///
QString AIBaseParser::getAircraftPath(const QString &pTitle)
{
    QString Response;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Path FROM Aircrafts WHERE Title = :Title;");
        query.bindValue(":Title",pTitle);
        if (query.exec())
        {
            while (query.next())
            {
                Response=query.value(0).toString();
            }
        }
        else
        {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to query path list for simulator ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        }
    }
    return Response;
}

///
/// \brief AIBaseParser::SolveTitle
/// \return
///
AIResol AIBaseParser::SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType)
{
    //Default substitute airtcraft
    AIResol Resol;
    QString Title = pTitle.toLower();
    QString Model = pModel.toLower();
    QString Type = pType.toLower();
    if (m_simManager->getType() == SIM_TYPE::FSX)
    {
        Resol.set_title("Mooney Bravo");
        Resol.set_cgheight(3.7);
        Resol.set_pitch(2.9);
    }
    else if (m_simManager->getVersion()==SimConnector::SIM_VERSION::XPLANE)
    {
        Resol.set_title("Beech Baron 58");
        Resol.set_cgheight(0.9);
        Resol.set_pitch(1.93);
    }
    else if (m_simManager->getVersion()==SimConnector::SIM_VERSION::XPLANE_CSL)
    {
        Resol.set_title("C172:C172:0");
        Resol.set_cgheight(0.0);
        Resol.set_pitch(0.0);
    }
    if ((!m_Initialized) || m_Table.isEmpty()) return Resol;
    // Check local rules
    QString Rule = getRule(Title);
    if (Rule != "")
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Local rule found in DB : ") +  Title + tr(" replaced by ") + Rule, Qt::darkYellow,LEVEL_VERBOSE);
#endif
        Title = Rule;
    }
    // Check if Title exist in database
    QSqlQuery query(m_DB);
    query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Title = :Title) and (Sim = :Sim)");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Title",Title);

    if (query.exec())
    {
        if (query.next())
        {
            Resol.set_title(query.value(0).toString().toStdString());
            Resol.set_cgheight(query.value(1).toDouble());
            Resol.set_pitch(query.value(2).toDouble());
            Resol.set_path(query.value(3).toString().toStdString());
            Resol.set_model(query.value(4).toString().toStdString());
            Resol.set_airline(query.value(5).toString().toStdString());
#ifdef FFS_DEBUG
            m_log->log(tr("AIMapping : Title Match found in database : ") + QString::fromStdString(Resol.title()), Qt::darkYellow,LEVEL_VERBOSE);
#endif
        }
        else if (Model.count()>0)
        {
            query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Model like :Model) and (Sim = :Sim)");
            query.bindValue(":Sim",m_Table);
            query.bindValue(":Model",Model);
            if (query.exec())
            {
                if (query.next())
                {
                    Resol.set_title(query.value(0).toString().toStdString());
                    Resol.set_cgheight(query.value(1).toDouble());
                    Resol.set_pitch(query.value(2).toDouble());
                    Resol.set_path(query.value(3).toString().toStdString());
                    Resol.set_model(query.value(4).toString().toStdString());
                    Resol.set_airline(query.value(5).toString().toStdString());
    #ifdef FFS_DEBUG
                    m_log->log(tr("AIMapping : ATC Model")+ Model+ tr(" Match found in database : ") + QString::fromStdString(Resol.title()), Qt::darkYellow,LEVEL_VERBOSE);
    #endif
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : Fail to query database."), Qt::darkYellow,LEVEL_VERBOSE);
#endif
            }
        }
        else
        {
            Title = QString::fromStdString(Resol.title());
            query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Title = :Title) and (Sim = :Sim)");
            query.bindValue(":Sim",m_Table);
            query.bindValue(":Title",Title);
            if (query.exec())
            {
                if (query.next())
                {
                    Resol.set_title(query.value(0).toString().toStdString());
                    Resol.set_cgheight(query.value(1).toDouble());
                    Resol.set_pitch(query.value(2).toDouble());
                    Resol.set_path(query.value(3).toString().toStdString());
                    Resol.set_model(query.value(4).toString().toStdString());
                    Resol.set_airline(query.value(5).toString().toStdString());
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : Fail to query database."), Qt::darkYellow,LEVEL_VERBOSE);
#endif
            }
        }

    }
    else
    {
#ifdef FFS_DEBUG
            m_log->log(tr("AIMapping : Fail to query database."), Qt::darkYellow,LEVEL_VERBOSE);
#endif
    }
    return Resol;
}

///
/// \brief AIBaseParser::getAITitreDispo
/// \param pCategory
/// \param pEditor
/// \param pType
/// \param pModel
/// \return
///
QStringList AIBaseParser::getAITitreDispo(QString pCategory, QString pEditor, QString pType, QString pModel)
{
    QStringList List;
    if (m_Initialized)
    {
        if (pCategory == "*") pCategory = "";
        if (pEditor == "*") pEditor = "";
        if (pType == "*") pType = "";
        if (pModel == "*") pModel = "";
        bool Category, Editor, Type, Model;
        Category=Editor=Type=Model=false;
        QSqlQuery query(m_DB);
        QString command =  "SELECT Title FROM Aircrafts WHERE Sim = :Sim";
        if (pCategory.length()>0)
        {
            command += " and Category like :Category";
            Category =true;
        }
        if (pEditor.length()>0)
        {
            command += " and Editor like :Editor";
            Editor = true;
        }
        if (pType.length()>0)
        {
            command += " and Type like :Type";
            Type = true;
        }
        if (pModel.length()>0)
        {
            command += " and Model like :Model";
            Model = true;
        }
        command += " order by Title;";
        query.prepare(command);
        query.bindValue(":Sim",m_Table);
        if (Category) query.bindValue(":Category",pCategory);
        if (Editor) query.bindValue(":Editor",pEditor);
        if (Type) query.bindValue(":Type",pType);
        if (Model) query.bindValue(":Model",pModel);

        if (query.exec())
        {
            while (query.next())
            {
                List.append(query.value(0).toString());
            }
        }
    }
    return List;
}

///
/// \brief AIBaseParser::getRule
/// \param pTitre
/// \return
///
QString AIBaseParser::getRule(const QString &pTitre)
{
    QString Response="";
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT LocalTitle FROM Replacement WHERE (RemoteTitle like :RemoteTitle) and (Sim like :Sim)");
        query.bindValue(":Sim",m_Table);
        query.bindValue(":RemoteTitle",pTitre);
        if (query.exec())
        {
            if (query.next())
            {
                Response=query.value(0).toString();
            }
        }
    }
    return Response;
}

///
/// \brief AIBaseParser::addRule
/// \param pRemoteTitle
/// \param pLocalTitle
///
void AIBaseParser::addRule(const QString &pRemoteTitle, const QString &pLocalTitle)
{
    if (!m_Initialized) return;
    QSqlQuery query(m_DB);
    query.prepare("INSERT OR REPLACE INTO Replacement (RemoteTitle, LocalTitle, Sim) VALUES ( :RemoteTitle , :LocalTitle , :Sim );");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":RemoteTitle",pRemoteTitle);
    query.bindValue(":LocalTitle",pLocalTitle);
    query.exec();
}

///
/// \brief AIBaseParser::delRule
/// \param pRemoteTitle
///
void AIBaseParser::delRule(const QString &pRemoteTitle)
{
    if (!m_Initialized) return;
    QSqlQuery query(m_DB);
    query.prepare("DELETE FROM Replacement WHERE ( RemoteTitle = :RemoteTitle ) and ( Sim =:Sim );");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":RemoteTitle",pRemoteTitle);
    query.exec();
}

///
/// \brief AIBaseParser::cleanAIDB
///
void AIBaseParser::cleanAIDB()
{
    if (!m_Initialized) return;
    QSqlQuery query(m_DB);
    query.prepare("DELETE FROM Replacement");
    query.exec();
    query.prepare("DELETE FROM Aircrafts");
    query.exec();
    query.prepare("DELETE FROM SimPaths");
    query.exec();
}

///
/// \brief AIBaseParser::addPath
/// \param pPath
/// \return
///
bool AIBaseParser::addPath(const QString& pPath,const QString& pSim)
{
    QString Sim;
    if (pSim.isEmpty()) Sim = m_Table;
    else Sim=pSim;
    if ((!m_Initialized) || (Sim.isEmpty())) return false;
    QSqlQuery query(m_DB);
    query.prepare("INSERT INTO SimPaths (Sim , Path) VALUES (:Sim , :Path);");
    query.bindValue(":Sim",Sim);
    query.bindValue(":Path",pPath);
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to add path ") + pPath + tr(" for simulator ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        return false;
    }
    return true;
}

///
/// \brief AIBaseParser::delPath
/// \param pPath
/// \return
///
bool AIBaseParser::delPath(const QString& pPath)
{
    if ((!m_Initialized) || (m_Table.isEmpty())) return false;
    QSqlQuery query(m_DB);
    // Remove All aircraft in the aircraft
    query.prepare("DELETE FROM Aircrafts WHERE (Sim  = :Sim ) and (Path like :Path);");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Path","%" + pPath + "%");
    if (query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Remove ") + QString::number(query.numRowsAffected()) + tr(" AI for simulator ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
    }
    query.prepare("DELETE FROM SimPaths WHERE (Sim  = :Sim ) and (Path = :Path);");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Path",pPath);
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : Fail to remove path ") + pPath + tr(" for simulator ") + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        return false;
    }
    return true;
}

}} // namespace ffs2play::disk
