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

#include "datagram.h"

namespace ffs2play
{
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

	void StringToStream(QDataStream& pStream,const QString& pString)
	{
		pStream << (quint8)pString.toStdString().length();
		pStream.writeRawData(pString.toStdString().c_str(),pString.toStdString().length());
	}
	void ProtobufToStream(QDataStream& pStream,::google::protobuf::Message& pMessage)
	{
		char* buffer = new char[pMessage.ByteSize()];
		pMessage.SerializeToArray(buffer,pMessage.ByteSize());
		pStream.writeRawData(buffer,pMessage.ByteSize());
		delete[] buffer;
	}
}

