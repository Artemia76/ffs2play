#include "simconnector.h"

SimConnector::SimConnector(QObject *parent) : QObject(parent)
{
    Log=CLogger::instance();
    m_bConnected = false;
}

SimConnector::~SimConnector()
{

}

bool SimConnector::isConnected()
{
    return m_bConnected;
}
