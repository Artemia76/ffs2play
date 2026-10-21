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
#include <QIODevice>
#include <QRegularExpression>
#include <cstring>

#include "fgdatagram.h"

using namespace simgear;

namespace
{
/// m/s to ft/min
constexpr double MPS_TO_FPM = 196.8503937;

/// Body frame (x forward, y right, z down) to local frame (north, east, down),
/// FlightGear angles : heading, pitch nose up, roll right wing down (radians)
void BodyToNED(double pPsi, double pTheta, double pPhi, const double pBody[3], double pNED[3])
{
    const double cPsi = cos(pPsi), sPsi = sin(pPsi);
    const double cTheta = cos(pTheta), sTheta = sin(pTheta);
    const double cPhi = cos(pPhi), sPhi = sin(pPhi);
    const double M[3][3] = {
        {cTheta * cPsi, sPhi * sTheta * cPsi - cPhi * sPsi, cPhi * sTheta * cPsi + sPhi * sPsi},
        {cTheta * sPsi, sPhi * sTheta * sPsi + cPhi * cPsi, cPhi * sTheta * sPsi - sPhi * cPsi},
        {-sTheta,       sPhi * cTheta,                      cPhi * cTheta}};
    for (int i = 0; i < 3; i++) pNED[i] = M[i][0] * pBody[0] + M[i][1] * pBody[1] + M[i][2] * pBody[2];
}

/// Inverse of BodyToNED (transposed matrix)
void NEDToBody(double pPsi, double pTheta, double pPhi, const double pNED[3], double pBody[3])
{
    const double cPsi = cos(pPsi), sPsi = sin(pPsi);
    const double cTheta = cos(pTheta), sTheta = sin(pTheta);
    const double cPhi = cos(pPhi), sPhi = sin(pPhi);
    const double M[3][3] = {
        {cTheta * cPsi, sPhi * sTheta * cPsi - cPhi * sPsi, cPhi * sTheta * cPsi + sPhi * sPsi},
        {cTheta * sPsi, sPhi * sTheta * sPsi + cPhi * cPsi, cPhi * sTheta * sPsi - sPhi * cPsi},
        {-sTheta,       sPhi * cTheta,                      cPhi * cTheta}};
    for (int i = 0; i < 3; i++) pBody[i] = M[0][i] * pNED[0] + M[1][i] * pNED[1] + M[2][i] * pNED[2];
}

double Clamp(double pValue, double pMin, double pMax)
{
    return qBound(pMin, pValue, pMax);
}
}

///
/// \brief FGToSquawkMode
/// FlightGear transponder mode (0 off, 1 standby, 2 test, 3 ground, 4 on, 5 altitude)
/// to the FFS2Play mode of the X-Plane connector (0 off, 1 standby, 2 on, 3 altitude,
/// 4 test, 5 ground), and back : the conversion is its own inverse
/// \param pMode
/// \return
///
int simgear::FGToSquawkMode(int pMode)
{
    switch (pMode)
    {
        case 2: return 4;
        case 3: return 5;
        case 4: return 2;
        case 5: return 3;
        default: return pMode;
    }
}

