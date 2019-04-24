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

///
/// \brief AIMapping::AIMapping
/// \param parent
///
AIMapping::AIMapping(QObject *parent) : QObject(parent)
{
    m_log = CLogger::instance();
    m_simManager = simbasemanager::instance();
    connect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReceiveOpen()));
    connect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReceiveClose()));
    m_AIParser = nullptr;
}

///
/// \brief AIMapping::~AIMapping
///
AIMapping::~AIMapping()
{
    disconnect(m_simManager,SIGNAL(opened()),this,SLOT(onSimReceiveOpen()));
    disconnect(m_simManager,SIGNAL(closed()),this,SLOT(onSimReceiveClose()));
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
AIMapping* AIMapping::instance()
{
    return Singleton<AIMapping>::instance(AIMapping::createInstance);
}

///
/// \brief AIMapping::onSimReceiveOpen
///
void AIMapping::onSimReceiveOpen()
{
    if (m_simManager==nullptr) return;
    m_Version = m_simManager->getVersion();
    switch (m_Version)
    {
#ifdef Q_OS_WIN
        case SimConnector::SIM_VERSION::FSX:
        case SimConnector::SIM_VERSION::FSX_STEAM:
        case SimConnector::SIM_VERSION::P3D_V2:
        case SimConnector::SIM_VERSION::P3D_V3:
        case SimConnector::SIM_VERSION::P3D_V4:
        m_AIParser = new SCAIParser(this);
        break;
#endif
        case SimConnector::SIM_VERSION::XPLANE:
        {
            m_AIParser = new XPAIParser(this);
            break;
        }
        case SimConnector::SIM_VERSION::XPLANE_CSL:
        {
            m_AIParser = new XPCSLParser(this);
            break;
        }
        /*case SimConnector::SIM_VERSION::FLIGHTGEAR:
        m_AIParser = new FGAIParser(this);
        break;*/
        default:
        {
            return;
        }
    }
    m_AIParser->startScanSimObject();
    m_simManager->setPathList(getPathList());
}

///
/// \brief AIMapping::onSimReceiveClose
///
void AIMapping::onSimReceiveClose()
{
    if (isInit()) delete m_AIParser;
    m_AIParser = nullptr;
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
/// \brief AIMapping::getAITitreDispo
/// \param pCategory
/// \param pEditor
/// \param pType
/// \param pModel
/// \return
///
QStringList AIMapping::getAITitreDispo(QString pCategory, QString pEditor, QString pType, QString pModel)
{
    QStringList List;
    if (isInit())
    {
        List = m_AIParser->getAITitreDispo(pCategory,pEditor,pType,pModel);
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

    }
    return List;
}

///
/// \brief AIMapping::getEditeurList
/// \return
///
QStringList AIMapping::getEditeurList()
{
    QStringList List;
    if (isInit())
    {

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

    }
    return List;
}

///
/// \brief AIMapping::getRule
/// \param pTitre
/// \return
///
QString AIMapping::getRule(const QString &pTitre)
{
    QString Response="";
    if (isInit())
        Response = m_AIParser->getRule(pTitre);
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
QMap<QString,QString> AIMapping::getList()
{
    QMap<QString,QString> md5mlist;
    if (isInit())
    {

    }
    return md5mlist;
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
/// \brief AIMapping::addPath
/// \param pPath
/// \return
///
bool AIMapping::addPath(const QString& pPath,const QString& pSim)
{
    if (isInit())
        return m_AIParser->addPath(pPath,pSim);
    else return false;
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
    else return false;
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
