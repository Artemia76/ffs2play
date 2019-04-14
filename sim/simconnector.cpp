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
** simconnector.cpp is part of FF2Play project
**
** This is the simulator abstractive class use to define a standard base for
** all simulators
****************************************************************************/

#include "simconnector.h"

///
/// \brief SimConnector::SimConnector
/// \param parent
///
SimConnector::SimConnector(QObject *parent) : QObject(parent)
{
    m_log=CLogger::instance();
    m_connected = false;
    m_settings.beginGroup("Sim");
    m_options = 0;
    m_options =m_settings.value("Options", 0x00000001).toUInt();
#ifdef FFS_DEBUG
    m_log->log("SimConnector : " + tr("Option value = ") + QString::number(m_options),Qt::darkBlue,LEVEL_NORMAL);
#endif
    m_CallSignColor = m_settings.value("CallSignColor", "#FF0000").toString();
    m_settings.endGroup();
    m_version.Build=0;
    m_version.FullVersion="none";
    m_version.Major=0;
    m_version.Minor=0;
    m_version.Name="none";
}

///
/// \brief SimConnector::~SimConnector
///
SimConnector::~SimConnector()
{

}

///
/// \brief SimConnector::isConnected
/// \return
///
bool SimConnector::isConnected()
{
    return m_connected;
}

///
/// \brief SimConnector::setOptionChange
/// \param pOption
/// \param pFlag
///
void SimConnector::setOptionChange(SimOption pOption,bool pFlag)
{
    quint32 mask = static_cast<quint32>(pOption);
    if (!pFlag)
    {
        m_options = m_options & (~mask);
    }
    else
    {
        m_options = m_options | mask;
    }
    m_settings.beginGroup("Sim");
    m_settings.setValue("Options",m_options);
    m_settings.endGroup();
    sendOptions();
}

///
/// \brief SimConnector::getOption
/// \param pOption
/// \return
///
bool SimConnector::getOption(SimOption pOption)
{
    return (m_options & static_cast<quint32>(pOption)) > 0;
}

///
/// \brief SimConnector::getState
/// \return
///
CAircraftState SimConnector::getState()
{
    CAircraftState state;
    m_Mutex.lock();
    state=m_state;
    m_Mutex.unlock();
    return state;
}

void SimConnector::setCallSignColor(const QColor& pColor)
{
    m_settings.beginGroup("Sim");
    m_settings.setValue("CallSignColor",pColor.name());
    m_settings.endGroup();
    m_CallSignColor = pColor;
    sendCallSignColor();
}

QColor SimConnector::getCallSignColor()
{
    return m_CallSignColor;
}
