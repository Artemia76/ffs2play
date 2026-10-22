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
** msfs2024aiparser.cpp is part of FF2Play project
**
** This class fills the AI database with the aircraft listed by MSFS 2024
****************************************************************************/

#include "msfs2024aiparser.h"

MSFS2024AIParser::MSFS2024AIParser(QObject *parent) : AIBaseParser(parent)
{
    connect(&m_simManager, &simbasemanager::fireSimObjectList, this, &MSFS2024AIParser::onSimObjectList);
}

MSFS2024AIParser::~MSFS2024AIParser()
{
}

///
/// \brief MSFS2024AIParser::startScanSimObject
/// No thread : the simulator sends the list through SimConnect
///
void MSFS2024AIParser::startScanSimObject()
{
    if ((!m_Initialized) || m_Table.isEmpty()) return;
    if (m_simManager.requestAircraftList())
    {
        m_log.log(tr("Start updating aircraft database..."));
    }
    else
    {
        m_log.log("AIMapping : the simulator does not provide its aircraft list", Qt::darkMagenta,LEVEL_NORMAL);
    }
}

///
/// \brief MSFS2024AIParser::onSimObjectList
/// \param pList (title, livery) of the aircraft the simulator can create
/// \param pComplete false if pages are missing : nothing is removed from the database
///
void MSFS2024AIParser::onSimObjectList(const QList<QPair<QString,QString>>& pList, bool pComplete)
{
    if ((!m_Initialized) || m_Table.isEmpty()) return;
    m_NbAdded=0;
    m_NbUpdated=0;
    m_NbDeleted=0;
    // Records already known, key = title + livery
    QSet<QPair<QString,QString>> Known;
    QSqlQuery query(m_DB);
    query.prepare("SELECT Title, Path FROM Aircrafts WHERE Sim = :Sim");
    query.bindValue(":Sim",m_Table);
    if (!query.exec())
    {
        m_log.log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);
        return;
    }
    while (query.next())
    {
        Known.insert(qMakePair(query.value(0).toString(), query.value(1).toString()));
    }
    m_DB.transaction();
    QSet<QPair<QString,QString>> Listed;
    for (const auto& Aircraft : pList)
    {
        if (Aircraft.first.isEmpty() || Listed.contains(Aircraft)) continue;
        Listed.insert(Aircraft);
        if (Known.contains(Aircraft)) continue;
        const QString Key = QString(QCryptographicHash::hash((Aircraft.first + "|" + Aircraft.second).toUtf8(),
                                                             QCryptographicHash::Md5).toHex());
        // The list gives neither ICAO type, category nor engines
        query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                      "VALUES (:checksum, :Sim, :Title, '', '', '', :Path, '', 0, 4, 0.0, 0.0)");
        query.bindValue(":checksum", Key);
        query.bindValue(":Sim", m_Table);
        query.bindValue(":Title", Aircraft.first);
        query.bindValue(":Path", Aircraft.second);
        if (query.exec()) m_NbAdded++;
    }
    if (pComplete)
    {
        for (const auto& Aircraft : std::as_const(Known))
        {
            if (Listed.contains(Aircraft)) continue;
            query.prepare("DELETE FROM Aircrafts WHERE (Sim = :Sim) and (Title = :Title) and (Path = :Path)");
            query.bindValue(":Sim", m_Table);
            query.bindValue(":Title", Aircraft.first);
            query.bindValue(":Path", Aircraft.second);
            if (query.exec()) m_NbDeleted++;
        }
    }
    else
    {
        m_log.log("AIMapping : incomplete aircraft list, old records kept", Qt::darkMagenta,LEVEL_NORMAL);
    }
    m_DB.commit();
#ifdef FFS_DEBUG
    m_log.log("AIMapping : " + QString::number(Listed.count()) + " aircraft/livery received from the simulator",
              Qt::darkYellow,LEVEL_NORMAL);
#endif
    finishScan();
}
