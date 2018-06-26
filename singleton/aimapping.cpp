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

	void AIMapping::onSimReceiveOpen()
	{
		if (m_simManager==nullptr) return;
		m_AircraftPaths.clear();
		QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
		m_Table = version.valueToKey(m_simManager->getVersion());
		startScanSimObject();
	}

	void AIMapping::onSimReceiveClose()
	{
		m_AircraftPaths.clear();
		m_Table.clear();
	}

	void AIMapping::initSQLite()
	{
		if (m_Initialized) return;
		m_DB=QSqlDatabase::addDatabase("QSQLITE");
		QString PathDatabase = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		if (!QDir(PathDatabase).exists())
		{
			QDir().mkpath(PathDatabase);
		}
		m_DB.setDatabaseName(PathDatabase + QString("/AIDB.sqlite"));
		m_DB.open();
		QSqlQuery query;
		query.prepare("create table if not exists SimPaths (Path varchar(256) UNIQUE, Sim varchar(16))");
		if (!query.exec())
		{
#ifdef FFS_DEBUG
			m_log->log(tr("AIMapping : Fail to create SimPath Table"), Qt::darkYellow,LEVEL_NORMAL);
#endif
			return;
		}
		query.prepare("create table if not exists Aircrafts ( checksum varchar(32), Sim varchar(16) collate nocase, Title varchar(256) collate nocase, Editor varchar(100) collate nocase, Type varchar(30) collate nocase, Model varchar(10) collate nocase, Path varchar(256) collate nocase, Category varchar(20) collate nocase, NbEngine int, EngineType int, CGHeight real, Pitch real);");
		if (!query.exec())
		{
#ifdef FFS_DEBUG
			m_log->log(tr("AIMapping : Fail to create Aircrafts Table"), Qt::darkYellow,LEVEL_NORMAL);
#endif
			return;
		}
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

	void AIMapping::checkSimTable()
	{
		if (!m_DB.isOpen() && (!m_Table.isEmpty())) return;
		QSqlQuery query("PRAGMA table_info(" + m_Table + ");");
	}

	///
	/// \brief AIMapping::startScanSimObject
	///
	void AIMapping::startScanSimObject()
	{
		m_AircraftPaths = getPathList();
		if (m_AircraftPaths.count() == 0) return;
		if (m_thread != nullptr)
		{
			if (m_thread->isRunning()) m_thread->exit();
		}
		m_thread = new QThread();
		m_thread->setPriority(QThread::IdlePriority);
		connect(m_thread, SIGNAL(started()), this, SLOT(scanSimObject()),Qt::DirectConnection);
		m_thread->start();
	}

	///
	/// \brief AIMapping::scanSimObject
	///
	void AIMapping::scanSimObject()
	{
		m_NbAdded=0;
		m_NbUpdated=0;
		m_log->log(tr("Start updating aircraft database..."));
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
		m_log->log(tr("End of update : ") + QString::number(m_NbAdded) + tr(" aircrafts added and ") + QString::number(m_NbUpdated) + tr(" updated."));

	}

	void AIMapping::DBFileUpdate(const QString &pKey, const QString &pPath)
	{
		if ((!m_Initialized) || m_Table.isEmpty()) return;
		QSqlQuery query;
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
					IniParser s(pPath,sim);
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
						Editor = s.value("properties", "acf/_studio").toString();
						Atc_model = s.value("properties","acf/_ICAO").toString();
						Atc_type = s.value("properties","acf/_manufacturer").toString();
						//Category =  s.value("General","Category").toString();
						//Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
						Static_CG_Height =  s.value("properties","acf/_h_eqlbm").toDouble();
						Static_Pitch =  s.value("properties","acf/_the_eqlbm").toDouble();
					}
#if FFS_DEBUG
					m_log->log(tr("AIMapping : Path = ") + pPath,Qt::darkYellow,LEVEL_VERBOSE);
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
							query.prepare("SELECT * FROM Aircrafts WHERE Title = :Title AND checksum = :checksum");
							query.bindValue(":Title",Title);
							query.bindValue(":checksum",pKey);
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
									query.bindValue(":Path", pPath);
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
									query.bindValue(":Path", pPath);
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

		Resol.set_title("Mooney Bravo");
		Resol.set_cgheight(3.7);
		Resol.set_pitch(2.9);
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
		QSqlQuery query;
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

	QStringList AIMapping::getAITitreDispo(QString pCategorie, QString pEditeur, QString pType, QString pModel)
	{
		QStringList List;
		if (m_Initialized)
		{

		}
		return List;
	}

	QStringList AIMapping::getCategoryList()
	{
		QStringList List;
		if (m_Initialized)
		{

		}
		return List;
	}

	QStringList AIMapping::getEditeurList()
	{
		QStringList List;
		if (m_Initialized)
		{

		}
		return List;
	}

	QStringList AIMapping::getATCTypeList()
	{
		QStringList List;
		if (m_Initialized)
		{

		}
		return List;
	}

	QStringList AIMapping::getATCModelList()
	{
		QStringList List;
		if (m_Initialized)
		{

		}
		return List;
	}

	QString AIMapping::getRule(const QString &pTitre)
	{
		QString Response;
		if (m_Initialized)
		{

		}
		return Response;
	}

	void AIMapping::addRule(const QString &pRemoteTitle, const QString &pLocalTitle)
	{
		if (!m_Initialized) return;
	}

	void AIMapping::delRule(const QString &pRemoteTitle)
	{
		if (!m_Initialized) return;
	}

	void AIMapping::cleanAIDB()
	{
		if (!m_Initialized) return;
	}

	QMap<QString,QString> AIMapping::getList()
	{
		QMap<QString,QString> md5mlist;
		if (m_Initialized)
		{

		}
		return md5mlist;
	}

	QString AIMapping::getAircraftPath(const QString &pTitle)
	{
		QString Response;
		if (m_Initialized)
		{
			QSqlQuery query;
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

	QString AIMapping::getFileData(const QString &pKey)
	{
		QString Response;
		if (m_Initialized)
		{

		}
		return Response;
	}

	void AIMapping::DBFilePurge()
	{
		if (!m_Initialized) return;
	}

	QStringList AIMapping::getPathList()
	{
		QStringList Result;
		if (m_Initialized && (!m_Table.isEmpty()))
		{
			QSqlQuery query;
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

	bool AIMapping::addPath(const QString& pPath)
	{
		if ((!m_Initialized) || (m_Table.isEmpty())) return false;
		QSqlQuery query;
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



	bool AIMapping::delPath(const QString& pPath)
	{
		if ((!m_Initialized) || (m_Table.isEmpty())) return false;
		QSqlQuery query;
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
