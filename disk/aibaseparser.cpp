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

AIBaseParser::AIBaseParser(QObject *parent) : QObject(parent)
{
    m_Initialized =false;
    m_ScanSim = m_simManager.getType();
    QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
    m_Table = version.valueToKey(m_simManager.getVersion());
#ifdef FFS_DEBUG
    m_log.log("AIMapping : aibaseparser constructor : " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
    // AI Database initialisation
    initSQLite();
}

AIBaseParser::~AIBaseParser()
{
    // Already done by the derived destructors : the scan calls their DBFileUpdate()
    stopScan();
    m_DB.close();
#ifdef FFS_DEBUG
    m_log.log("AIMapping : aibaseparser destructor : " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
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
    m_log.log("AIMapping : open database : " + m_BaseFile, Qt::darkYellow,LEVEL_NORMAL);
#endif
    m_DB.setDatabaseName(m_BaseFile);
    m_DB.open();
    QSqlQuery query(m_DB);

    //create if not exist the Version Table
    query.prepare("create table if not exists Version (TableName varchar(256) UNIQUE, Version int)");
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : Fail to create Version Table : " + query.lastError().text(), Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Fail to create SimPath Table", Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Fail to create Aircrafts Table", Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Fail to create Replacement Table", Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Fail to query Version Table : " + query.lastError().text(), Qt::darkYellow,LEVEL_NORMAL);
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
    // Only one scan at a time on the database
    if (m_thread && m_thread->isRunning()) return;
    // Inputs of the scan are copied before the thread starts
    m_AircraftPaths = getPathList();
    m_ScanSim = m_simManager.getType();
    m_thread.reset(QThread::create([this]() { scanSimObject(); }));
    m_thread->start(QThread::IdlePriority);
}

///
/// \brief AIBaseParser::stopScan
/// Interrupts a running scan and waits for its end. Must be called by the destructor
/// of each derived class, before its part of the object is destroyed.
///
void AIBaseParser::stopScan()
{
    if (m_thread)
    {
        m_thread->requestInterruption();
        m_thread->wait();
    }
}

///
/// \brief AIBaseParser::isScanInterrupted
/// \return true if the scan thread must stop (parser destroyed)
///
bool AIBaseParser::isScanInterrupted() const
{
    return m_thread && m_thread->isInterruptionRequested();
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
        m_log.log("AIMapping : Fail to query path list for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
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
    m_log.log(tr("Start updating aircraft database..."));
    //Since QT5.11, create new connexion for thread
    const QString thread_address = QStringLiteral("0x%1" ).arg( reinterpret_cast<quintptr>( QThread::currentThread() ), 2 * QT_POINTER_SIZE, 16, QLatin1Char( '0' ) );
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
        for (const QString& Directory : m_AircraftPaths)
        {
            QDirIterator it(QDir(Directory).canonicalPath(), m_AIFileFilter, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext() && !isScanInterrupted())
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
        m_log.log("AIMapping : Failed to start thread scan Object Path du to SQLite DB connexion",Qt::darkMagenta);
#endif
    }
    //Close and remove the DB connexion of this thread
    m_Thread_DB.close();
    m_Thread_DB = QSqlDatabase();
    QSqlDatabase::removeDatabase(thread_address);
    finishScan();
}

///
/// \brief AIBaseParser::finishScan
/// Reports the counters of the update
///
void AIBaseParser::finishScan()
{
    m_log.log(tr("End of update : ") + QString::number(m_NbAdded) +
               tr(" aircrafts added, ") + QString::number(m_NbUpdated) +
               tr(" updated and ") + QString::number(m_NbDeleted) + tr(" deleted."));
    emit fireScanFinished(m_NbAdded, m_NbUpdated, m_NbDeleted);
}

///
/// \brief AIMapping::DBFilePurge
///
void AIBaseParser::DBFilePurge()
{
    if (!m_Initialized) return;
    int total =0;
#ifdef FFS_DEBUG
    m_log.log("AIMapping : Start of automatic cleaning...",Qt::darkYellow,LEVEL_NORMAL);
#endif
    // Runs in the scan thread : only its own connection can be used
    QSqlQuery query(m_Thread_DB);
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
                m_log.log("AIMapping : deleting file " + Acft.fileName() + " from DB",Qt::darkYellow,LEVEL_NORMAL);
#endif
                m_NbDeleted++;
                QSqlQuery query2(m_Thread_DB);
                query2.prepare("DELETE FROM Aircrafts WHERE (Path =:Path) and (Sim =:Sim)");
                query2.bindValue(":Sim",m_Table);
                query2.bindValue(":Path",Acft.fileName());
                query2.exec();
            }
        }
    }
#ifdef FFS_DEBUG
    m_log.log("AIMapping : End of automatic cleaning.: " + QString::number(m_NbDeleted)  + " over " + QString::number(total) + " entry(ies) removed",Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Fail to query path list for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        }
    }
    return Response;
}

