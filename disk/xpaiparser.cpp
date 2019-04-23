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
** xpaiparser.cpp is part of FF2Play project
**
** This class provide method to parse X-Plane AI files
****************************************************************************/

#include "xpaiparser.h"
#include "tools/iniparser.h"

XPAIParser::XPAIParser(QObject *parent) : AIBaseParser(parent)
{
    m_AIFileFilter << "*.acf";
}

XPAIParser::~XPAIParser()
{

}

///
/// \brief AIMapping::DBFileUpdate
/// \param pKey
/// \param pPath
///
void XPAIParser::DBFileUpdate(const QString &pKey, const QString &pPath)
{
    if ((!m_Initialized) || m_Table.isEmpty()) return;
    if (!m_Thread_DB.isOpen()) return;
    QSqlQuery query(m_Thread_DB);
    QString Path= pPath;
    query.prepare( "SELECT checksum FROM Aircrafts WHERE (checksum = :checksum) and (Sim = :Sim)");
    query.bindValue(":checksum",pKey);
    query.bindValue(":Sim",m_Table);
    if (query.exec())
    {
        if (!query.next())
        {
#ifdef FFS_DEBUG
            m_log->log("AIMapping : No record was found for " + pKey, Qt::darkYellow,LEVEL_VERBOSE);
#endif
            try
            {
                SIM_TYPE sim=m_simManager->getType();
                IniParser s(Path,sim);
#ifdef Q_OS_WIN
                Path = Path.replace("/","\\");
#endif
                QString Editor,Atc_model, Atc_type, Category, Title;
                int Engine_Type = 0;
                double Static_CG_Height=0.0, Static_Pitch = 0.0;
                Title = s.value("properties", "acf/_name").toString();
                if (Title=="")
                {
                    QFileInfo File(Path);
                    Title = File.baseName();
                }
                Editor = s.value("properties", "acf/_studio").toString();
                Atc_model = s.value("properties","acf/_ICAO").toString();
                Atc_type = s.value("properties","acf/_manufacturer").toString();
                //Category =  s.value("General","Category").toString();
                //Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
                Static_CG_Height =  s.value("properties","acf/_h_eqlbm").toDouble();
                Static_Pitch =  s.value("properties","acf/_the_eqlbm").toDouble();
#if FFS_DEBUG
                m_log->log("AIMapping : Path = " + Path,Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Editor = " + Editor,Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : ATCModel = " + Atc_model, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : ATCType = " + Atc_type, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Category = " + Category, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : EngineType = " + QString::number(Engine_Type), Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : CG Height = " +  QString::number(Static_CG_Height), Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Pitch = " +  QString::number(Static_Pitch), Qt::darkYellow,LEVEL_VERBOSE);
#endif
                int nb_moteur = 0;
                while (true)
                {
                    QString Reponse = s.value("GeneralEngineData", QString("Engine.") + QString::number(nb_moteur)).toString();
                    if (Reponse != "")
                    {
                        nb_moteur++;
                    }
                    else break;
                }
#if FFS_DEBUG
                m_log->log("AIMapping : Engine quantity = " + QString::number(nb_moteur),Qt::darkYellow,LEVEL_VERBOSE);
#endif
                int nb_title = 0;
                while (true)
                {
                    if (Title != "")
                    {
#if FFS_DEBUG
                        m_log->log("AIMapping : livery found Title = " + Title,Qt::darkYellow,LEVEL_VERBOSE);
#endif
                        nb_title++;
                        query.prepare("SELECT * FROM Aircrafts WHERE (Title = :Title) AND (checksum = :checksum) AND (Sim = :Sim)" );
                        query.bindValue(":Title",Title);
                        query.bindValue(":checksum",pKey);
                        query.bindValue(":Sim", m_Table);
                        if (query.exec())
                        {
                            // If there is no result, we have a new aircraft to register
                            if (!query.next())
                            {
                                query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                                              "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :Path, :Category, :NBEngine, :EngineType, :CGHeight, :Pitch)");
                                query.bindValue(":checksum", pKey);
                                query.bindValue(":Sim",m_Table);
                                query.bindValue(":Title", Title);
                                query.bindValue(":Editor", Editor);
                                query.bindValue(":Type", Atc_type);
                                query.bindValue(":Model", Atc_model);
                                query.bindValue(":Path", Path);
                                query.bindValue(":Category", Category);
                                query.bindValue(":NBEngine", nb_moteur);
                                query.bindValue(":EngineType", Engine_Type);
                                query.bindValue(":CGHeight", Static_CG_Height);
                                query.bindValue(":Pitch", Static_Pitch);
                                query.exec();
                                m_NbAdded++;
                            }
                            else //Update Record
                            {
                                query.prepare("UPDATE Aircrafts SET checksum = :checksum, Editor = :Editor, Type = :Type, Model = :Model, Path = :Path, Category = :Category, NbEngine = :NbEngine, EngineType = :EngineType, CGHeight = :CHeight, Pitch = :Pitch WHERE (Title = :Title) and (Sim = :Sim)");
                                query.bindValue(":checksum", pKey);
                                query.bindValue(":Sim",m_Table);
                                query.bindValue(":Title", Title);
                                query.bindValue(":Editor", Editor);
                                query.bindValue(":Type", Atc_type);
                                query.bindValue(":Model", Atc_model);
                                query.bindValue(":Path", Path);
                                query.bindValue(":Category", Category);
                                query.bindValue(":NBEngine", nb_moteur);
                                query.bindValue(":EngineType", Engine_Type);
                                query.bindValue(":CGHeight", Static_CG_Height);
                                query.bindValue(":Pitch", Static_Pitch);
                                query.exec();
                                m_NbUpdated++;
                            }
                        }
                    }
                    break;
                }
            }
            catch (IniParserException& ex)
            {
                m_log->log(tr("AIMapping : IniParser error = ") + ex.Message,Qt::magenta,LEVEL_NORMAL);
            }
        }
    }
    else m_log->log(tr("AIMapping : Error = ") + query.lastError().text(), Qt::darkYellow,LEVEL_VERBOSE);

}
