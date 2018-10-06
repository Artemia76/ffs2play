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
 * datagram.cpp is part of FF2Play project
 *
 * These functions provide standard stream conversion to work with QTStream
 * properly
 * **************************************************************************/

#include "datagram.h"

namespace ffs2play
{
    ///
    /// \brief StringFromStream
    /// \param pStream
    /// \return
    ///
    QString StringFromStream(QDataStream& pStream)
    {
        quint8 Len;
        pStream >> Len;
        char* Buffer = new char[Len+1];
        pStream.readRawData(Buffer,Len);
        Buffer[Len]='\0';
        QString Result(Buffer);
        delete[] Buffer;
        return Result;
    }

    ///
    /// \brief StringToStream
    /// \param pStream
    /// \param pString
    ///
    void StringToStream(QDataStream& pStream,const QString& pString)
    {
        pStream << static_cast<quint8>(pString.toStdString().length());
        pStream.writeRawData(pString.toStdString().c_str(),static_cast<int>(pString.toStdString().length()));
    }

    ///
    /// \brief ProtobufToStream
    /// \param pStream
    /// \param pMessage
    ///
    void ProtobufToStream(QDataStream& pStream,::google::protobuf::Message& pMessage)
    {
        char* buffer = new char[static_cast<uint>(pMessage.ByteSize())];
        pMessage.SerializeToArray(buffer,pMessage.ByteSize());
        pStream.writeRawData(buffer,pMessage.ByteSize());
        delete[] buffer;
    }

    QString CStringFromStream(QDataStream& pStream, size_t pLen)
    {
        QString Response;
        char* str = new char[pLen];
        pStream.readRawData(str,static_cast<int>(pLen));
        Response = QString::fromLocal8Bit(str);
        delete[] str;
        return Response;
    }
}

