/****************************************************************************
**
** Copyright (C) 2026 FSFranceSimulateur team.
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
** fgaiparser.cpp is part of FF2Play project
**
** This class scans the FlightGear aircraft (-set.xml files)
** The title of an aircraft is its model path (/sim/model/path), the only
** identification sent by FlightGear in its multiplayer packets
****************************************************************************/

#include "fgaiparser.h"
#include "tools/fgpropertylist.h"

FGAIParser::FGAIParser(QObject *parent) : AIBaseParser(parent)
{
    m_AIFileFilter << "*-set.xml";
    m_FGRoots = findFGRoots();
    searchSimPath();
}

FGAIParser::~FGAIParser()
{
    stopScan();
}

///
/// \brief FGAIParser::userFolders
/// \return the folders where the launcher downloads FlightGear data and aircraft
/// (FlightGear 2024.1 on Windows : %USERPROFILE%\FlightGear)
///
QStringList FGAIParser::userFolders()
{
    QStringList Folders;
    Folders << QDir::homePath() + "/FlightGear";
    Folders << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FlightGear";
#ifndef Q_OS_WIN
    Folders << QDir::homePath() + "/.fgfs";
#endif
    return Folders;
}

///
/// \brief FGAIParser::findFGRoots
/// \return the existing FlightGear data folders (FG_ROOT)
///
QStringList FGAIParser::findFGRoots()
{
    QStringList Candidates;
    const QString EnvRoot = qEnvironmentVariable("FG_ROOT");
    if (!EnvRoot.isEmpty()) Candidates << EnvRoot;
    // Data downloaded by the launcher : <user folder>/Downloads/fgdata_2024_1, newest version first
    for (const QString& Folder : userFolders())
    {
        const QDir Downloads(Folder + "/Downloads");
        const QStringList Datas = Downloads.entryList({"fgdata*"}, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (const QString& Data : Datas) Candidates << Downloads.absoluteFilePath(Data);
    }
#if defined(Q_OS_WIN)
    // Data installed with the program : C:\Program Files\FlightGear 2020.3\data
    for (const QString& ProgramFiles : {qEnvironmentVariable("ProgramFiles"), qEnvironmentVariable("ProgramFiles(x86)")})
    {
        if (ProgramFiles.isEmpty()) continue;
        const QDir Dir(ProgramFiles);
        const QStringList Installs = Dir.entryList({"FlightGear*"}, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed);
        for (const QString& Install : Installs) Candidates << Dir.absoluteFilePath(Install + "/data");
    }
#elif defined(Q_OS_MACOS)
    Candidates << "/Applications/FlightGear.app/Contents/Resources/data";
#else
    Candidates << "/usr/share/games/flightgear" << "/usr/share/flightgear" << "/usr/local/share/flightgear";
#endif
    QStringList Roots;
    for (const QString& Candidate : std::as_const(Candidates))
    {
        const QString Root = QDir::fromNativeSeparators(QDir::cleanPath(Candidate));
        if (QDir(Root + "/Aircraft").exists() && !Roots.contains(Root)) Roots << Root;
    }
    return Roots;
}

///
/// \brief FGAIParser::searchSimPath
/// Registers the aircraft folders of FlightGear not yet in the list : a new version
/// of FlightGear downloads its data to a new folder (fgdata_2024_1, fgdata_2024_2...)
///
void FGAIParser::searchSimPath()
{
    if (!m_Initialized) return;
    const QStringList Known = getPathList();
    QStringList Candidates;
    for (const QString& Root : std::as_const(m_FGRoots)) Candidates << Root + "/Aircraft";
    // Additional aircraft folders given to FlightGear
    const QString EnvAircraft = qEnvironmentVariable("FG_AIRCRAFT");
    if (!EnvAircraft.isEmpty()) Candidates << EnvAircraft.split(QDir::listSeparator(), Qt::SkipEmptyParts);
    // Hangars downloaded by the launcher (Downloads/Aircraft/org.flightgear.fgaddon.stable_2024/Aircraft)
    // and default folder of the aircraft added by hand
    for (const QString& Folder : userFolders())
    {
        Candidates << Folder + "/Downloads/Aircraft" << Folder + "/Custom Aircraft" << Folder + "/Aircraft";
    }
    QStringList Added;
    for (const QString& Candidate : std::as_const(Candidates))
    {
        const QString Path = QDir::fromNativeSeparators(QDir::cleanPath(Candidate));
        if (!QDir(Path).exists() || Added.contains(Path) || Known.contains(Path, Qt::CaseInsensitive)) continue;
        if (addPath(Path))
        {
            Added << Path;
#ifdef FFS_DEBUG
            m_log.log("AIMapping : Path found for " + m_Table + " = " + Path, Qt::darkYellow,LEVEL_NORMAL);
#endif
        }
    }
#ifdef FFS_DEBUG
    if (Added.isEmpty()) m_log.log("AIMapping : No new path found for " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
}

///
/// \brief FGAIParser::DBFileUpdate
/// \param pKey
/// \param pPath
///
void FGAIParser::DBFileUpdate(const QString &pKey, const QString &pPath)
{
    if ((!m_Initialized) || m_Table.isEmpty()) return;
    if (!m_Thread_DB.isOpen()) return;
    QSqlQuery query(m_Thread_DB);
    query.prepare( "SELECT checksum FROM Aircrafts WHERE (checksum = :checksum) and (Sim = :Sim)");
    query.bindValue(":checksum",pKey);
    query.bindValue(":Sim",m_Table);
    if (!query.exec())
    {
        m_log.log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);
        return;
    }
    if (query.next()) return;
#ifdef FFS_DEBUG
    m_log.log("AIMapping : No record was found for " + pKey, Qt::darkYellow,LEVEL_VERBOSE);
#endif
    CFGPropertyList Props;
    if (!Props.read(pPath, m_FGRoots))
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : Not a PropertyList file : " + pPath, Qt::darkYellow,LEVEL_VERBOSE);
#endif
        return;
    }
    // Model path sent by FlightGear in the multiplayer packets
    const QString Title = Props.value("sim/model/path");
    if (Title.isEmpty())
    {
#ifdef FFS_DEBUG
        m_log.log("AIMapping : No model path in " + pPath, Qt::darkYellow,LEVEL_VERBOSE);
#endif
        return;
    }
    const QString Editor = Props.value("sim/author");
    const QString Atc_model = Props.value("aircraft/icao/type");
    // No manufacturer in the FlightGear metadata : the full name of the aircraft is used
    const QString Atc_type = Props.value("sim/description");
    QStringList Tags;
    for (const QString& Tag : Props.values("sim/tags/tag")) Tags << Tag.toLower();

    QString Category = "airplane";
    if (Tags.contains("helicopter")) Category = "helicopter";
    else if (Tags.contains("glider")) Category = "glider";
    else if (Tags.contains("seaplane") || Tags.contains("floats") || Tags.contains("amphibious")) Category = "seaplane";
    else if (Tags.contains("vtol")) Category = "vtol";
    else if (Tags.contains("airliner") || Tags.contains("passenger")) Category = "airliner";

    // Same codes as the other simulators (see AIBaseParser::getAITitleDispo)
    int Engine_Type = 4; // Undefined
    if (Category == "glider") Engine_Type = 2;
    else if (Tags.contains("turboprop")) Engine_Type = 5;
    else if (Tags.contains("jet") || Tags.contains("turbofan")) Engine_Type = 1;
    else if (Tags.contains("turboshaft") || (Category == "helicopter" && Tags.contains("turbine"))) Engine_Type = 3;
    else if (Tags.contains("electric")) Engine_Type = 6;
    else if (Tags.contains("piston")) Engine_Type = 0;

    int nb_moteur = 1;
    if (Tags.contains("twin-engine")) nb_moteur = 2;
    else if (Tags.contains("three-engine")) nb_moteur = 3;
    else if (Tags.contains("four-engine")) nb_moteur = 4;
    else if (Tags.contains("six-engine")) nb_moteur = 6;
    else if (Tags.contains("eight-engine")) nb_moteur = 8;
    else if (Engine_Type == 2) nb_moteur = 0;

#if FFS_DEBUG
    m_log.log("AIMapping : Path = " + pPath,Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : Title = " + Title,Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : Editor = " + Editor,Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : ATCModel = " + Atc_model, Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : ATCType = " + Atc_type, Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : Category = " + Category, Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : EngineType = " + QString::number(Engine_Type), Qt::darkYellow,LEVEL_VERBOSE);
    m_log.log("AIMapping : NBEngine = " + QString::number(nb_moteur), Qt::darkYellow,LEVEL_VERBOSE);
#endif
    // One record per -set.xml file : the variants of an aircraft share the same model path.
    // A modified file replaces its previous record
    query.prepare("DELETE FROM Aircrafts WHERE (Path = :Path) and (Sim = :Sim)");
    query.bindValue(":Path", pPath);
    query.bindValue(":Sim",m_Table);
    const bool Replaced = query.exec() && (query.numRowsAffected() > 0);
    query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, AirLine, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                  "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :AirLine, :Path, :Category, :NbEngine, :EngineType, :CGHeight, :Pitch)");
    query.bindValue(":checksum", pKey);
    query.bindValue(":Sim",m_Table);
    query.bindValue(":Title", Title);
    query.bindValue(":Editor", Editor);
    query.bindValue(":Type", Atc_type);
    query.bindValue(":Model", Atc_model);
    query.bindValue(":AirLine", QString());
    query.bindValue(":Path", pPath);
    query.bindValue(":Category", Category);
    query.bindValue(":NbEngine", nb_moteur);
    query.bindValue(":EngineType", Engine_Type);
    // No static CG height and pitch in the FlightGear metadata
    query.bindValue(":CGHeight", 0.0);
    query.bindValue(":Pitch", 0.0);
    if (!query.exec())
    {
        m_log.log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);
        return;
    }
    if (Replaced) m_NbUpdated++;
    else m_NbAdded++;
}
