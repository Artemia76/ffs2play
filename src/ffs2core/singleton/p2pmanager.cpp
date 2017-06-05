/****************************************************************************
**
** Copyright (C) 2017 FSFranceSimulateur team.
** Contact: https://github.com/ffs2/ffs2play
**
** FFS2Play is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** FFS2Play is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** The license is as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this software. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
****************************************************************************/

#include "singleton/p2pmanager.h"
#include <QNetworkDatagram>
#include "singleton/singleton.h"
#include "WinSock2.h"

#include <sstream>

using namespace ::std;
///
/// \brief CP2PManager::CP2PManager
/// \param parent
///

CP2PManager::CP2PManager(QObject *parent) : QObject(parent)
{
    m_logger=CLogger::instance();
    m_server = new UDPServer(this);
    connect (m_server,&UDPServer::fireIncomingData, this,&CP2PManager::OnReceiveDatagram);
}

///
/// \brief CP2PManager::createInstance
/// \return
///

CP2PManager* CP2PManager::createInstance()
{
    return new CP2PManager();
}

///
/// \brief CP2PManager::~CP2PManager
///

CP2PManager::~CP2PManager()
{
    init(false);
}

///
/// \brief CP2PManager::instance
/// \return
///

CP2PManager* CP2PManager::instance()
{
    return Singleton<CP2PManager>::instance(CP2PManager::createInstance);
}

///
/// \brief CP2PManager::init
/// \param flag
///

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

///
/// \brief CP2PManager::initUPNP
/// \param flag
///

void CP2PManager::initUPNP(bool flag)
{
    if (flag)
    {
        if (m_bUPNP) return;
        WSADATA wsaData;
        int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if(nResult != NO_ERROR) {
            m_logger->log("P2PManager : "+ tr("UPNP Windows Socket failed."),Qt::red);
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
#ifdef FFS_DEBUG
            for (UPNPDev* device = devlist; device !=NULL; device = device->pNext)
            {
                m_logger->log("P2PManager : " + tr("IGP Desc = ") + QString(device->descURL) + tr(" st: ") + QString(device->st),Qt::darkBlue,LEVEL_NORMAL);
            }
#endif
            if ((error = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, 64)) != 0)
            {
#ifdef FFS_DEBUG
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
#endif
                m_InternalIP.add_ip(lanaddr);
                UPNP_GetExternalIPAddress(urls.controlURL,data.first.servicetype,wanaddr);
                m_ExternalIP = QString(wanaddr);
#ifdef FFS_DEBUG
                m_logger->log("P2PManager : " + tr("Local LAN IP address : "),Qt::darkBlue,LEVEL_NORMAL);
                for (int i=0 ; i< m_InternalIP.ip_size(); ++i)
                {
                    m_logger->log("P2PManager : " + tr("IP address : ") + QString(m_InternalIP.ip(i).c_str()),Qt::darkBlue,LEVEL_NORMAL);
                }
                m_logger->log("P2PManager : " + tr("Internet IP address : ") + QString(m_ExternalIP),Qt::darkBlue,LEVEL_NORMAL);
#endif
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
                    m_logger->log("P2PManager : " + tr("Adding port mappping failed with code ") + QString::number(error),Qt::red);
                else m_bUPNP=true;
            }
            else
            {
                m_logger->log("P2PManager : " + tr("No valid UPNP Internet Gateway Device found."),Qt::red);
            }
            freeUPNPDevlist(devlist);
        }
        else
        {
            m_logger->log("P2PManager : " + tr("No IGD UPNP Device found on the network !"),Qt::red);
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
            m_logger->log("P2PManager : " + tr("Delete port mapping failed with code ") + QString::number(error),Qt::red);
        m_bUPNP = false;
    }
}

///
/// \brief CP2PManager::OnReceiveDatagram
/// \param data
///

void CP2PManager::OnReceiveDatagram(const QNetworkDatagram* data)
{

}

int CP2PManager::getPort()
{
    return m_port;
}

QString CP2PManager::getLocalIPs()
{
    std::ostringstream out;
    m_InternalIP.SerializeToOstream(&out);
    QByteArray OutPut(out.str().c_str());
    return QString(OutPut.toBase64());
}
