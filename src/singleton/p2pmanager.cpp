#include "singleton/p2pmanager.h"
#include <QNetworkDatagram>
#include "singleton/singleton.h"
#include "WinSock2.h"

CP2PManager::CP2PManager(QObject *parent) : QObject(parent)
{
    m_logger=CLogger::instance();
    m_server = new UDPServer(this);
    connect (m_server,&UDPServer::fireIncomingData, this,&CP2PManager::OnReceiveDatagram);
}

CP2PManager* CP2PManager::createInstance()
{
    return new CP2PManager();
}

CP2PManager::~CP2PManager()
{
    init(false);
}

CP2PManager* CP2PManager::instance()
{
    return Singleton<CP2PManager>::instance(CP2PManager::createInstance);
}

void CP2PManager::init(bool flag)
{
    if (flag)
    {
        s.beginGroup("P2P");
        m_port = s.value("Port",54856).toInt();
        m_bUPNP = false;
        m_server->setPort(m_port);
        m_server->start();
        if (s.value("UPNPEnable", true).toBool())
        {
            initUPNP(true);
        }
        s.endGroup();
    }
    else
    {
        m_server->stop();
        initUPNP(false);
    }
}

void CP2PManager::initUPNP(bool flag)
{
    if (flag)
    {
        if (m_bUPNP) return;
        WSADATA wsaData;
        int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(nResult != NO_ERROR) {
            m_logger->log("P2PManager : WSAStartup() failed.");
            return;
        }
        UPNPDev* devlist;
        char lanaddr[64]; // IP address on the LAN
        char wanaddr[64]; // IP address on the Internet
        const char* multicastif = 0;
        const char* minissdpdpath = 0;
        int error;
        devlist = upnpDiscover( 1000, multicastif, minissdpdpath, 0, 0,255, &error);
        if (devlist != NULL)
        {
            for (UPNPDev* device = devlist; device !=NULL; device = device->pNext)
            {
                m_logger->log("P2PManager : " + tr("IGP Desc = ") + QString(device->descURL) + tr(" st: ") + QString(device->st),Qt::darkBlue,LEVEL_NORMAL);
            }
            if ((error = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, 64)) != 0)
            {
                switch (error)
                {
                    case 1:
                    {
                        m_logger->log("P2PManager : " + tr("Found valid IGD : ") + QString(urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
                        break;
                    }
                    case 2:
                    {
                        m_logger->log("P2PManager : " + tr("Found a (not connected?) IGD : ") + QString(urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
                        break;
                    }
                    case 3:
                    {
                        m_logger->log("P2PManager : " + tr("UPnP device found. Is it an IGD ? : ") + QString(urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
                        break;
                    }
                    default:
                    {
                        m_logger->log("P2PManager : " + tr("Found device (IGD ?) : ") + QString(urls.controlURL),Qt::darkBlue,LEVEL_NORMAL);
                        break;
                    }
                }
                m_InternalIP = QString(lanaddr);
                m_logger->log("P2PManager : " + tr("Local LAN IP address : ") + QString(m_InternalIP),Qt::darkBlue,LEVEL_NORMAL);
                UPNP_GetExternalIPAddress(urls.controlURL,data.first.servicetype,wanaddr);
                m_ExternalIP = QString(wanaddr);
                m_logger->log("P2PManager : " + tr("Internet IP address : ") + QString(m_ExternalIP),Qt::darkBlue,LEVEL_NORMAL);
                error = UPNP_AddPortMapping(
                        urls.controlURL,
                        data.first.servicetype,
                        QString::number(m_port).toLatin1(),
                        QString::number(m_port).toLatin1(),
                        lanaddr,
                        "FFS2Play",
                        "UDP",
                        NULL,
                        "0"
                );
                if (error != UPNPCOMMAND_SUCCESS)
                    m_logger->log("P2PManager : " + tr("AddPortMapping failed with code ") + QString::number(error),Qt::red);
                else m_bUPNP=true;
            }
            else
            {
                m_logger->log("P2PManager : " + tr("No valid UPNP Internet Gateway Device found."),Qt::darkRed);
            }
            freeUPNPDevlist(devlist);
        }
        else
        {
            m_logger->log("P2PManager : " + tr("No IGD UPNP Device found on the network !"),Qt::darkRed);
        }
    }
    else
    {
        if (!m_bUPNP) return;
        int error = UPNP_DeletePortMapping(
                    urls.controlURL,
                    data.first.servicetype,
                    QString::number(m_port).toLatin1(),
                    "UDP",
                    NULL
        );
        if (error != UPNPCOMMAND_SUCCESS)
            m_logger->log("P2PManager : " + tr("DelPortMapping failed with code ") + QString::number(error),Qt::red);
        m_bUPNP = false;
    }
}

void CP2PManager::OnReceiveDatagram(const QNetworkDatagram* data)
{
    m_logger->log(tr("P2PManager : Receive datagram from ") + data->senderAddress().toString());
}
