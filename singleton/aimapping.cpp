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
		m_log = CLogger::instance();
		m_DB= QSqlDatabase::addDatabase("QSQLITE");
		QString PathDatabase = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		if (!QDir(PathDatabase).exists())
		{
			QDir().mkpath(PathDatabase);
		}
		m_DB.setDatabaseName(PathDatabase + QString("/AIDB.sqlite"));
		m_DB.open();
	}

	///
	/// \brief AIMapping::~AIMapping
	///
	AIMapping::~AIMapping()
	{
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
}
