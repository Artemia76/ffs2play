#include "aimapping.h"
#include "singleton/singleton.h"

namespace ffs2play
{
	///
	/// \brief AIMapping::AIMapping
	/// \param parent
	///
	AIMapping::AIMapping(QObject *parent) : QObject(parent)
	{
		m_Initialized =false;
		m_log = CLogger::instance();
		m_simManager = simbasemanager::instance();
		connect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReveiveOpen()));
		connect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReveiveClose()));
		initSQLite();
	}

	///
	/// \brief AIMapping::~AIMapping
	///
	AIMapping::~AIMapping()
	{
		disconnect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReveiveOpen()));
		disconnect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReveiveClose()));
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

	void AIMapping::onSimReveiveOpen()
	{
		if (m_simManager==nullptr) return;
		m_AircraftPaths.clear();
		switch (m_simManager->getVersion())
		{
			case SIM_VERSION::FLIGHTGEAR:
			{
				m_Table = "FlightGear";
				break;
			}
			case SIM_VERSION::FSX:
			{
				m_Table = "FSX";
				break;
			}
			case SIM_VERSION::FSX_STEAM:
			{
				m_Table = "FSX_Steam";
				break;
			}
			case SIM_VERSION::P3D_V2:
			{
				m_Table = "P3D_V2";
				break;
			}
			case SIM_VERSION::P3D_V3:
			{
				m_Table = "P3D_V3";
				break;
			}
			case SIM_VERSION::P3D_V4:
			{
				m_Table = "P3D_V4";
				break;
			}
			case SIM_VERSION::XPLANE:
			{
				m_Table = "XPlane";
				break;
			}
			case SIM_VERSION::UNKNOWN:
			default:
			{
				m_Table.clear();
				break;
			}
			if (m_AircraftPaths.count() > 0)
			{
				checkSimTable();
				startScanSimObject();
			}
		}
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
		query.prepare( "create table if not exists Aircrafts ( checksum varchar(32), Sim varchar(16), Titre varchar(256) collate nocase, Editeur varchar(100) collate nocase, Type varchar(30) collate nocase, Model varchar(10) collate nocase, Path varchar(256) collate nocase, Categorie varchar(20) collate nocase, NbMoteur int, TypeMoteur int, CGHeight real, Pitch real);");
		if (!query.exec())
		{
#ifdef FFS_DEBUG
			m_log->log(tr("AIMapping : Fail to create SimPath Table"), Qt::darkYellow,LEVEL_NORMAL);
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
	void AIMapping::startScanSimObject()
	{
		if (m_AircraftPaths.count() == 0) return;
		if (m_thread != nullptr)
		{
			if (m_thread->isRunning()) m_thread->exit();
		}
		m_thread = new QThread();
		connect(m_thread, SIGNAL(started()), this, SLOT(scanSimObject()));
		m_thread->setPriority(QThread::LowestPriority);
		m_thread->start();
	}

	void AIMapping::scanSimObject()
	{
		m_NbAdded=0;
		m_NbUpdated=0;
		m_log->log(tr("Start updating aircraft database..."));
		for (QString Directory : m_AircraftPaths)
		{

		}
		DBFilePurge();
		m_log->log(tr("End of update : ") + QString::number(m_NbAdded) + tr(" aircrafts added and ") + QString::number(m_NbUpdated) + tr(" updated."));

	}

	void AIMapping::DBFileUpdate(const QString &pKey, const QString &pPath)
	{
		if (!m_Initialized) return;
	}

	///
	/// \brief AIMapping::SolveTitle
	/// \return
	///
	AIResol AIMapping::SolveTitle(const QString& /*pTitle*/, const QString& /*pModel*/, const QString& /*pType*/)
	{
		//Default substitute airtcraft
		AIResol Resol;
		Resol.Titre = "Mooney Bravo";
		Resol.CG_Height = 3.7;
		Resol.Pitch = 2.9;
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

	QString AIMapping::getPath(const QString &pCheckSum)
	{
		QString Response;
		if (m_Initialized)
		{

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
}
