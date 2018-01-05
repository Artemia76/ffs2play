/****************************************************************************
**
** Copyright (C) 2018 FSFranceSimulateur team.
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

#ifndef XPMANAGER_H
#define XPMANAGER_H

#include <QObject>
#include <QThread>
#include <QDateTime>

#include "sim/simconnector.h"
#include "net/udpserver.h"
#include "protobuf/ffs2play.pb.h"

#pragma pack(1)

namespace ffs2p_xplane
{
    enum FFS2Proto
    {
        PING = 0x00, // Ping receive
        PONG = 0x01, // Pong With protocol Version
        DATA = 0x02, // Ask start or stop receive data
        DATA_SEND = 0x03, // Contain data of user plane
        DISC = 0x04 // Disconnect client
    };

    /// <summary>
    /// Structure d'importation des données envoyées par
    /// la librairie simconnect
    /// </summary>
    #pragma pack(1)
    struct UserState
    {
        char Title[256];
        char Type[256];
        char Model[256];
        char Category[256];
        double PlaneAltitude;
        double GndAltitude;
        double Vario;
        double PlaneHeading;
        double Longitude;
        double Latitude;
        double GndSpeed;
        double TASSpeed;
        double FuelQty;
        double TotalFuelCapacity;
        double FuelWeightGallon;
        double PlanePitch;
        double PlaneBank;
        double GForce;
        double PlaneWeight;
        quint32  Realism;
        double AmbiantWindVelocity;
        double AmbiantWindDirection;
        quint32    AmbiantPrecipState;
        double SeaLevelPressure;
        double IASSpeed;
        double ElevatorPos;
        double AileronPos;
        double RudderPos;
        quint32   TimeFactor;
        qint32    Door1Pos;
        qint32    Door2Pos;
        qint32    Door3Pos;
        qint32 Door4Pos;
        qint32 NumEngine;
        qint32 StateEng1;
        qint32 StateEng2;
        qint32 StateEng3;
        qint32 StateEng4;
        qint32 ThrottleEng1;
        qint32 ThrottleEng2;
        qint32 ThrottleEng3;
        qint32 ThrottleEng4;
        qint32 Squawk;
    };
    #pragma pack ()


    ///
    /// \brief The CSCManager class
    ///

    class CXPManager : public SimConnector
    {
        Q_OBJECT
    public:
        explicit CXPManager(QObject *parent = 0);
        ~CXPManager();
    private:
        qint64      m_SimRate;
        qint64      m_VariableRate;
        quint16     m_DestPort;
        quint16     m_LocalPort;
        QDateTime   m_LastVariable;
        QDateTime   m_LastSim;
        UDPServer*  Server;
        AirData     PlayerData;

    protected:
        bool        open();
        void        close();
        void        main_loop();
    private slots:
        void OnReceiveDatagram(const QNetworkDatagram* datagram);
    };
}
#endif // XPMANAGER_H
