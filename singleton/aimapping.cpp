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
 * aimapping.cpp is part of FF2Play project
 *
 * This singleton provide AI database manager and tools for aircraft
 * translation
 * **************************************************************************/

#include "aimapping.h"
#include "singleton/singleton.h"
#include "tools/iniparser.h"

namespace ffs2play
{
    ///
    /// \brief AIMapping::AIMapping
    /// \param parent
    ///
    AIMapping::AIMapping(QObject *parent) : QObject(parent)
    {
        m_Initialized =false;
        m_thread = nullptr;
        m_log = CLogger::instance();
        m_simManager = simbasemanager::instance();
        connect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReceiveOpen()));
        connect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReceiveClose()));
        initSQLite();
    }

    ///
    /// \brief AIMapping::~AIMapping
    ///
    AIMapping::~AIMapping()
    {
        disconnect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReceiveOpen()));
        disconnect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReceiveClose()));
        m_DB.close();
    }

    ///
    /// \brief AIMapping::createInstance
    /// \return
    ///
    AIMapping* AIMapping::createInstance()
    {
        return new AIMapping();
    }

    ///
    /// \brief AIMapping::instance
    /// \return
    ///
    AIMapping* AIMapping::instance()
    {
        return Singleton<AIMapping>::instance(AIMapping::createInstance);
    }

    /*QString AIMapping::findInstallationPath(const QString &pKey)
    {
        QString Response = "";
#ifdef Q_OS_WIN32
        QSettings localKey("HKEY_LOCAL_MACHINE\\Software\\",QSettings::NativeFormat);
        QString registryKey;
        if (localKey.childGroups().contains("Software\\" + pKey,Qt::CaseInsensitive))
        {
            registryKey = "Software\\" + pKey;
        }
        else if (localKey.childGroups().contains("WOW6432Node\\" + pKey,Qt::CaseInsensitive))
        {
            registryKey = "WOW6432Node\\" + pKey;
        }
        else
        {
#ifdef FFS_DEBUG
            m_log->log(tr("AIMapping : Simulator ") + pKey + " not found", Qt::darkYellow,LEVEL_NORMAL);
#endif
            return "";
        }
        localKey.beginGroup(registryKey);
        // First we search key "Install_Path"
        if (localKey.childKeys().contains("Install_Path"),Qt::CaseInsensitive)
        {
            registryKey = "Install_Path";
        }
        else
        {
#ifdef FFS_DEBUG
            m_log->log(tr("AIMapping : Key Install_Path was not found in ") + pKey, Qt::darkYellow,LEVEL_NORMAL);
#endif
            // 2nd we search with key "SetupPath"
            if (localKey.childKeys().contains("SetupPath"),Qt::CaseInsensitive)
            {
                registryKey = "SetupPath";
            }
            else
            {
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : Key SetupPath was not found in ") + pKey, Qt::darkYellow,LEVEL_NORMAL);
#endif
                // 3rd we search with key "AppPath"
                if (localKey.childKeys().contains("AppPath"),Qt::CaseInsensitive)
                {
                    registryKey = "AppPath";
                }
                else
                {
#ifdef FFS_DEBUG
                    m_log->log(tr("AIMapping : Key AppPath was not found in ") + pKey, Qt::darkYellow,LEVEL_NORMAL);
#endif
                    return "";
                }
            }
        }
        Response = localKey.value(registryKey,"").toString();
#endif
        return Response;
    }*/

    ///
    /// \brief AIMapping::onSimReceiveOpen
    ///
    void AIMapping::onSimReceiveOpen()
    {
        if (m_simManager==nullptr) return;
        m_AircraftPaths.clear();
        QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
        m_Table = version.valueToKey(m_simManager->getVersion());
        startScanSimObject();
    }

    ///
    /// \brief AIMapping::onSimReceiveClose
    ///
    void AIMapping::onSimReceiveClose()
    {
        m_AircraftPaths.clear();
        m_Table.clear();
    }

    ///
    /// \brief AIMapping::initSQLite
    ///
    void AIMapping::initSQLite()
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
        query.prepare("create table if not exists Aircrafts ( checksum varchar(32), Sim varchar(16) collate nocase, Title varchar(256) collate nocase, Editor varchar(100) collate nocase, Type varchar(30) collate nocase, Model varchar(10) collate nocase, Path varchar(256) collate nocase, Category varchar(20) collate nocase, NbEngine int, EngineType int, CGHeight real, Pitch real);");
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
    void AIMapping::DBTableVersionCheck(const QString& pTable, int pVersion, bool pForceClean)
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
                if (query.value(0).toInt()!=pVersion)
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
    /// \brief AIMapping::startScanSimObject
    ///
    void AIMapping::startScanSimObject()
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
    /// \brief AIMapping::scanSimObject
    ///
    void AIMapping::scanSimObject()
    {
        m_NbAdded=0;
        m_NbUpdated=0;
        m_log->log(tr("Start updating aircraft database..."));
        //Since QT5.11, create new connexion for thread
        const QString thread_address = QLatin1String("0x") + QString::number((quintptr)QThread::currentThreadId(), 16);
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
                QDirIterator it(Directory, QStringList() << "aircraft.cfg" << "sim.cfg" << "*.acf", QDir::Files, QDirIterator::Subdirectories);
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
        m_log->log(tr("End of update : ") + QString::number(m_NbAdded) + tr(" aircrafts added and ") + QString::number(m_NbUpdated) + tr(" updated."));
        emit(fireScanFinished());
    }

    ///
    /// \brief AIMapping::DBFileUpdate
    /// \param pKey
    /// \param pPath
    ///
    void AIMapping::DBFileUpdate(const QString &pKey, const QString &pPath)
    {
        if ((!m_Initialized) || m_Table.isEmpty()) return;
        if (!m_Thread_DB.isOpen()) return;
        QSqlQuery query(m_Thread_DB);
        QString Path= pPath;
        query.prepare( "SELECT checksum FROM Aircrafts WHERE (checksum = :checksum) and (Sim = :Sim)");
        query.bindValue(":checksum",pKey);
        query.bindValue(":Sim",m_Table);
        if (query.exec())
        {
            if (!query.next())
            {
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : No record was found for ") + pKey, Qt::darkYellow,LEVEL_VERBOSE);
#endif
                try
                {
                    SIM_TYPE sim=m_simManager->getType();
                    IniParser s(Path,sim);
#ifdef Q_OS_WIN
                    Path = Path.replace("/","\\");
#endif
                    QString Editor,Atc_model, Atc_type, Category, Title;
                    int Engine_Type = 0;
                    double Static_CG_Height=0.0, Static_Pitch = 0.0;
                    if (sim==SIM_TYPE::FSX)
                    {
                        Editor = s.value("fltsim.0", "ui_createdby").toString();
                        Atc_model = s.value("General","atc_model").toString();
                        Atc_type =  s.value("General","atc_type").toString();
                        Category =  s.value("General","Category").toString();
                        Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
                        Static_CG_Height =  s.value("contact_points","static_cg_height").toDouble();
                        Static_Pitch =  s.value("contact_points","static_pitch").toDouble();
                    }
                    else if (sim==SIM_TYPE::XPLANE)
                    {
                        Title = s.value("properties", "acf/_name").toString();
                        if (Title=="")
                        {
                            QFileInfo File(Path);
                            Title = File.baseName();
                        }
                        Editor = s.value("properties", "acf/_studio").toString();
                        Atc_model = s.value("properties","acf/_ICAO").toString();
                        Atc_type = s.value("properties","acf/_manufacturer").toString();
                        //Category =  s.value("General","Category").toString();
                        //Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
                        Static_CG_Height =  s.value("properties","acf/_h_eqlbm").toDouble();
                        Static_Pitch =  s.value("properties","acf/_the_eqlbm").toDouble();
                    }
#if FFS_DEBUG
                    m_log->log(tr("AIMapping : Path = ") + Path,Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : Editor = ") + Editor,Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : ATCModel = ") + Atc_model, Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : ATCType = ") + Atc_type, Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : Category = ") + Category, Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : EngineType = ") + QString::number(Engine_Type), Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : CG Height = ") +  QString::number(Static_CG_Height), Qt::darkYellow,LEVEL_VERBOSE);
                    m_log->log(tr("AIMapping : Pitch = ") +  QString::number(Static_Pitch), Qt::darkYellow,LEVEL_VERBOSE);
#endif
                    int nb_moteur = 0;
                    while (true)
                    {
                        QString Reponse = s.value("GeneralEngineData", QString("Engine.") + QString::number(nb_moteur)).toString();
                        if (Reponse != "")
                        {
                            nb_moteur++;
                        }
                        else break;
                    }
#if FFS_DEBUG
                    m_log->log(tr("AIMapping : Engine quantity = ") + QString::number(nb_moteur),Qt::darkYellow,LEVEL_VERBOSE);
#endif
                    int nb_title = 0;
                    while (true)
                    {
                        if (sim==SIM_TYPE::FSX)
                            Title = s.value(QString("fltsim.") + QString::number(nb_title), "title").toString();

                        if (Title != "")
                        {
#if FFS_DEBUG
                            m_log->log(tr("AIMapping : livery found Title = ") + Title,Qt::darkYellow,LEVEL_VERBOSE);
#endif
                            nb_title++;
                            query.prepare("SELECT * FROM Aircrafts WHERE (Title = :Title) AND (checksum = :checksum) AND (Sim = :Sim)" );
                            query.bindValue(":Title",Title);
                            query.bindValue(":checksum",pKey);
                            query.bindValue(":Sim", m_Table);
                            if (query.exec())
                            {
                                // If there is no result, we have a new aircraft to register
                                if (!query.next())
                                {
                                    query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                                                  "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :Path, :Category, :NBEngine, :EngineType, :CGHeight, :Pitch)");
                                    query.bindValue(":checksum", pKey);
                                    query.bindValue(":Sim",m_Table);
                                    query.bindValue(":Title", Title);
                                    query.bindValue(":Editor", Editor);
                                    query.bindValue(":Type", Atc_type);
                                    query.bindValue(":Model", Atc_model);
                                    query.bindValue(":Path", Path);
                                    query.bindValue(":Category", Category);
                                    query.bindValue(":NBEngine", nb_moteur);
                                    query.bindValue(":EngineType", Engine_Type);
                                    query.bindValue(":CGHeight", Static_CG_Height);
                                    query.bindValue(":Pitch", Static_Pitch);
                                    query.exec();
                                    m_NbAdded++;
                                }
                                else //Update Record
                                {
                                    query.prepare("UPDATE Aircrafts SET checksum = :checksum, Editor = :Editor, Type = :Type, Model = :Model, Path = :Path, Category = :Category, NbEngine = :NbEngine, EngineType = :EngineType, CGHeight = :CHeight, Pitch = :Pitch WHERE (Title = :Title) and (Sim = :Sim)");
                                    query.bindValue(":checksum", pKey);
                                    query.bindValue(":Sim",m_Table);
                                    query.bindValue(":Title", Title);
                                    query.bindValue(":Editor", Editor);
                                    query.bindValue(":Type", Atc_type);
                                    query.bindValue(":Model", Atc_model);
                                    query.bindValue(":Path", Path);
                                    query.bindValue(":Category", Category);
                                    query.bindValue(":NBEngine", nb_moteur);
                                    query.bindValue(":EngineType", Engine_Type);
                                    query.bindValue(":CGHeight", Static_CG_Height);
                                    query.bindValue(":Pitch", Static_Pitch);
                                    query.exec();
                                    m_NbUpdated++;
                                }
                            }
                        }
                        else break;
                        if(sim==SIM_TYPE::XPLANE) break;
                    }
                }
                catch (IniParserException& ex)
                {
                    m_log->log(tr("AIMapping : IniParser error = ") + ex.Message,Qt::magenta,LEVEL_NORMAL);
                }
            }
        }
        else m_log->log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);

    }

    ///
    /// \brief AIMapping::SolveTitle
    /// \return
    ///
    AIResol AIMapping::SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType)
    {
        //Default substitute airtcraft
        AIResol Resol;
        QString Title = pTitle.toLower();
        QString Model = pModel.toLower();
        QString Type = pType.toLower();
        SIM_TYPE sim=m_simManager->getType();
        if (sim == SIM_TYPE::FSX)
        {
            Resol.set_title("Mooney Bravo");
            Resol.set_cgheight(3.7);
            Resol.set_pitch(2.9);
        }
        else if (sim == SIM_TYPE::XPLANE)
        {
            Resol.set_title("Beechcraft Baron 58");
            Resol.set_cgheight(3.7);
            Resol.set_pitch(2.9);
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
        query.prepare( "SELECT Title,CGHeight,Pitch,Path FROM Aircrafts WHERE (Title = :Title) and (Sim = :Sim)");
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
#ifdef FFS_DEBUG
                m_log->log(tr("AIMapping : Title Match found in database : ") + QString::fromStdString(Resol.title()), Qt::darkYellow,LEVEL_VERBOSE);
#endif
            }
            else
            {
                query.prepare( "SELECT Title,CGHeight,Pitch,Path FROM Aircrafts WHERE (Model like :Model) and (Sim = :Sim)");
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
        #ifdef FFS_DEBUG
                        m_log->log(tr("AIMapping : ATC Model Match found in database : ") + QString::fromStdString(Resol.title()), Qt::darkYellow,LEVEL_VERBOSE);
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
    /// \brief AIMapping::getAITitreDispo
    /// \param pCategory
    /// \param pEditor
    /// \param pType
    /// \param pModel
    /// \return
    ///
    QStringList AIMapping::getAITitreDispo(QString pCategory, QString pEditor, QString pType, QString pModel)
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
    /// \brief AIMapping::getCategoryList
    /// \return
    ///
    QStringList AIMapping::getCategoryList()
    {
        QStringList List;
        if (m_Initialized)
        {

        }
        return List;
    }

    ///
    /// \brief AIMapping::getEditeurList
    /// \return
    ///
    QStringList AIMapping::getEditeurList()
    {
        QStringList List;
        if (m_Initialized)
        {

        }
        return List;
    }

    ///
    /// \brief AIMapping::getATCTypeList
    /// \return
    ///
    QStringList AIMapping::getATCTypeList()
    {
        QStringList List;
        if (m_Initialized)
        {

        }
        return List;
    }

    ///
    /// \brief AIMapping::getATCModelList
    /// \return
    ///
    QStringList AIMapping::getATCModelList()
    {
        QStringList List;
        if (m_Initialized)
        {

        }
        return List;
    }

    ///
    /// \brief AIMapping::getRule
    /// \param pTitre
    /// \return
    ///
    QString AIMapping::getRule(const QString &pTitre)
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
    /// \brief AIMapping::addRule
    /// \param pRemoteTitle
    /// \param pLocalTitle
    ///
    void AIMapping::addRule(const QString &pRemoteTitle, const QString &pLocalTitle)
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
    /// \brief AIMapping::delRule
    /// \param pRemoteTitle
    ///
    void AIMapping::delRule(const QString &pRemoteTitle)
    {
        if (!m_Initialized) return;
        QSqlQuery query(m_DB);
        query.prepare("DELETE FROM Replacement WHERE ( RemoteTitle = :RemoteTitle ) and ( Sim =:Sim );");
        query.bindValue(":Sim",m_Table);
        query.bindValue(":RemoteTitle",pRemoteTitle);
        query.exec();
    }

    ///
    /// \brief AIMapping::cleanAIDB
    ///
    void AIMapping::cleanAIDB()
    {
        if (!m_Initialized) return;
    }

    ///
    /// \brief AIMapping::getList
    /// \return
    ///
    QMap<QString,QString> AIMapping::getList()
    {
        QMap<QString,QString> md5mlist;
        if (m_Initialized)
        {

        }
        return md5mlist;
    }

    ///
    /// \brief AIMapping::getAircraftPath
    /// \param pTitle
    /// \return
    ///
    QString AIMapping::getAircraftPath(const QString &pTitle)
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
    /// \brief AIMapping::getFileData
    /// \param pKey
    /// \return
    ///
    QString AIMapping::getFileData(const QString& /*&pKey*/)
    {
        QString Response;
        if (m_Initialized)
        {

        }
        return Response;
    }

    ///
    /// \brief AIMapping::DBFilePurge
    ///
    void AIMapping::DBFilePurge()
    {
        if (!m_Initialized) return;
        int counter=0;
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
                    counter++;
                    query.prepare("DELETE FROM Aircrafts WHERE (Path =:Path) and (Sim =:Sim)");
                    query.bindValue(":Sim",m_Table);
                    query.bindValue(":Path",Acft.fileName());
                    query.exec();
                }
            }
        }
