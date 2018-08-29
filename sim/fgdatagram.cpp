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

/****************************************************************************
 * fgdatagram.cpp is part of FF2Play project
 *
 * This is coder/decoder class for FlightGear Datagram
 * **************************************************************************/

#include "fgdatagram.h"

namespace ffs2play
{
    ///
    /// \brief FGDataGram::FGDatagram
    ///
    FGDatagram::FGDatagram()
    {
        m_Log = CLogger::instance();
        m_Sane = false;
    }

    ///
    /// \brief FGDatagram::~FGDatagram
    ///
    FGDatagram::~FGDatagram()
    {

    }

    ///
    /// \brief FGDatagram::Read
    /// \param pStream
    ///
    void FGDatagram::Read(QDataStream& pStream)
    {
        m_Sane = false;
        // Reading Header and continue if it is valid
        pStream.setByteOrder(QDataStream::BigEndian);
        pStream >> m_Header.Magic;
        pStream >> m_Header.Version;
        pStream >> m_Header.MsgId;
        pStream >> m_Header.MsgLen;
        pStream >> m_Header.Requested_Visibility;
        pStream >> m_Header.Replyport;
        pStream.readRawData(&m_Header.CallSign[0],sizeof(m_Header.CallSign));
        if (m_Header.Magic != MSG_MAGIC) return;
        m_Sane = true;
#ifdef FFS_DEBUG
        m_Log->log("FGManager : " + QString::asprintf("Receive FlightGear Header Magic= %0X, Version= %08X, MsgId = %d, MsgLen = %d, Requested Visibility = %d, Reply Port = %d, CallSign = ",
            m_Header.Magic,
            m_Header.Version,
            m_Header.MsgId,
            m_Header.MsgLen,
            m_Header.Requested_Visibility,
            m_Header.Replyport) + m_Header.CallSign,
            Qt::darkBlue,LEVEL_VERBOSE);
#endif
        //Reading Position Data
        pStream.readRawData(&m_Pos.ModelName[0],sizeof (m_Pos.ModelName));
        pStream >> m_Pos.Time;
        pStream >> m_Pos.Lag;
        for (int i=0; i<3; i++) pStream >> m_Pos.Position[i];
        for (int i=0; i<3; i++) pStream >> m_Pos.Orientation[i];
        for (int i=0; i<3; i++) pStream >> m_Pos.LinearVelocity[i];
        for (int i=0; i<3; i++) pStream >> m_Pos.AngularVelocity[i];
        for (int i=0; i<3; i++) pStream >> m_Pos.LinearAccel[i];
        for (int i=0; i<3; i++) pStream >> m_Pos.AngularAccel[i];
        char Dummy[8];
        pStream.readRawData(&Dummy[0],8);
#ifdef FFS_DEBUG
        m_Log->log(QString::asprintf("FGManager : a=%f , b=%f , c=%f",
            m_Pos.Orientation[0],
            m_Pos.Orientation[1],
            m_Pos.Orientation[2]
            ),Qt::darkBlue,LEVEL_VERBOSE);
#endif
    }

    ///
    /// \brief FGDatagram::Write
    /// \param pStream
    ///
    void FGDatagram::Write(QDataStream& pStream)
    {

    }

    ///
    /// \brief FGDatagram::IsSane
    ///
    bool FGDatagram::IsSane()
    {
        return m_Sane;
    }

    ///
    /// \brief FGDatagram::GetHeader
    /// \param pHeader
    /// \return
    ///
    bool FGDatagram::GetHeader(FGHeader& pHeader)
    {
        if (!m_Sane) return false;
        pHeader = m_Header;
        return true;
    }

    ///
    /// \brief FGDatagram::SetHeader
    /// \param pHeader
    /// \return
    ///
    bool FGDatagram::SetHeader(const FGHeader& pHeader)
    {
        if (pHeader.Magic!=MSG_MAGIC) return false;
        m_Header = pHeader;
        return true;
    }

    ///
    /// \brief FGDatagram::GetPos
    /// \param pPos
    /// \return
    ///
    bool FGDatagram::GetPos(FGPos& pPos)
    {
        if (!m_Sane) return false;
        pPos = m_Pos;
        return true;
    }

    ///
    /// \brief FGDatagram::SetPos
    /// \param pPos
    /// \return
    ///
    bool FGDatagram::SetPos(FGPos& pPos)
    {
        if (!m_Sane) return false;
        m_Pos = pPos;
        return true;
    }

    ///
    /// \brief FGDatagram::GetAirData
    /// \param pData
    /// \return
    ///
    bool FGDatagram::GetAirData(CAircraftState& pData)
    {
        if (!m_Sane) return false;
        CartToGeod (m_Pos.Position[0],m_Pos.Position[1], m_Pos.Position[2], pData.Longitude,pData.Latitude, pData.Altitude);
        pData.Title = m_Pos.ModelName;
        return true;
    }

    ///
    /// \brief FGDatagram::SetAirData
    /// \param pData
    /// \return
    ///
    bool FGDatagram::SetAirData(const CAircraftState& pData)
    {
        if (!m_Sane) return false;
    }
}