///
/// \brief AIBaseParser::getAircraftInfo
/// \param pTitle
/// \param pModel ICAO type of the aircraft
/// \param pType
/// \param pCategory
/// \param pPath if not null, file of the aircraft
/// \param pNbEngine if not null, number of engines (0 unknown)
/// \param pEngineType if not null, engine type (FGAIParser codes, 4 undefined)
/// \return false if the aircraft is not in the database
///
bool AIBaseParser::getAircraftInfo(const QString& pTitle, QString& pModel, QString& pType, QString& pCategory,
                                   QString* pPath, int* pNbEngine, int* pEngineType)
{
    if ((!m_Initialized) || m_Table.isEmpty()) return false;
    QSqlQuery query(m_DB);
    // Several variants can share the title : first one with an ICAO type, then the main
    // file of the aircraft folder (c172p-set.xml rather than c172p-amphibious-set.xml)
    query.prepare("SELECT Model,Type,Category,Path,NbEngine,EngineType FROM Aircrafts WHERE (Title = :Title) and (Sim = :Sim) ORDER BY (ifnull(Model,'') = ''), length(Path)");
    query.bindValue(":Title",pTitle);
    query.bindValue(":Sim",m_Table);
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : Fail to query database.", Qt::darkYellow,LEVEL_VERBOSE);
#endif
        return false;
    }
    if (!query.next()) return false;
    pModel = query.value(0).toString();
    pType = query.value(1).toString();
    pCategory = query.value(2).toString();
    if (pPath) *pPath = query.value(3).toString();
    if (pNbEngine) *pNbEngine = query.value(4).toInt();
    if (pEngineType) *pEngineType = query.value(5).isNull() ? 4 : query.value(5).toInt();
    return true;
}

