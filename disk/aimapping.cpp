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
** aimapping.cpp is part of FF2Play project
**
** This singleton provide AI database manager and tools for aircraft
** translation
****************************************************************************/

#include "aimapping.h"
#include "tools/singleton.h"
#include "tools/iniparser.h"
#include <utility>

///
/// \brief AIMapping::AIMapping
/// \param parent
///
AIMapping::AIMapping(QObject *parent) : QObject(parent)
{
    connect(&m_simManager, &simbasemanager::opened, this, &AIMapping::onSimReceiveOpen);
    connect(&m_simManager, &simbasemanager::closed, this, &AIMapping::onSimReceiveClose);
    connect(&m_simManager, &simbasemanager::fireSimEvent, this, &AIMapping::onSimEvent);
    m_Version = SimConnector::SIM_VERSION::UNKNOWN;
}

///
/// \brief AIMapping::~AIMapping
///
AIMapping::~AIMapping()
{
    disconnect(&m_simManager, &simbasemanager::opened, this, &AIMapping::onSimReceiveOpen);
    disconnect(&m_simManager, &simbasemanager::closed, this, &AIMapping::onSimReceiveClose);
    disconnect(&m_simManager, &simbasemanager::fireSimEvent, this, &AIMapping::onSimEvent);
}

///
/// \brief AIMapping::createInstance
/// \return
///
AIMapping* AIMapping::createInstance()
{
    return new AIMapping();
}

///
/// \brief AIMapping::instance
/// \return
///
AIMapping& AIMapping::instance()
{
    return Singleton<AIMapping>::instance(AIMapping::createInstance);
}

///
/// \brief AIMapping::onSimReceiveOpen
///
void AIMapping::onSimReceiveOpen()
{
    m_Version = m_simManager.getVersion();
    switch (m_Version)
    {
#ifdef SIMCONNECT_BUILD
        case SimConnector::SIM_VERSION::FSX:
        case SimConnector::SIM_VERSION::FSX_STEAM:
        case SimConnector::SIM_VERSION::P3D_V2:
        case SimConnector::SIM_VERSION::P3D_V3:
        case SimConnector::SIM_VERSION::P3D_V4:
        case SimConnector::SIM_VERSION::P3D_V5:
        case SimConnector::SIM_VERSION::FS2020:
        m_AIParser = std::make_unique<SCAIParser>();
        break;
        case SimConnector::SIM_VERSION::FS2024:
        m_AIParser = std::make_unique<MSFS2024AIParser>();
        break;
#endif
        case SimConnector::SIM_VERSION::XPLANE:
        {
            m_AIParser = std::make_unique<XPAIParser>();
            break;
        }
        case SimConnector::SIM_VERSION::XPLANE_CSL:
        {
            m_AIParser = std::make_unique<XPCSLParser>();
            break;
        }
        case SimConnector::SIM_VERSION::FLIGHTGEAR:
        {
            m_AIParser = std::make_unique<FGAIParser>();
            break;
        }
        default:
        {
            return;
        }
    }
    // Forwarded to the main window (connection lost in 2019, see docs/audit-2026-10.md F1)
    connect(m_AIParser.get(), &AIBaseParser::fireScanFinished, this, &AIMapping::fireScanFinished);
    m_AIParser->startScanSimObject();
    m_simManager.setPathList(getPathList());
}

///
/// \brief AIMapping::onSimEvent
/// MSFS 2024 can install aircraft while running : its list is read again at each flight
/// \param pEvent
///
void AIMapping::onSimEvent(SimEvent pEvent)
{
    if ((pEvent.Evt_Id == SIM_EVENT_ID::SIMSTART) && (m_Version == SimConnector::SIM_VERSION::FS2024) && isInit())
        m_AIParser->startScanSimObject();
}

///
/// \brief AIMapping::onSimReceiveClose
///
void AIMapping::onSimReceiveClose()
{
    // Waits for the end of a running scan (AIBaseParser destructor)
    m_AIParser.reset();
}

///
/// \brief AIMapping::rebuild
/// After cleanAIDB : creates the parser again, it finds the simulator folders and scans them
///
void AIMapping::rebuild()
{
    if (!m_AIParser) return;
    // The old parser and its SQLite connection are gone before the new one opens its own
    m_AIParser.reset();
    onSimReceiveOpen();
}

///
/// \brief AIMapping::startScanSimObject
///
void AIMapping::startScanSimObject()
{
    if (isInit()) m_AIParser->startScanSimObject();
}

///
/// \brief AIMapping::SolveTitle
/// \return
///
AIResol AIMapping::SolveTitle(const QString& pTitle, const QString& pModel, const QString& pType)
{
    //Default substitute airtcraft
    AIResol Resol;
    if (isInit())
    {
        Resol = m_AIParser->SolveTitle(pTitle,pModel,pType);
    }
    return Resol;
}

