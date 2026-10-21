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
** fgdatagram.cpp is part of FF2Play project
**
** This is coder/decoder class for FlightGear Datagram
****************************************************************************/

#include <QDateTime>
#include <cstring>

#include "fgdatagram.h"

using namespace simgear;
///
/// \brief FGDataGram::FGDatagram
///
FGDatagram::FGDatagram()
{
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
    m_Log.log("FGManager : " + QString::asprintf("Receive FlightGear Header Magic= %0X, Version= %08X, MsgId = %d, MsgLen = %d, Requested Visibility = %d, Reply Port = %d, CallSign = ",
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
    // Time, lag and position are 8 bytes XDR doubles, all the following values 4 bytes
    // XDR floats : QDataStream reads float and double with the same precision setting
    pStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    pStream >> m_Pos.Time;
    pStream >> m_Pos.Lag;
    for (int i=0; i<3; i++) pStream >> m_Pos.Position[i];
    pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (int i=0; i<3; i++) pStream >> m_Pos.Orientation[i];
    for (int i=0; i<3; i++) pStream >> m_Pos.LinearVelocity[i];
    for (int i=0; i<3; i++) pStream >> m_Pos.AngularVelocity[i];
    for (int i=0; i<3; i++) pStream >> m_Pos.LinearAccel[i];
    for (int i=0; i<3; i++) pStream >> m_Pos.AngularAccel[i];
    // xdr_data_t pad of T_PositionMsg (FlightGear mpmessages.hxx) : properties start at byte 232
    char Dummy[4];
    pStream.readRawData(&Dummy[0],sizeof(Dummy));
#ifdef FFS_DEBUG
    m_Log.log(QString::asprintf("FGManager : Orientation (angle axis) = %f , %f , %f",
        static_cast<double>(m_Pos.Orientation[0]),
        static_cast<double>(m_Pos.Orientation[1]),
        static_cast<double>(m_Pos.Orientation[2])
        ),Qt::darkBlue,LEVEL_VERBOSE);
#endif
    //Reading Property data
    m_Props.clear();
    while (!pStream.atEnd())
    {
        FGPropertyData prop;
        if (!GetProperty(pStream,prop))
        {
            break;
        }
#ifdef FFS_DEBUG
        QString Value;
        switch (static_cast<int>(prop.type))
        {
        case TT_SHORTINT:
        case TT_SHORT_FLOAT_1:
        case TT_SHORT_FLOAT_2:
        case TT_SHORT_FLOAT_3:
        case TT_SHORT_FLOAT_4:
        case TT_SHORT_FLOAT_NORM:
        case INT:
        case LONG:
        case BOOL:
            Value = QString::number(prop.int_value);
            break;
        case FLOAT:
        case DOUBLE:
            Value = QString::number(static_cast<double>(prop.float_value));
            break;
        case STRING:
        case UNSPECIFIED:
            Value = QString::fromUtf8(prop.string_value);
            break;
        }
        m_Log.log(QString::asprintf("FGManager : Property Id=%d , Value = ", prop.id)+Value,Qt::darkBlue,LEVEL_VERBOSE);
#endif
        //break;
    }
}

///
/// \brief FGDatagram::Write
/// \param pStream
///
void FGDatagram::Write(QDataStream& pStream)
{
    pStream.setByteOrder(QDataStream::BigEndian);
    pStream << m_Header.Magic;
    pStream << m_Header.Version;
    pStream << m_Header.MsgId;
    pStream << m_Header.MsgLen;
    pStream << m_Header.Requested_Visibility;
    pStream << m_Header.Replyport;
    pStream.writeRawData(&m_Header.CallSign[0],sizeof(m_Header.CallSign));
    pStream.writeRawData(&m_Pos.ModelName[0],sizeof (m_Pos.ModelName));
    pStream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    pStream << m_Pos.Time;
    pStream << m_Pos.Lag;
    for (int i=0; i<3; i++) pStream << m_Pos.Position[i];
    pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    for (int i=0; i<3; i++) pStream << m_Pos.Orientation[i];
    for (int i=0; i<3; i++) pStream << m_Pos.LinearVelocity[i];
    for (int i=0; i<3; i++) pStream << m_Pos.AngularVelocity[i];
    for (int i=0; i<3; i++) pStream << m_Pos.LinearAccel[i];
    for (int i=0; i<3; i++) pStream << m_Pos.AngularAccel[i];
    pStream << quint32(0); // pad
}

///
/// \brief FGDatagram::SetIdentity
/// Prepares a position message without properties
/// \param pCallSign truncated to 7 characters
/// \param pModel model path, ex "Aircraft/c172p/Models/c172p.xml"
///
void FGDatagram::SetIdentity(const QString& pCallSign, const QString& pModel)
{
    m_Header = FGHeader();
    m_Header.Magic = MSG_MAGIC;
    m_Header.Version = PROTO_VER;
    m_Header.MsgId = POS_DATA_ID;
    m_Header.MsgLen = POS_MSG_LEN;
    m_Header.Requested_Visibility = 100;
    m_Header.Replyport = 0;
    const QByteArray CallSign = pCallSign.toLatin1().left(sizeof(m_Header.CallSign) - 1);
    memset(m_Header.CallSign, 0, sizeof(m_Header.CallSign));
    memcpy(m_Header.CallSign, CallSign.constData(), static_cast<size_t>(CallSign.size()));
    const QByteArray Model = pModel.toLatin1().left(sizeof(m_Pos.ModelName) - 1);
    memset(m_Pos.ModelName, 0, sizeof(m_Pos.ModelName));
    memcpy(m_Pos.ModelName, Model.constData(), static_cast<size_t>(Model.size()));
    m_Props.clear();
    m_Sane = true;
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
    FGOrientationToEuler(pData.Longitude, pData.Latitude, m_Pos.Orientation, pData.Heading, pData.Pitch, pData.Bank);
    // Body frame velocity (m/s)
    const double Speed = sqrt(static_cast<double>(m_Pos.LinearVelocity[0]*m_Pos.LinearVelocity[0] +
                                                  m_Pos.LinearVelocity[1]*m_Pos.LinearVelocity[1] +
                                                  m_Pos.LinearVelocity[2]*m_Pos.LinearVelocity[2]));
    pData.TASSpeed = Speed / SG_KT_TO_MPS;
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
    // Time of the sender : FlightGear interpolates the positions with it
    m_Pos.Time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
    m_Pos.Lag = 0.1;
    GeodToCart(pData.Longitude, pData.Latitude, pData.Altitude, m_Pos.Position[0], m_Pos.Position[1], m_Pos.Position[2]);
    EulerToFGOrientation(pData.Longitude, pData.Latitude, pData.Heading, pData.Pitch, pData.Bank, m_Pos.Orientation);
    // Body frame : x forward
    m_Pos.LinearVelocity[0] = static_cast<float>(pData.TASSpeed * SG_KT_TO_MPS);
    m_Pos.LinearVelocity[1] = 0;
    m_Pos.LinearVelocity[2] = 0;
    for (int i=0; i<3; i++)
    {
        m_Pos.AngularVelocity[i] = 0;
        m_Pos.LinearAccel[i] = 0;
        m_Pos.AngularAccel[i] = 0;
    }
    return true;
}

///
/// \brief FGDatagram::GetProperty
/// \param pStream
/// \return true if property was succesfully readed, else false
///
bool FGDatagram::GetProperty(QDataStream& pStream, FGPropertyData& pPropData)
{
    quint16 id;
    bool Decoded=false;
    bool short_int_encoded = true;
    pStream >> id;
    //Detect if we have 4 Byte int ID format
    if( id == 0)
    {
        short_int_encoded =false;
        pStream >> id;
    }

    const IdPropertyList* Item = FindProperty(id);
    if (Item!=nullptr)
    {
        pPropData.id=Item->id;
        pPropData.type = Item->type;
        switch (pPropData.type)
        {
            case BOOL:
            {
                if (id >= BOOLARRAY_START_ID && id<= BOOLARRAY_END_ID)
                {
                    quint32 val;
                    pStream >> val;
                    for (quint16 bitidx = 0; bitidx <= 30; bitidx++)
                    {
                        // ensure that this property is in the master list.
                        const IdPropertyList* plistBool = FindProperty(id + bitidx);

                        if (plistBool)
                        {
                            pPropData.id = id + bitidx;
                            pPropData.int_value = (val & (1 << bitidx)) != 0;
                            pPropData.type = BOOL;
                            Decoded=true;;
                        }
                    }
                }
                break;
            }
            case INT:
            case LONG:
            {
                if (short_int_encoded)
                {
                    qint16 Value;
                    pStream >> Value;
                    pPropData.int_value = Value;
                    Decoded = true;
                }
                else
                {
                    pStream >> pPropData.int_value;
                    Decoded = true;
                }
                break;
            }
            case FLOAT:
            case DOUBLE:
            {
                if (short_int_encoded)
                {
                    qint16 Value;
                    pStream >> Value;
                    switch (Item->TransmitAs)
                    {
                        case TT_SHORT_FLOAT_1:
                        {
                            pPropData.float_value = static_cast<float>(Value / 10.0);
                            Decoded=true;
                            break;
                        }
                        case TT_SHORT_FLOAT_2:
                        {
                            pPropData.float_value = static_cast<float>(Value / 100.0);
                            Decoded=true;
                            break;
                        }
                        case TT_SHORT_FLOAT_3:
                        {
                            pPropData.float_value = static_cast<float>(Value / 1000.0);
                            Decoded=true;
                            break;
                        }
                        case TT_SHORT_FLOAT_4:
                        {
                            pPropData.float_value = static_cast<float>(Value / 10000.0);
                            Decoded=true;
                            break;
                        }
                        case TT_SHORT_FLOAT_NORM:
                        {
                            pPropData.float_value = static_cast<float>(Value / 32767.0);
                            Decoded=true;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                else
                {
                    pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
                    float Value;
                    pStream >> Value;
                    pPropData.float_value = Value;
                    Decoded=true;
                }
                break;
            }
            case STRING:
            case UNSPECIFIED:
            {
                if (short_int_encoded)
                {
                    quint16 Len;
                    pStream >> Len;
                    pPropData.string_value = QByteArray(Len, '\0');
                    pStream.readRawData(pPropData.string_value.data(),Len);
                }
                else
                {
                    quint32 Len;
                    pStream >> Len;
                    if (Len > MAX_TEXT_SIZE)
                        Len = MAX_TEXT_SIZE;
                    pPropData.string_value = QByteArray(static_cast<qsizetype>(Len), '\0');
                    if (Len>0) pStream.readRawData(pPropData.string_value.data(),static_cast<int>(Len));
                }
                Decoded=true;
                break;
            }
            default:
            {
                pStream >> pPropData.float_value;
                Decoded=true;
                break;
            }
        }
    }
    return Decoded;
}

const IdPropertyList* FGDatagram::FindProperty(unsigned short pId)
{
  std::pair<const IdPropertyList*, const IdPropertyList*> result
    = std::equal_range(sIdPropertyList, sIdPropertyList + numProperties, pId,
                       ComparePropertyId());
  if (result.first == result.second) {
    return nullptr;
  } else {
    return result.first;
  }
}