///
/// \brief FGGenericMap::fromPropertyList
/// Finds the lights and the doors among the generic properties declared in the -set.xml
/// of an aircraft (ex. c172p : <int n="12" alias="/sim/model/c172p/lighting/beacon/enabled"/>).
/// An aircraft which sets its generic properties by Nasal has nothing to find here.
/// \param pProps
/// \return
///
FGGenericMap FGGenericMap::fromPropertyList(const CFGPropertyList& pProps)
{
    FGGenericMap Map;
    static const QRegularExpression Generic("^sim/multiplay/generic/(float|int|short|bool)(\\[\\d+\\])?$");
    // Sorted by property id : the first declared light wins, the doors keep their order
    QMap<quint16, QString> Targets;
    const QHash<QString, QString> Aliases = pProps.aliases();
    for (auto it = Aliases.constBegin(); it != Aliases.constEnd(); ++it)
    {
        const QRegularExpressionMatch Match = Generic.match(it.key());
        if (!Match.hasMatch()) continue;
        // FlightGear names the index 0 of the generic properties
        const QString Name = Match.captured(2).isEmpty() ? it.key() + "[0]" : it.key();
        const quint16 Id = FGDatagram::FindPropertyId(Name);
        if (Id != 0) Targets.insert(Id, it.value().toLower());
    }
    int Door = 0;
    for (auto it = Targets.constBegin(); it != Targets.constEnd(); ++it)
    {
        const QString& Target = it.value();
        const quint16 Id = it.key();
        const bool IsFloat = (FGDatagram::FindProperty(Id)->type == FLOAT);
        // A path element naming a door ("rightdoor"), not a group ("door-positions/leftwindow")
        bool IsDoor = false;
        for (const QString& Element : Target.split('/'))
        {
            if (Element.contains("door") && !Element.contains("positions") && !Element.endsWith("doors")) IsDoor = true;
        }
        if (IsDoor)
        {
            if (IsFloat && (Door < DoorCount)) Map.Doors[Door++] = Id;
            continue;
        }
        if (!Target.contains("light")) continue;
        quint16* Light = nullptr;
        if (Target.contains("landing")) Light = &Map.LandingLight;
        else if (Target.contains("strobe")) Light = &Map.StrobeLight;
        else if (Target.contains("beacon")) Light = &Map.BeaconLight;
        else if (Target.contains("nav")) Light = &Map.NavLight;
        else if (Target.contains("recognition")) Light = &Map.RecoLight;
        if (Light && (*Light == 0)) *Light = Id;
    }
    return Map;
}

///
/// \brief FGGenericMap::isEmpty
/// \return
///
bool FGGenericMap::isEmpty() const
{
    for (quint16 Door : Doors)
    {
        if (Door != 0) return false;
    }
    return (LandingLight == 0) && (StrobeLight == 0) && (BeaconLight == 0) && (NavLight == 0) && (RecoLight == 0);
}

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
    // xdr_data_t pad of T_PositionMsg (FlightGear mpmessages.hxx) : properties start at byte 232.
    // V2_PAD_MAGIC with the 2017.2 protocol, 0 with the first one.
    char Dummy[4];
    pStream.readRawData(&Dummy[0],sizeof(Dummy));
#ifdef FFS_DEBUG
    m_Log.log(QString::asprintf("FGManager : Orientation (angle axis) = %f , %f , %f",
        static_cast<double>(m_Pos.Orientation[0]),
        static_cast<double>(m_Pos.Orientation[1]),
        static_cast<double>(m_Pos.Orientation[2])
        ),Qt::darkBlue,LEVEL_VERBOSE);
#endif
    //Reading Property data : an unknown property ends the reading, as in FlightGear
    m_Props.clear();
    while (!pStream.atEnd() && (pStream.status() == QDataStream::Ok))
    {
        if (!ReadProperty(pStream)) break;
    }
