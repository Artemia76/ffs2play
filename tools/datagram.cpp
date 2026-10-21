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

/****************************************************************************
** datagram.cpp is part of FF2Play project
**
** These functions provide standard stream conversion to work with QTStream
** properly
****************************************************************************/

#include "datagram.h"

#include <QProtobufSerializer>

///
/// \brief StringFromStream
/// \param pStream
/// \return
///
QString StringFromStream(QDataStream& pStream)
{
    quint8 Len;
    pStream >> Len;
    QByteArray Buffer(Len, '\0');
    pStream.readRawData(Buffer.data(),Len);
    // As the former C string conversion : stops at the first null byte
    return QString::fromUtf8(Buffer.constData(), qstrnlen(Buffer.constData(), Buffer.size()));
}

///
/// \brief StringToStream
/// \param pStream
/// \param pString
///
void StringToStream(QDataStream& pStream,const QString& pString)
{
    QByteArray Utf8 = pString.toUtf8();
    // The length is one byte : truncate to 255 bytes without cutting a UTF-8 sequence
    if (Utf8.size() > 255)
    {
        qsizetype Len = 255;
        while ((Len > 0) && ((static_cast<quint8>(Utf8[Len]) & 0xC0) == 0x80)) --Len;
        Utf8.truncate(Len);
    }
    pStream << static_cast<quint8>(Utf8.size());
    pStream.writeRawData(Utf8.constData(),static_cast<int>(Utf8.size()));
}

///
/// \brief ProtobufToStream
/// \param pStream
/// \param pMessage
///
void ProtobufToStream(QDataStream& pStream,const QProtobufMessage& pMessage)
{
    const QByteArray buffer = ProtobufSerialize(pMessage);
    pStream.writeRawData(buffer.constData(),static_cast<int>(buffer.size()));
}

///
/// \brief ProtobufSerialize
/// \param pMessage
/// \return protobuf wire format of pMessage
///
QByteArray ProtobufSerialize(const QProtobufMessage& pMessage)
{
    QProtobufSerializer serializer;
    return pMessage.serialize(&serializer);
}

///
/// \brief ProtobufParse
/// \param pMessage
/// \param pData protobuf wire format
/// \return true on success
///
bool ProtobufParse(QProtobufMessage& pMessage, QByteArrayView pData)
{
    QProtobufSerializer serializer;
    return pMessage.deserialize(&serializer, pData);
}

QString CStringFromStream(QDataStream& pStream, size_t pLen)
{
    QByteArray Buffer(static_cast<qsizetype>(pLen), '\0');
    pStream.readRawData(Buffer.data(),static_cast<int>(pLen));
    // Fixed size field : the string may fill it without null terminator
    return QString::fromLocal8Bit(Buffer.constData(), qstrnlen(Buffer.constData(), Buffer.size()));
}
