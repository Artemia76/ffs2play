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

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QSettings>

#include "singleton/clogger.h"

class UDPServer : public QObject
{
    Q_OBJECT
private:
    QUdpSocket* m_pListenner;
    CLogger*    Logger;
    bool        m_running;
    int         m_port;
    QSettings    s;
public:
                UDPServer(QObject *parent = 0);
                ~UDPServer();
    void        start();
    void        stop();
    void        setPort(int pPort);
    void        send(const QNetworkDatagram& pData);
signals:
    void        fireIncomingData (const QNetworkDatagram*);
private slots:
    void        incomingData () ;
};

#endif // UDPSERVER_H
