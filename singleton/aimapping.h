#ifndef AIMAPPING_H
#define AIMAPPING_H

#include <QObject>
#include "singleton/clogger.h"

namespace ffs2play
{
	struct AIResol
	{
		QString Titre;
		double  CG_Height;
		double  Pitch;
	};

	class AIMapping : public QObject
	{
		Q_OBJECT
	private:
		explicit			AIMapping(QObject *parent = nullptr);
		static	AIMapping*	createInstance();
		CLogger*			Log;

	public:
							~AIMapping();
		static	AIMapping*	instance();
		AIResol				SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType );
	signals:

	public slots:
	};
}
#endif // AIMAPPING_H