///
/// \brief AIBaseParser::SolveTitle
/// \return
///
AIResol AIBaseParser::SolveTitle(const QString& pTitle, const QString& pModel, const QString&)
{
    //Default substitute airtcraft
    AIResol Resol;
    QString Title = pTitle.toLower();
    QString Model = pModel.toLower();
    //QString Type = pType.toLower();
    switch (m_simManager.getVersion())
    {
#ifdef SIMCONNECT_BUILD
        case SimConnector::SIM_VERSION::FSX:
        case SimConnector::SIM_VERSION::FSX_STEAM:
        case SimConnector::SIM_VERSION::P3D_V2:
        case SimConnector::SIM_VERSION::P3D_V3:
        case SimConnector::SIM_VERSION::P3D_V4:
        case SimConnector::SIM_VERSION::P3D_V5:
            Resol.setTitle("Mooney Bravo");
            Resol.setCgHeight(3.7);
            Resol.setPitch(2.9);
            break;
#endif
        case SimConnector::SIM_VERSION::XPLANE:
            Resol.setTitle("Beech Baron 58");
            Resol.setCgHeight(0.9);
            Resol.setPitch(1.93);
            break;
        case SimConnector::SIM_VERSION::XPLANE_CSL:
            Resol.setTitle("C172:C172:0");
            Resol.setCgHeight(0.0);
            Resol.setPitch(0.0);
            break;
        case SimConnector::SIM_VERSION::FS2020:
            Resol.setTitle("Asobo Savage Cub");
            Resol.setCgHeight(0.0);
            Resol.setPitch(0.0);
            break;
        case SimConnector::SIM_VERSION::FS2024:
        {
            // No title is sure to exist : one of the listed aircraft, a 172 if possible
            if (m_Initialized)
            {
                QSqlQuery query(m_DB);
                query.prepare("SELECT Title, Path FROM Aircrafts WHERE Sim = :Sim ORDER BY (Title NOT LIKE '%172%'), Title LIMIT 1");
                query.bindValue(":Sim",m_Table);
                if (query.exec() && query.next())
                {
                    Resol.setTitle(query.value(0).toString());
                    Resol.setPath(query.value(1).toString());
                }
            }
            Resol.setCgHeight(0.0);
            Resol.setPitch(0.0);
            break;
        }
        case SimConnector::SIM_VERSION::FLIGHTGEAR:
            // Delivered with FlightGear
            Resol.setTitle("Aircraft/c172p/Models/c172p.xml");
            Resol.setCgHeight(0.0);
            Resol.setPitch(0.0);
            break;
        default:
            break;
    }
    if ((!m_Initialized) || m_Table.isEmpty()) return Resol;
    // Check local rules
    QString Rule = getRule(Title);
    if (Rule != "")
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : Local rule found in DB : " +  Title + " replaced by " + Rule, Qt::darkYellow,LEVEL_VERBOSE);
#endif
        Title = Rule;
    }
    // Check if Title exist in database
    QSqlQuery query(m_DB);
    query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Title like :Title) and (Sim = :Sim)");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Title",Title);

    if (query.exec())
    {
        if (query.next())
        {
            Resol.setTitle(query.value(0).toString());
            Resol.setCgHeight(query.value(1).toDouble());
            Resol.setPitch(query.value(2).toDouble());
            Resol.setPath(query.value(3).toString());
            Resol.setModel(query.value(4).toString());
            Resol.setAirLine(query.value(5).toString());
#ifdef FFS_DEBUG
            m_log.log("AIMapping : Title Match found in database : " + Resol.title(), Qt::darkYellow,LEVEL_VERBOSE);
#endif
        }
        else if (!Model.isEmpty())
        {
            query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Model like :Model) and (Sim = :Sim)");
            query.bindValue(":Sim",m_Table);
            query.bindValue(":Model",Model);
            if (query.exec())
            {
                if (query.next())
                {
                    Resol.setTitle(query.value(0).toString());
                    Resol.setCgHeight(query.value(1).toDouble());
                    Resol.setPitch(query.value(2).toDouble());
                    Resol.setPath(query.value(3).toString());
                    Resol.setModel(query.value(4).toString());
                    Resol.setAirLine(query.value(5).toString());
    #ifdef FFS_DEBUG
                    m_log.log("AIMapping : ATC Model"+ Model+ " Match found in database : " + Resol.title(), Qt::darkYellow,LEVEL_VERBOSE);
    #endif
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log.log("AIMapping : Fail to query database.", Qt::darkYellow,LEVEL_VERBOSE);
#endif
            }
        }
        else
        {
            Title = Resol.title();
            query.prepare( "SELECT Title,CGHeight,Pitch,Path,Model,AirLine FROM Aircrafts WHERE (Title = :Title) and (Sim = :Sim)");
            query.bindValue(":Sim",m_Table);
            query.bindValue(":Title",Title);
            if (query.exec())
            {
                if (query.next())
                {
                    Resol.setTitle(query.value(0).toString());
                    Resol.setCgHeight(query.value(1).toDouble());
                    Resol.setPitch(query.value(2).toDouble());
                    Resol.setPath(query.value(3).toString());
                    Resol.setModel(query.value(4).toString());
                    Resol.setAirLine(query.value(5).toString());
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log.log("AIMapping : Fail to query database.", Qt::darkYellow,LEVEL_VERBOSE);
#endif
            }
        }

    }
    else
    {
#ifdef FFS_DEBUG
            m_log.log("AIMapping : Fail to query database.", Qt::darkYellow,LEVEL_VERBOSE);
#endif
    }
    return Resol;
}

