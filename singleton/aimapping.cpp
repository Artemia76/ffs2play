#include "aimapping.h"
#include "singleton/singleton.h"

namespace ffs2play
{
	AIMapping::AIMapping(QObject *parent) : QObject(parent)
	{
		Log = CLogger::instance();
	}

	AIMapping::~AIMapping()
	{

	}

	AIMapping* AIMapping::createInstance()
	{
		return new AIMapping();
	}

	AIMapping* AIMapping::instance()
	{
		return Singleton<AIMapping>::instance(AIMapping::createInstance);
	}

	AIResol AIMapping::SolveTitle(const QString &pTitle, const QString &pModel, const QString &pType)
	{
		//Default substitute airtcraft
		AIResol Resol;
		Resol.Titre = "Mooney Bravo";
		Resol.CG_Height = 3.7;
		Resol.Pitch = 2.9;
		return Resol;
	}
}
