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
                QString Editor,Atc_model, Atc_type, Airline, Category, Title;
                int Engine_Type = 0,nb_moteur = 1;
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
                Airline = s.value("properties","acf/_callsign").toString();
                if (s.value("properties","acf/_is_airliner").toBool())
                    Category = "airliner";
                else if (s.value("properties","acf/_is_helicopter").toBool())
                    Category = "helicopter";
                else if (s.value("properties","acf/_is_general_aviation").toBool())
                    Category = "airplane";
                else if (s.value("properties","acf/_is_seaplane").toBool())
                    Category = "seaplane";
                else if (s.value("properties","acf/_is_vtol").toBool())
                    Category = "vtol";
                else if (s.value("properties","acf/_is_glider").toBool())
                    Category = "glider";
                else
                    Category = "undefined";

                // Find the engine Type
                // For XPlane AI, we find the fan rpm for N1 100% . if value different to 0, this mean
                // we have a turbine
                QString EngType = s.value("properties","_engn/0/_type").toString();
                if (Category != "glider")
                {
                    if (EngType == "TRB_FRE")
                    {
                        if (Category == "helicopter")
                            Engine_Type = 3;//We have a rotor turbine
                        else
                            Engine_Type = 5;// We have a turbo propeller
                    }
                    else if (EngType.startsWith("JET"))
                    {
                        Engine_Type = 1; // We have a piston engine
                    }
                    else if (EngType == "ELE")
                    {
                        Engine_Type = 6; // Electrical engine
                    }
                    else if (EngType =="RCP_INJ")
                    {
                        Engine_Type = 0; // We have a piston engine
                    }
                    else
                    {
                        Engine_Type = 4; // Undefined
                    }
                }
                else
                {
                    Engine_Type =2;
                }
                Static_CG_Height =  s.value("properties","acf/_h_eqlbm").toDouble();
                Static_Pitch =  s.value("properties","acf/_the_eqlbm").toDouble();
                nb_moteur = s.value("properties","acf/_num_engn").toInt();

#if FFS_DEBUG
                m_log->log("AIMapping : Path = " + Path,Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Editor = " + Editor,Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : ATCModel = " + Atc_model, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : ATCType = " + Atc_type, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Airline = " + Airline, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Category = " + Category, Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : EngineType = " + QString::number(Engine_Type), Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : CG Height = " +  QString::number(Static_CG_Height), Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : Pitch = " +  QString::number(Static_Pitch), Qt::darkYellow,LEVEL_VERBOSE);
                m_log->log("AIMapping : NBEngine = " + QString::number(nb_moteur), Qt::darkYellow,LEVEL_VERBOSE);
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
                                query.prepare("INSERT INTO Aircrafts (checksum, Sim, Title, Editor, Type, Model, AirLine, Path, Category, NbEngine, EngineType, CGHeight, Pitch)"
                                              "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :AirLine, :Path, :Category, :NbEngine, :EngineType, :CGHeight, :Pitch)");
                                query.bindValue(":checksum", pKey);
                                query.bindValue(":Sim",m_Table);
                                query.bindValue(":Title", Title);
                                query.bindValue(":Editor", Editor);
                                query.bindValue(":Type", Atc_type);
                                query.bindValue(":Model", Atc_model);
                                query.bindValue(":AirLine", Airline);
                                query.bindValue(":Path", Path);
                                query.bindValue(":Category", Category);
                                query.bindValue(":NbEngine", nb_moteur);
                                query.bindValue(":EngineType", Engine_Type);
                                query.bindValue(":CGHeight", Static_CG_Height);
                                query.bindValue(":Pitch", Static_Pitch);
                                query.exec();
                                m_NbAdded++;
                            }
                            else //Update Record
                            {
                                query.prepare("UPDATE Aircrafts SET checksum = :checksum, Editor = :Editor, Type = :Type, Model = :Model, AirLine = :AirLine, Path = :Path, Category = :Category, NbEngine = :NbEngine, EngineType = :EngineType, CGHeight = :CHeight, Pitch = :Pitch WHERE (Title = :Title) and (Sim = :Sim)");
                                query.bindValue(":checksum", pKey);
                                query.bindValue(":Sim",m_Table);
                                query.bindValue(":Title", Title);
                                query.bindValue(":Editor", Editor);
                                query.bindValue(":Type", Atc_type);
                                query.bindValue(":Model", Atc_model);
                                query.bindValue(":AirLine", Airline);
                                query.bindValue(":Path", Path);
                                query.bindValue(":Category", Category);
                                query.bindValue(":NbEngine", nb_moteur);
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