#ifdef FFS_DEBUG
        m_log->log(tr("AIMapping : End of automatic cleaning. ") + QString::number(counter)  + tr("  over ") + QString::number(total) + tr(" entry(ies) removed"),Qt::darkYellow,LEVEL_NORMAL);
#endif
    }

    ///
    /// \brief AIMapping::getPathList
    /// \return
    ///
    QStringList AIMapping::getPathList()
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
    /// \brief AIMapping::addPath
    /// \param pPath
    /// \return
    ///
    bool AIMapping::addPath(const QString& pPath)
    {
        if ((!m_Initialized) || (m_Table.isEmpty())) return false;
        QSqlQuery query(m_DB);
        query.prepare("INSERT INTO SimPaths (Sim , Path) VALUES (:Sim , :Path);");
        query.bindValue(":Sim",m_Table);
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
    /// \brief AIMapping::delPath
    /// \param pPath
    /// \return
    ///
    bool AIMapping::delPath(const QString& pPath)
    {
        if ((!m_Initialized) || (m_Table.isEmpty())) return false;
        QSqlQuery query(m_DB);
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

    ///
    /// \brief AIMapping::MD5Hash
    /// \param pFileName
    /// \return
    ///
    QString AIMapping::MD5Hash(const QString& pFileName)
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
}
