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
 * fgdatagram.h is part of FF2Play project
 *
 * This is coder/decoder class for FlightGear Datagram
 * **************************************************************************/

#ifndef FGDATAGRAM_H
#define FGDATAGRAM_H

#include <QVector>
#include <QDataStream>

#include "tools/calculation.h"
#include "sim/aircraftstate.h"

///
/// Imported from SimGear of FlightGear Project
/// simgear/src/props/props.hxx
///
namespace simgear
{
    namespace props
    {
        /**
         * The possible types of an SGPropertyNode. Types that appear after
         * EXTENDED are not stored in the SGPropertyNode itself.
         */
        enum Type {
            NONE = 0, /**< The node hasn't been assigned a value yet. */
            ALIAS, /**< The node "points" to another node. */
            BOOL,
            INT,
            LONG,
            FLOAT,
            DOUBLE,
            STRING,
            UNSPECIFIED,
            EXTENDED, /**< The node's value is not stored in the property;
                       * the actual value and type is retrieved from an
                       * SGRawValue node. This type is never returned by @see
                       * SGPropertyNode::getType.
                       */
            // Extended properties
            VEC3D,
            VEC4D
        };
    }
}

namespace ffs2play
{
    // magic value for messages
    const uint32_t MSG_MAGIC = 0x46474653;  // "FGFS"
    // protocoll version
    const uint32_t PROTO_VER = 0x00010001;  // 1.1

    struct FGHeader
    {
        quint32 Magic;
        quint32 Version;
        quint32 MsgId;
        quint32 MsgLen;
        quint32 Requested_Visibility;
        quint32 Replyport;
        char    CallSign[8];
    };

    struct FGPos
    {
        char    ModelName[96];
        double  Time;
        double  Lag;
        double  Position[3];
        float   Orientation[3];
        float   LinearVelocity[3];
        float   AngularVelocity[3];
        float   LinearAccel[3];
        float   AngularAccel[3];
    };

    struct FGPropertyData
    {
        unsigned id;
        // While the type isn't transmitted, it is needed for the destructor
        simgear::props::Type type;
        union
        {
            int int_value;
            float float_value;
            char* string_value;
        };
        FGPropertyData() : string_value(nullptr) {}
        ~FGPropertyData()
        {
            if ((type == simgear::props::STRING) || (type == simgear::props::UNSPECIFIED))
            {
                delete [] string_value;
            }
        }
    };

    class FGDatagram
    {
    public:
                                FGDatagram();
                                ~FGDatagram();
        void                    Read(QDataStream& pStream);
        void                    Write(QDataStream& pStream);
        bool                    IsSane();
        bool                    GetHeader(FGHeader& pHeader);
        bool                    SetHeader(const FGHeader& pHeader);
        bool                    GetPos(FGPos& pPos);
        bool                    SetPos(FGPos& pPos);
        bool                    GetAirData(CAircraftState& pData);
        bool                    SetAirData(const CAircraftState& pData);

    private:
        FGHeader                m_Header;
        FGPos                   m_Pos;
        QVector<FGPropertyData> m_Props;
        bool                    m_Sane;
    };
}

#endif // FGDATAGRAM_H
