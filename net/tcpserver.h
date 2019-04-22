/****************************************************************************
**
** Copyright (C) 2019 FSFranceSimulateur team.
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

/****************************************************************************
** tcpserver.h is part of FF2Play project
**
** This class provide a tcp server to receive and send data over networks
****************************************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QSctpServer>
#include <QSctpSocket>
#include <QSettings>

#include "net/tcpclient.h"
#include "tools/clogger.h"
#include "tools/datetime.h"

class TCPServer : public QTcpServer
{
    Q_OBJECT
private:
    QVector<TCPClient*> m_clients;
    CLogger*    m_log;
    bool        m_running;
    quint16     m_port;

protected:
    void        incomingConnection(qintptr socketDescriptor);

public:
    explicit    TCPServer(QObject *parent = nullptr);
                ~TCPServer();
    bool        start();
    void        stop();
    void        setPort(quint16 pPort);
    void        accept(TCPClient*);

signals:
    void        fireNewConnexion (TCPClient*);

private slots:
    void        OnCloseClient(TCPClient*);
signals:

public slots:

};

#endif // TCPSERVER_H
