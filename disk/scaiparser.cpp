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
** scaiparser.cpp is part of FF2Play project
**
** This class provide method to parse FSX like AI files
****************************************************************************/

#include "scaiparser.h"
#include "tools/iniparser.h"

SCAIParser::SCAIParser(QObject *parent) : AIBaseParser(parent)
{
    m_AIFileFilter << "aircraft.cfg" << "sim.cfg";
    // FSX,FSX Steam Edition, P3D v2, P3D v3, P3D v4
    searchSimPath();
}

SCAIParser::~SCAIParser() = default;

///
/// \brief AIMapping::DBFileUpdate
/// \param pKey
/// \param pPath
///
void SCAIParser::DBFileUpdate(const QString &pKey, const QString &pPath)
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
                Path = Path.replace("/","\\");
                QString Editor;
                QString Atc_model;
                QString Atc_type;
                QString Category;
                QString Title;
                int Engine_Type = 0;
                double Static_CG_Height=0.0;
                double Static_Pitch = 0.0;
                Editor = s.value("fltsim.0", "ui_createdby").toString();
                Atc_model = s.value("General","atc_model").toString();
                Atc_type =  s.value("General","atc_type").toString();
                Category =  s.value("General","Category").toString().toLower();
                Category = Category.remove(' ');
                Engine_Type =  s.value("GeneralEngineData","engine_type").toInt();
                Static_CG_Height =  s.value("contact_points","static_cg_height").toDouble();
                Static_Pitch =  s.value("contact_points","static_pitch").toDouble();
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
                    Title = s.value(QString("fltsim.") + QString::number(nb_title), "title").toString();

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
                                              "VALUES (:checksum, :Sim, :Title, :Editor, :Type, :Model, :Path, :Category, :NbEngine, :EngineType, :CGHeight, :Pitch)");
                                query.bindValue(":checksum", pKey);
                                query.bindValue(":Sim",m_Table);
                                query.bindValue(":Title", Title);
                                query.bindValue(":Editor", Editor);
                                query.bindValue(":Type", Atc_type);
                                query.bindValue(":Model", Atc_model);
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
                    else break;
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

///
/// \brief SCAIParser::searchSimPath
///
void SCAIParser::searchSimPath()
{
    if (!m_Initialized) return;
    QString Path;
    QSqlQuery query(m_DB);
    QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
    QHash<QString, QString> hash;
    hash[version.valueToKey(SimConnector::SIM_VERSION::FSX)] = R"(Microsoft\Microsoft Games\Flight Simulator\10.0)";
    hash[version.valueToKey(SimConnector::SIM_VERSION::FSX_STEAM)] = R"(DovetailGames\FSX)";
    hash[version.valueToKey(SimConnector::SIM_VERSION::P3D_V2)] = R"(Lockheed Martin\Prepar3D v2)";
    hash[version.valueToKey(SimConnector::SIM_VERSION::P3D_V3)] = R"(Lockheed Martin\Prepar3D v3)";
    hash[version.valueToKey(SimConnector::SIM_VERSION::P3D_V4)] = R"(Lockheed Martin\Prepar3D v4)";

    // Lets search path installation for each supported windows simulators
    QHashIterator<QString, QString> it(hash);
    while (it.hasNext())
    {
        it.next();
        query.prepare("SELECT Path FROM SimPaths WHERE Sim = :Sim;");
        query.bindValue(":Sim",it.key());
        if (query.exec())
        {
            if (!query.next()) // If path installation is empty
            {
#ifdef FFS_DEBUG
                m_log->log("AIMapping : " + it.key() + " Path list is empty, let try to find path installation...", Qt::darkYellow,LEVEL_NORMAL);
#endif
                Path = findInstallationPath(it.value());
                //If Path is empty let try to search in 64Bit registry
                if (Path.isEmpty())  Path = findInstallationPath(it.value(),QSettings::Registry64Format);
                if (!Path.isEmpty())
                {
#ifdef FFS_DEBUG
                    m_log->log("AIMapping : Path found for " + it.key() + " = " + Path, Qt::darkYellow,LEVEL_NORMAL);
#endif
                    addPath(QDir::fromNativeSeparators(Path + "SimObjects\\Airplanes"), it.key());
                    addPath(QDir::fromNativeSeparators(Path + "SimObjects\\Rotorcraft"), it.key());
                }
                else
                {
#ifdef FFS_DEBUG
                    m_log->log("AIMapping : No path found for " + it.key(), Qt::darkYellow,LEVEL_NORMAL);
#endif
                }
            }
            else
            {
#ifdef FFS_DEBUG
                m_log->log("AIMapping : " + it.key() + " Path list already contains some path", Qt::darkYellow,LEVEL_NORMAL);
#endif
            }
        }
        else
        {
#ifdef FFS_DEBUG
            m_log->log("AIMapping : Fail to query path list for simulator " + m_Table, Qt::darkYellow,LEVEL_NORMAL);
#endif
        }
    }
}

///
/// \brief SCAIParser::findInstallationPath
/// \param pKey
/// \return
///
QString SCAIParser::findInstallationPath(const QString &pKey,QSettings::Format pFormat)
{
    QString Response = "";
    QSettings localKey32("HKEY_LOCAL_MACHINE\\Software\\" + pKey ,pFormat);
    QSettings localKey64("HKEY_LOCAL_MACHINE\\WOW6432Node\\" + pKey ,pFormat);
    QString registryKey;
    Response = localKey32.value("Install_Path","").toString();
    if (Response.isEmpty())
    {
        Response = localKey32.value("SetupPath","").toString();
    }
    if (Response.isEmpty())
    {
        Response = localKey64.value("Install_Path","").toString();
    }
    if (Response.isEmpty())
    {
        Response = localKey64.value("SetupPath","").toString();
    }
    return Response;
}
