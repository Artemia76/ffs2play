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
** webmanager.Utils.cpp is part of FF2Play project
**
** This singleton provide Web Manager for web server communication
****************************************************************************/

#include "web/webmanager.h"
#include "tools/singleton.h"
#include <QXmlStreamWriter>

/// Build Users Profils to Setting file
/// \brief operator <<
/// \param out
/// \param v
/// \return
///
QDataStream& operator<<(QDataStream& out, const QVector<SUser>& v)
{
    out << v.count();
    for (QVector<SUser>::const_iterator it = v.constBegin(); it != v.constEnd(); ++it)
    {
        out << it->Profil << it->Login << it->Password << it->URL;
    }
    return out;
}

/// Extraction of Users profils data from Setting file
/// \brief operator >>
/// \param in
/// \param v
/// \return
///
QDataStream& operator>>(QDataStream& in, QVector<SUser>& v)
{
    v.clear();
    int num;
    in >> num;
    for (int i=0; i < num; i++)
    {
        SUser User;
        in >> User.Profil;
        in >> User.Login;
        in >> User.Password;
        in >> User.URL;
        v.append(User);
    }
    return in;
}

///
/// \brief CWebManager::CheckError
/// \param Doc
/// \return
///
bool CWebManager::CheckError(QDomDocument& pDoc)
{
    QDomElement Element = pDoc.elementsByTagName("erreur").at(0).toElement();
    if (!Element.isNull())
    {
        int Level = Element.attribute("error").toInt();
        QColor Couleur = Qt::black;
        switch (Level)
        {
            case 0:
                Couleur = Qt::black;
                break;
            case 1:
                Couleur = Qt::darkRed;
                break;
            case 2:
                Couleur = Qt::darkRed;
                //Disconnect();
                break;
        }
        m_log->log(tr("Server : ") + Element.firstChild().nodeValue(),Couleur);
        if (Level >= 2) return true;
    }
    return false;
}

///
/// \brief CWebManager::CheckWhazzup
/// \param Doc
///
void CWebManager::CheckWhazzup(QDomDocument& pDoc)
{
    QDomElement Element = pDoc.elementsByTagName("whazzup").at(0).toElement();
    if (!Element.isNull())
    {
        QVector<PeerData> whazzup;
        for (int i = 0; i< Element.childNodes().count();i++)
        {
            if (Element.childNodes().at(i).isElement())
            {
                QDomElement Player = Element.childNodes().at(i).toElement();
                PeerData item;
                item.CallSign = Player.nodeName();
                item.Port=Player.attribute("port").toUShort();
                item.externalIP=m_crypto.DecryptFromBase64(Player.attribute("ip"));
#ifdef FFS_DEBUG
                m_log->log("WEBManager : CallSign = " + item.CallSign
                            + " IP = " + item.externalIP.toString()
                            + " Port = " + QString::number(item.Port),Qt::darkBlue,LEVEL_VERBOSE);
#endif
                StringArray LocalIP;
                LocalIP.ParseFromString(QByteArray::fromBase64(Player.attribute("local_ip").toLatin1()).toStdString());
                for (int j=0; j<LocalIP.string_size();j++)
                {
                    item.localIPs.append(QHostAddress(QString::fromStdString(LocalIP.string(j))));
#ifdef FFS_DEBUG
                    m_log->log("WEBManager : Local IP = " + item.localIPs.last().toString() ,Qt::darkBlue,LEVEL_VERBOSE);
#endif
                }
                //Check for team membership
                item.Team = Player.attribute("team");
                whazzup.append(item);
            }
        }
        if (whazzup.count()>0)
        {
            m_P2P->whazzup_Update(whazzup);
        }
    }
}
bool CWebManager::isConnected()
{
    return m_bConnected;
}
