#ifndef AIMAPPING_H
#define AIMAPPING_H

#include <QObject>
#include <QThread>
#include <QtSql>
#include "singleton/clogger.h"
#include "singleton/simbasemanager.h"

namespace ffs2play
{
	struct AIResol
	{
		QString Title;
		double  CG_Height;
		double  Pitch;
	};

	class AIMapping : public QObject
	{
		Q_OBJECT
		friend class FileExplorer;
	private:
		explicit			AIMapping(QObject *parent = nullptr);
		static	AIMapping*	createInstance();
		//QString				findInstallationPath (const QString& pKey);
		void				initSQLite();
		void				checkSimTable();
		void				DBFileUpdate(const QString& pKey,const QString& pPath);
		void				DBFilePurge ();
		QString				MD5Hash(const QString& pFileName);

		bool				m_Initialized;
		CLogger*			m_log;
		QSqlDatabase		m_DB;
		QThread*			m_thread;
		simbasemanager*		m_simManager;
		QString				m_Table;
		QStringList			m_AircraftPaths;
		int					m_NbAdded;
		int					m_NbUpdated;

	public:
							~AIMapping();
		static	AIMapping*	instance();
		void				startScanSimObject();
		AIResol				SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType );
		QStringList			getAITitreDispo(QString pCategorie = "", QString pEditeur="", QString pType="", QString pModel="");
		QStringList			getCategoryList();
		QStringList			getEditeurList();
		QStringList			getATCTypeList();
		QStringList			getATCModelList();
		QString				getRule(const QString& pTitre);
		void				addRule(const QString& pRemoteTitle, const QString& pLocalTitle);
		void				delRule(const QString& pRemoteTitle);
		void				cleanAIDB();
		QMap<QString,QString> getList();
		QString				getPath(const QString& pCheckSum);
		QString				getFileData(const QString& pKey);
		QStringList			getPathList();
		bool				addPath(const QString& pPath);
		bool				delPath(const QString& pPath);

	signals:

	private slots:
		void				onSimReceiveOpen();
		void				onSimReceiveClose();
		void				scanSimObject();
	public slots:
	};
}
#endif // AIMAPPING_H
