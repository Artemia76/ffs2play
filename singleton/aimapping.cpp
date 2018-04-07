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
	}

	///
	/// \brief AIMapping::~AIMapping
	///
	AIMapping::~AIMapping()
	{
		disconnect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReveiveOpen()));
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

	QString AIMapping::findInstallationPath(const QString &pKey)
	{
		QString Response = "";
#ifdef Q_OS_WIN32
#endif
		return Response;
	}

	void AIMapping::onSimReveiveOpen()
	{
		if (m_simManager==nullptr) return;
		m_AircraftPaths.clear();
		switch (m_simManager->getVersion())
		{
			case SIM_VERSION::FLIGHTGEAR:
			{
				break;
			}
			case SIM_VERSION::FSX:
			{
				break;
			}
			case SIM_VERSION::FSX_STEAM:
			{
				break;
			}
			case SIM_VERSION::P3D_V2:
			{
				break;
			}
			case SIM_VERSION::P3D_V3:
			{
				break;
			}
			case SIM_VERSION::P3D_V4:
			{
				break;
			}
			case SIM_VERSION::XPLANE:
			{
				break;
			}
			case SIM_VERSION::UNKNOWN:
			default:
			{
				break;
			}
			if (m_AircraftPaths.count() > 0)
			{
				initSQLite();
				startScanSimObject();
			}
		}
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


}