///
/// \brief AIBaseParser::getAITitleDispo
/// \param pCategory
/// \param pEditor
/// \param pType
/// \param pModel
/// \return
///
QStringList AIBaseParser::getAITitleDispo(
        QString pCategory,
        QString pEditor,
        QString pType,
        QString pModel,
        QString pEngineType,
        QString pNbEngine)
{
    QStringList List;
    if (m_Initialized)
    {
        if (pCategory == "*") pCategory = "";
        if (pEditor == "*") pEditor = "";
        if (pType == "*") pType = "";
        if (pModel == "*") pModel = "";
        if (pEngineType == "*") pEngineType = "";
        if (pNbEngine == "*") pNbEngine = "";
        bool Category, Editor, Type, Model, EngineType, NbEngine;
        int EngineTypeVal=0;
        Category=Editor=Type=Model=EngineType=NbEngine=false;
        QSqlQuery query(m_DB);
        QString command =  "SELECT DISTINCT Title FROM Aircrafts WHERE Sim = :Sim";
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
        if (pEngineType.length()>0)
        {
            if (pEngineType=="Piston") EngineTypeVal=0;
            else if (pEngineType=="Jet") EngineTypeVal=1;
            else if (pEngineType=="None") EngineTypeVal=2;
            else if (pEngineType=="RotorTurbine") EngineTypeVal=3;
            else if (pEngineType=="TurboProp") EngineTypeVal=5;
            else if (pEngineType=="Elec") EngineTypeVal=6;
            else EngineTypeVal=4;
            command += " and EngineType like :EngineType";
            EngineType = true;
        }
        if (pNbEngine.length()>0)
        {
            command += " and NbEngine = :NbEngine";
            NbEngine = true;
        }
        command += " order by Title;";
        query.prepare(command);
        query.bindValue(":Sim",m_Table);
        if (Category) query.bindValue(":Category",pCategory);
        if (Editor) query.bindValue(":Editor",pEditor);
        if (Type) query.bindValue(":Type",pType);
        if (Model) query.bindValue(":Model",pModel);
        if (EngineType) query.bindValue(":EngineType",EngineTypeVal);
        if (NbEngine) query.bindValue(":NbEngine",pNbEngine.toInt());
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
/// \param pTitle
/// \return
///
QString AIBaseParser::getRule(const QString &pTitle)
{
    QString Response="";
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT LocalTitle FROM Replacement WHERE (RemoteTitle like :RemoteTitle) and (Sim like :Sim)");
        query.bindValue(":Sim",m_Table);
        query.bindValue(":RemoteTitle",pTitle);
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
/// \brief AIBaseParser::getRulesList
/// \return
///
QMap<QString,QString> AIBaseParser::getRulesList()
{
    QMap<QString,QString> RulesList;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT RemoteTitle,LocalTitle FROM Replacement WHERE (Sim like :Sim)");
        query.bindValue(":Sim",m_Table);
        if (query.exec())
        {
            while (query.next())
            {
                RulesList.insert(query.value(0).toString(),query.value(1).toString());
            }
        }
    }
    return RulesList;
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
/// \brief AIBaseParser::getCategoryList
/// \return
///
QStringList AIBaseParser::getCategoryList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct Category FROM Aircrafts order by Category");
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
/// \brief AIBaseParser::getEditorList
/// \return
///
QStringList AIBaseParser::getEditorList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct Editor FROM Aircrafts order by Editor");
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
/// \brief AIBaseParser::getATCTypeList
/// \return
///
QStringList AIBaseParser::getATCTypeList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct Type FROM Aircrafts order by Type");
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
/// \brief AIBaseParser::getATCModelList
/// \return
///
QStringList AIBaseParser::getATCModelList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct Model FROM Aircrafts order by Model");
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
/// \brief AIBaseParser::getEngineTypeList
/// \return
///
QStringList AIBaseParser::getEngineTypeList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct EngineType FROM Aircrafts order by EngineType");
        if (query.exec())
        {
            while (query.next())
            {
                QString EngineType;
                switch (query.value(0).toInt())
                {
                    case 0:
                    {
                        EngineType = "Piston";
                        break;
                    }
                    case 1:
                    {
                        EngineType = "Jet";
                        break;
                    }
                    case 2:
                    {
                        EngineType = "None";
                        break;
                    }
                    case 3:
                    {
                        EngineType = "RotorTurbine";
                        break;
                    }
                    case 4:
                    {
                        EngineType = "Undefined";
                        break;
                    }
                    case 5:
                    {
                        EngineType = "TurboProp";
                        break;
                    }
                    case 6:
                    {
                        EngineType = "Elec";
                        break;
                    }
                    default:
                        EngineType = "Undefined";
                }
                List.append(EngineType);
            }
        }
    }
    return List;
}

///
/// \brief AIBaseParser::getEngineNumberList
/// \return
///
QStringList AIBaseParser::getNbEngineList()
{
    QStringList List;
    if (m_Initialized)
    {
        QSqlQuery query(m_DB);
        query.prepare("SELECT Distinct NbEngine FROM Aircrafts order by NbEngine");
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
        m_log.log("AIMapping : Fail to add path " + pPath + " for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
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
        m_log.log("AIMapping : Remove " + QString::number(query.numRowsAffected()) + " AI for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
    }
    query.prepare("DELETE FROM SimPaths WHERE (Sim  = :Sim ) and (Path = :Path);");
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Path",pPath);
    if (!query.exec())
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : Fail to remove path " + pPath + " for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        return false;
    }
    return true;
}