#ifdef FFS_DEBUG
    // One line for the message : about 150 properties at 10 Hz
    QStringList Values;
    for (const FGPropertyData& Prop : std::as_const(m_Props))
    {
        QString Value;
        switch (Prop.type)
        {
        case FLOAT:
        case DOUBLE:
            Value = QString::number(static_cast<double>(Prop.float_value));
            break;
        case STRING:
        case UNSPECIFIED:
            Value = "\"" + QString::fromUtf8(Prop.string_value) + "\"";
            break;
        default:
            Value = QString::number(Prop.int_value);
            break;
        }
        Values << QString::number(Prop.id) + "=" + Value;
    }
    m_Log.log("FGManager : Properties " + Values.join(" "),Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

///
/// \brief FGDatagram::Write
/// Properties with the first protocol (pad 0) : 4 bytes id, then a 4 bytes value,
/// understood by every FlightGear version
/// \param pStream
///
void FGDatagram::Write(QDataStream& pStream)
{
    QByteArray Properties;
    {
        QDataStream PropStream(&Properties, QIODevice::WriteOnly);
        WriteProperties(PropStream);
    }
    m_Header.MsgLen = POS_MSG_LEN + static_cast<quint32>(Properties.size());
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
    pStream.writeRawData(Properties.constData(), static_cast<int>(Properties.size()));
}

///
/// \brief FGDatagram::WriteProperties
/// Numeric properties only. The generic booleans are sent by blocks of 31 in one int.
/// \param pStream
///
void FGDatagram::WriteProperties(QDataStream& pStream) const
{
    pStream.setByteOrder(QDataStream::BigEndian);
    pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    QMap<quint32, quint32> BoolBlocks;
    for (const FGPropertyData& Prop : m_Props)
    {
        if ((Prop.id >= BOOLARRAY_START_ID) && (Prop.id < BOOLARRAY_END_ID + BOOLARRAY_BLOCKSIZE))
        {
            const quint32 Block = BOOLARRAY_BASE_1 + ((Prop.id - BOOLARRAY_BASE_1) / BOOLARRAY_BLOCKSIZE) * BOOLARRAY_BLOCKSIZE;
            quint32& Bits = BoolBlocks[Block];
            if (Prop.int_value) Bits |= (1u << (Prop.id - Block));
            continue;
        }
        switch (Prop.type)
        {
        case BOOL:
        case INT:
        case LONG:
            pStream << quint32(Prop.id) << qint32(Prop.int_value);
            break;
        case FLOAT:
        case DOUBLE:
            pStream << quint32(Prop.id) << Prop.float_value;
            break;
        default:
            break;
        }
    }
    for (auto it = BoolBlocks.constBegin(); it != BoolBlocks.constEnd(); ++it)
    {
        pStream << it.key() << it.value();
    }
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
/// Only what FlightGear transmits : the values without property in the message
/// (throttle, parking brake, fuel...) are left unchanged
/// \param pData
/// \param pMap generic properties of the lights and doors of the aircraft
/// \return
///
bool FGDatagram::GetAirData(CAircraftState& pData, const FGGenericMap& pMap)
{
    if (!m_Sane) return false;
    CartToGeod (m_Pos.Position[0],m_Pos.Position[1], m_Pos.Position[2], pData.Longitude,pData.Latitude, pData.Altitude);
    FGOrientationToEuler(pData.Longitude, pData.Latitude, m_Pos.Orientation, pData.Heading, pData.Pitch, pData.Bank);
    // Body frame velocity relative to the earth (m/s)
    const double Body[3] = {static_cast<double>(m_Pos.LinearVelocity[0]),
                            static_cast<double>(m_Pos.LinearVelocity[1]),
                            static_cast<double>(m_Pos.LinearVelocity[2])};
    const double Speed = sqrt(Body[0] * Body[0] + Body[1] * Body[1] + Body[2] * Body[2]);
    pData.TASSpeed = Speed / SG_KT_TO_MPS;
    // FFS2Play pitch is positive nose down and bank positive left wing down
    double NED[3];
    BodyToNED(pData.Heading * SGD_DEGREES_TO_RADIANS, -pData.Pitch * SGD_DEGREES_TO_RADIANS, -pData.Bank * SGD_DEGREES_TO_RADIANS, Body, NED);
    pData.GroundSpeed = sqrt(NED[0] * NED[0] + NED[1] * NED[1]) / SG_KT_TO_MPS;
    pData.Vario = -NED[2] * MPS_TO_FPM;
    pData.Title = m_Pos.ModelName;

    // Flight controls : FlightGear elevator positive nose down, X-Plane connector positive nose up
    if (HasProperty(FGProp::LeftAileron)) pData.AileronPos = PropertyValue(FGProp::LeftAileron);
    if (HasProperty(FGProp::Elevator)) pData.ElevatorPos = -PropertyValue(FGProp::Elevator);
    if (HasProperty(FGProp::Rudder)) pData.RudderPos = PropertyValue(FGProp::Rudder);
    // Flaps index 0 to 3, as the X-Plane connector
    if (HasProperty(FGProp::Flaps)) pData.FlapsIndex = qRound(Clamp(PropertyValue(FGProp::Flaps), 0.0, 1.0) * 3.0);
    if (HasProperty(FGProp::SpeedbrakePos)) pData.SpoilerPos = Clamp(PropertyValue(FGProp::SpeedbrakePos), 0.0, 1.0);
    // Gear : position of the first gear (controls/gear/gear-down stays 0 on a fixed gear)
    if (HasProperty(FGProp::GearPosition)) pData.GearPos = (PropertyValue(FGProp::GearPosition) >= 0.5) ? 1 : 0;
    bool HasCompression = false;
    bool Compressed = false;
    for (int i = 0; i < FGProp::GearCount; i++)
    {
        const quint16 Id = FGProp::GearCompression + 10 * i;
        if (!HasProperty(Id)) continue;
        HasCompression = true;
        if (PropertyValue(Id) > 0.0) Compressed = true;
    }
    if (HasCompression) pData.OnGround = Compressed;
    // Engines : running while turning. Only the engines of the aircraft, the others
    // can carry anything (the c172p sends its amphibious gear in engine[3] to [6])
    int* const States[4] = {&pData.StateEng1, &pData.StateEng2, &pData.StateEng3, &pData.StateEng4};
    const int Engines = qBound(1, pData.NumEngine, 4);
    for (int i = 0; i < Engines; i++)
    {
        const quint16 Rpm = FGProp::EngineRpm + 10 * i;
        const quint16 N1 = FGProp::EngineN1 + 10 * i;
        if (!HasProperty(Rpm) && !HasProperty(N1)) continue;
        *States[i] = ((PropertyValue(Rpm) > 0.0) || (PropertyValue(N1) > 0.0)) ? 1 : 0;
    }
    // Lights : nav lights in the static list, the others in the generic properties
    if (pMap.NavLight && HasProperty(pMap.NavLight)) pData.NavLight = (PropertyValue(pMap.NavLight) > 0.5) ? 1 : 0;
    else if (HasProperty(FGProp::NavLights)) pData.NavLight = (PropertyValue(FGProp::NavLights) > 0.5) ? 1 : 0;
    const std::pair<quint16, int*> Lights[] = {{pMap.LandingLight, &pData.LandingLight}, {pMap.StrobeLight, &pData.StrobeLight},
                                               {pMap.BeaconLight, &pData.BeaconLight}, {pMap.RecoLight, &pData.RecoLight}};
    for (const auto& Light : Lights)
    {
        if (Light.first && HasProperty(Light.first)) *Light.second = (PropertyValue(Light.first) > 0.5) ? 1 : 0;
    }
    // Doors in percent. The canopy when the aircraft has no door
    int* const Doors[FGGenericMap::DoorCount] = {&pData.Door1Pos, &pData.Door2Pos, &pData.Door3Pos, &pData.Door4Pos};
    bool HasDoor = false;
    for (int i = 0; i < FGGenericMap::DoorCount; i++)
    {
        if (!pMap.Doors[i] || !HasProperty(pMap.Doors[i])) continue;
        HasDoor = true;
        *Doors[i] = qRound(Clamp(PropertyValue(pMap.Doors[i]), 0.0, 1.0) * 100.0);
    }
    if (!HasDoor && HasProperty(FGProp::Canopy)) pData.Door1Pos = qRound(Clamp(PropertyValue(FGProp::Canopy), 0.0, 1.0) * 100.0);
    // Transponder : the code (decimal, as X-Plane) only when it replies
    if (HasProperty(FGProp::TransponderMode)) pData.SquawkMode = FGToSquawkMode(static_cast<int>(PropertyValue(FGProp::TransponderMode)));
    if (HasProperty(FGProp::TransponderId) && (PropertyValue(FGProp::TransponderId) >= 0.0))
        pData.Squawk = static_cast<int>(PropertyValue(FGProp::TransponderId));
    if (HasProperty(FGProp::TransponderAirspeed) && (PropertyValue(FGProp::TransponderAirspeed) >= 0.0))
        pData.IASSpeed = PropertyValue(FGProp::TransponderAirspeed);
    return true;
}

///
/// \brief FGDatagram::SetAirData
/// \param pData
/// \param pMap generic properties of the lights and doors of the displayed model
/// \return
///
bool FGDatagram::SetAirData(const CAircraftState& pData, const FGGenericMap& pMap)
{
    if (!m_Sane) return false;
    // Time of the sender : FlightGear interpolates the positions with it
    m_Pos.Time = static_cast<double>(QDateTime::currentMSecsSinceEpoch()) / 1000.0;
    m_Pos.Lag = 0.1;
    GeodToCart(pData.Longitude, pData.Latitude, pData.Altitude, m_Pos.Position[0], m_Pos.Position[1], m_Pos.Position[2]);
    EulerToFGOrientation(pData.Longitude, pData.Latitude, pData.Heading, pData.Pitch, pData.Bank, m_Pos.Orientation);
    // Body frame velocity from the ground speed along the heading and the vertical speed,
    // FlightGear extrapolates the position with it
    const double Psi = pData.Heading * SGD_DEGREES_TO_RADIANS;
    const double GroundSpeed = ((pData.GroundSpeed > 0.0) ? pData.GroundSpeed : pData.TASSpeed) * SG_KT_TO_MPS;
    const double NED[3] = {GroundSpeed * cos(Psi), GroundSpeed * sin(Psi), -pData.Vario / MPS_TO_FPM};
    double Body[3];
    NEDToBody(Psi, -pData.Pitch * SGD_DEGREES_TO_RADIANS, -pData.Bank * SGD_DEGREES_TO_RADIANS, NED, Body);
    for (int i=0; i<3; i++)
    {
        m_Pos.LinearVelocity[i] = static_cast<float>(Body[i]);
        m_Pos.AngularVelocity[i] = 0;
        m_Pos.LinearAccel[i] = 0;
        m_Pos.AngularAccel[i] = 0;
    }

    // Flight controls
    const double Aileron = Clamp(pData.AileronPos, -1.0, 1.0);
    SetProperty(FGProp::LeftAileron, Aileron);
    SetProperty(FGProp::RightAileron, Aileron);
    SetProperty(FGProp::Elevator, Clamp(-pData.ElevatorPos, -1.0, 1.0));
    SetProperty(FGProp::Rudder, Clamp(pData.RudderPos, -1.0, 1.0));
    SetProperty(FGProp::Flaps, Clamp(pData.FlapsIndex / 3.0, 0.0, 1.0));
    const double Spoilers = Clamp(pData.SpoilerPos, 0.0, 1.0);
    SetProperty(FGProp::SpeedbrakePos, Spoilers);
    SetProperty(FGProp::Speedbrake, Spoilers);
    SetProperty(FGProp::Spoilers, Spoilers);
    // Gear
    const double Gear = pData.GearPos ? 1.0 : 0.0;
    SetProperty(FGProp::GearDown, Gear);
    for (int i = 0; i < FGProp::GearCount; i++) SetProperty(FGProp::GearPosition + 10 * i, Gear);
    // Lights
    SetProperty(FGProp::NavLights, pData.NavLight ? 1.0 : 0.0);
    const std::pair<quint16, int> Lights[] = {{pMap.LandingLight, pData.LandingLight}, {pMap.StrobeLight, pData.StrobeLight},
                                              {pMap.BeaconLight, pData.BeaconLight}, {pMap.NavLight, pData.NavLight},
                                              {pMap.RecoLight, pData.RecoLight}};
    for (const auto& Light : Lights)
    {
        if (Light.first) SetProperty(Light.first, Light.second ? 1.0 : 0.0);
    }
    // Doors
    const int Doors[FGGenericMap::DoorCount] = {pData.Door1Pos, pData.Door2Pos, pData.Door3Pos, pData.Door4Pos};
    for (int i = 0; i < FGGenericMap::DoorCount; i++)
    {
        if (pMap.Doors[i]) SetProperty(pMap.Doors[i], Clamp(Doors[i] / 100.0, 0.0, 1.0));
    }
    // Transponder : code and altitude only when it replies
    const int Mode = pData.SquawkMode;
    const bool Replies = (Mode == 2) || (Mode == 3) || (Mode == 5);
    SetProperty(FGProp::TransponderMode, FGToSquawkMode(Mode));
    SetProperty(FGProp::TransponderId, Replies ? pData.Squawk : FGProp::TransponderNone);
    SetProperty(FGProp::TransponderAltitude, (Mode == 3) ? qRound(pData.Altitude) : FGProp::TransponderNone);
    SetProperty(FGProp::TransponderGround, pData.OnGround ? 1.0 : 0.0);
    SetProperty(FGProp::TransponderAirspeed, Replies ? qRound(pData.IASSpeed) : FGProp::TransponderNone);
    return true;
}

///
/// \brief FGDatagram::HasProperty
/// \param pId
/// \return true if the received message has this property
///
bool FGDatagram::HasProperty(quint16 pId) const
{
    return m_Props.contains(pId);
}

///
/// \brief FGDatagram::PropertyValue
/// \param pId
/// \param pDefault value of a missing or text property
/// \return
///
double FGDatagram::PropertyValue(quint16 pId, double pDefault) const
{
    const auto it = m_Props.constFind(pId);
    if (it == m_Props.constEnd()) return pDefault;
    switch (it->type)
    {
    case FLOAT:
    case DOUBLE:
        return static_cast<double>(it->float_value);
    case BOOL:
    case INT:
    case LONG:
        return it->int_value;
    default:
        return pDefault;
    }
}

///
/// \brief FGDatagram::SetProperty
/// Sets a numeric property of the static list with its type
/// \param pId
/// \param pValue
///
void FGDatagram::SetProperty(quint16 pId, double pValue)
{
    const IdPropertyList* Item = FindProperty(pId);
    if (Item == nullptr) return;
    FGPropertyData& Prop = m_Props[pId];
    Prop.id = pId;
    Prop.type = Item->type;
    if ((Item->type == FLOAT) || (Item->type == DOUBLE)) Prop.float_value = static_cast<float>(pValue);
    else Prop.int_value = static_cast<int>(std::lround(pValue));
}

///
/// \brief FGDatagram::Properties
/// \return
///
const QMap<quint16, FGPropertyData>& FGDatagram::Properties() const
{
    return m_Props;
}

///
/// \brief FGDatagram::ReadProperty
/// Port of FGMultiplayMgr::ProcessPosMsg (FlightGear 2024.1). A 32 bits word with a
/// non zero upper half holds the id and a 16 bits value (2017.2 protocol), else the
/// word is the id and the value follows. The 2017.2 strings are not padded : the
/// next property can start on any byte.
/// \param pStream
/// \return false if the property is unknown or the message is truncated
///
bool FGDatagram::ReadProperty(QDataStream& pStream)
{
    quint32 Word;
    pStream >> Word;
    if (pStream.status() != QDataStream::Ok) return false;
    quint16 Id;
    int ShortValue = 0;
    const bool ShortIntEncoded = (Word & 0xffff0000u) != 0;
    if (ShortIntEncoded)
    {
        Id = static_cast<quint16>(Word >> 16);
        ShortValue = static_cast<qint16>(Word & 0xffffu);
    }
    else
    {
        if (Word > 0xffffu) return false;
        Id = static_cast<quint16>(Word);
    }
    const IdPropertyList* Item = FindProperty(Id);
    if (Item == nullptr) return false;

    FGPropertyData Prop;
    Prop.id = Id;
    Prop.type = Item->type;
    // Launchbar state, 2017.2 encoding : property 120 for the string property 108
    if (Id == 120)
    {
        static const char* const States[] = {"Engaged", "Launching", "Completed", "Disengaged"};
        Prop.id = 108;
        Prop.type = STRING;
        if ((ShortValue >= 0) && (ShortValue < 4)) Prop.string_value = States[ShortValue];
        m_Props.insert(Prop.id, Prop);
        return true;
    }
    switch (Item->type)
    {
    case BOOL:
        if ((Id >= BOOLARRAY_START_ID) && (Id <= BOOLARRAY_END_ID))
        {
            quint32 Bits;
            pStream >> Bits;
            for (quint16 Bit = 0; Bit <= 30; Bit++)
            {
                const IdPropertyList* BoolItem = FindProperty(Id + Bit);
                if (BoolItem == nullptr) continue;
                FGPropertyData BoolProp;
                BoolProp.id = Id + Bit;
                BoolProp.type = BOOL;
                BoolProp.int_value = (Bits & (1u << Bit)) != 0;
                m_Props.insert(BoolProp.id, BoolProp);
            }
            return pStream.status() == QDataStream::Ok;
        }
        [[fallthrough]];
    case INT:
    case LONG:
        if (ShortIntEncoded)
        {
            Prop.int_value = ShortValue;
        }
        else
        {
            qint32 Value;
            pStream >> Value;
            Prop.int_value = Value;
        }
        break;
    case FLOAT:
    case DOUBLE:
        if (ShortIntEncoded)
        {
            double Scale = 1.0;
            switch (Item->TransmitAs)
            {
            case TT_SHORT_FLOAT_1: Scale = 10.0; break;
            case TT_SHORT_FLOAT_2: Scale = 100.0; break;
            case TT_SHORT_FLOAT_3: Scale = 1000.0; break;
            case TT_SHORT_FLOAT_4: Scale = 10000.0; break;
            case TT_SHORT_FLOAT_NORM: Scale = 32767.0; break;
            default: break;
            }
            Prop.float_value = static_cast<float>(ShortValue / Scale);
        }
        else
        {
            pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            float Value;
            pStream >> Value;
            Prop.float_value = Value;
        }
        break;
    case STRING:
    case UNSPECIFIED:
        if (ShortIntEncoded)
        {
            // Length in the word, then the characters
            if ((ShortValue < 0) || (ShortValue > MAX_TEXT_SIZE)) return false;
            Prop.string_value = QByteArray(ShortValue, '\0');
            if (ShortValue > 0 && pStream.readRawData(Prop.string_value.data(), ShortValue) != ShortValue) return false;
        }
        else
        {
            // Length, then each character in a 4 bytes word, padded to a multiple of 4 words
            quint32 Len;
            pStream >> Len;
            if (Len > MAX_TEXT_SIZE) Len = MAX_TEXT_SIZE;
            Prop.string_value.reserve(static_cast<qsizetype>(Len));
            quint32 Count = Len;
            while ((Count % 4) != 0) Count++;
            for (quint32 i = 0; i < Count; i++)
            {
                qint32 Char;
                pStream >> Char;
                if (i < Len) Prop.string_value.append(static_cast<char>(Char));
            }
        }
        break;
    default:
        {
            pStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            float Value;
            pStream >> Value;
            Prop.float_value = Value;
            Prop.type = FLOAT;
        }
        break;
    }
    if (pStream.status() != QDataStream::Ok) return false;
    m_Props.insert(Prop.id, Prop);
    return true;
}

///
/// \brief FGDatagram::FindProperty
/// \param pId
/// \return the property of the static list, nullptr if unknown
///
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

///
/// \brief FGDatagram::FindPropertyId
/// \param pName property path, ex "sim/multiplay/generic/int[12]"
/// \return its id in the static list, 0 if unknown
///
quint16 FGDatagram::FindPropertyId(const QString& pName)
{
    const QByteArray Name = pName.toLatin1();
    for (const IdPropertyList& Item : sIdPropertyList)
    {
        if (Name == Item.name) return Item.id;
    }
    return 0;
}