///
/// \brief AIMapping::getAITitleDispo
/// \param pCategory
/// \param pEditor
/// \param pType
/// \param pModel
/// \return
///
QStringList AIMapping::getAITitleDispo(
        QString pCategory,
        QString pEditor,
        QString pType,
        QString pModel,
        QString pEngineType,
        QString pNbEngine)
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getAITitleDispo(
                    std::move(pCategory),
                    std::move(pEditor),
                    std::move(pType),
                    std::move(pModel),
                    std::move(pEngineType),
                    std::move(pNbEngine));
    }
    return List;
}

///
/// \brief AIMapping::getCategoryList
/// \return
///
QStringList AIMapping::getCategoryList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getCategoryList();
    }
    return List;
}

///
/// \brief AIMapping::getEditorList
/// \return
///
QStringList AIMapping::getEditorList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getEditorList();
    }
    return List;
}

///
/// \brief AIMapping::getATCTypeList
/// \return
///
QStringList AIMapping::getATCTypeList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getATCTypeList();
    }
    return List;
}

///
/// \brief AIMapping::getATCModelList
/// \return
///
QStringList AIMapping::getATCModelList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getATCModelList();
    }
    return List;
}

///
/// \brief AIMapping::getEngineTypeList
/// \return
///
QStringList AIMapping::getEngineTypeList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getEngineTypeList();
    }
    return List;
}

///
/// \brief AIMapping::getEngineNumberList
/// \return
///
QStringList AIMapping::getNbEngineList()
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getNbEngineList();
    }
    return List;
}

///
/// \brief AIMapping::getRule
/// \param pTitre
/// \return
///
QString AIMapping::getRule(const QString &pTitle)
{
    QString Response="";
    if (isInit())
        Response = m_AIParser->getRule(pTitle);
    return Response;
}

///
/// \brief AIMapping::addRule
/// \param pRemoteTitle
/// \param pLocalTitle
///
void AIMapping::addRule(const QString &pRemoteTitle, const QString &pLocalTitle)
{
    if (isInit()) m_AIParser->addRule(pRemoteTitle,pLocalTitle);
}

///
/// \brief AIMapping::delRule
/// \param pRemoteTitle
///
void AIMapping::delRule(const QString &pRemoteTitle)
{
    if (isInit()) m_AIParser->delRule(pRemoteTitle);
}

///
/// \brief AIMapping::cleanAIDB
///
void AIMapping::cleanAIDB()
{
    if (isInit()) m_AIParser->cleanAIDB();
}

///
/// \brief AIMapping::getList
/// \return
///
QMap<QString,QString> AIMapping::getRulesList()
{
    QMap<QString,QString> RulesList;
    if (isInit())
        return m_AIParser->getRulesList();
    return RulesList;
}

///
/// \brief AIMapping::getAircraftPath
/// \param pTitle
/// \return
///
QString AIMapping::getAircraftPath(const QString &pTitle)
{
    QString Response;
    if (isInit())
        Response  = m_AIParser->getAircraftPath(pTitle);
    return Response;
}

///
/// \brief AIMapping::getAircraftInfo
/// \param pTitle
/// \param pModel ICAO type of the aircraft
/// \param pType
/// \param pCategory
/// \param pPath if not null, file of the aircraft
/// \param pNbEngine if not null, number of engines (0 unknown)
/// \param pEngineType if not null, engine type (FGAIParser codes, 4 undefined)
/// \return false if the aircraft is not in the database
///
bool AIMapping::getAircraftInfo(const QString& pTitle, QString& pModel, QString& pType, QString& pCategory,
                                QString* pPath, int* pNbEngine, int* pEngineType)
{
    if (!isInit()) return false;
    return m_AIParser->getAircraftInfo(pTitle, pModel, pType, pCategory, pPath, pNbEngine, pEngineType);
}

///
/// \brief AIMapping::getFileData
/// \param pKey
/// \return
///
QString AIMapping::getFileData(const QString& /*&pKey*/)
{
    QString Response;
    if (isInit())
    {

    }
    return Response;
}

///
/// \brief AIMapping::getPathList
/// \return
///
QStringList AIMapping::getPathList()
{
    QStringList List;
    if (isInit())
        List = m_AIParser->getPathList();
    return List;
}

///
/// \brief AIMapping::needPaths
/// \return false if the simulator gives its aircraft list (MSFS 2024) : no folder to configure
///
bool AIMapping::needPaths()
{
    return isInit() && m_AIParser->needPaths();
}

///
/// \brief AIMapping::addPath
/// \param pPath
/// \return
///
bool AIMapping::addPath(const QString& pPath,const QString& pSim)
{
    if (isInit())
        return m_AIParser->addPath(pPath,pSim);
    return false;
}

///
/// \brief AIMapping::delPath
/// \param pPath
/// \return
///
bool AIMapping::delPath(const QString& pPath)
{
    if (isInit())
        return m_AIParser->delPath(pPath);
    return false;
}

///
/// \brief AIMapping::isInit
/// \return
///
bool  AIMapping::isInit()
{
    bool Result=false;
    if (m_AIParser!=nullptr)
    {
        if ( m_AIParser->m_Initialized) Result=true;
    }
    return Result;
}
