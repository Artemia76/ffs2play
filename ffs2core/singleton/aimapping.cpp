#include "aimapping.h"
#include "singleton/singleton.h"

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
