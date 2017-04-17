/****************************************************************************
**
** Copyright (C) 2017 Gianni Peschiutta.
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

/*! \class CWebManager
*/

#include "singleton/webmanager.h"
#include "singleton/singleton.h"

#include <QCryptographicHash>
#include <QByteArray>
#include <QXmlStreamWriter>

#include "tools/getappversion.h"

QDataStream& operator<<(QDataStream& out, const QList<SUser>& v)
{
    out << v.count();
    for (QList<SUser>::const_iterator it = v.constBegin(); it != v.constEnd(); ++it)
    {
        out << it->Profil << it->Login << it->Password << it->URL;
    }
    return out;
}

QDataStream& operator>>(QDataStream& in, QList<SUser>& v)
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

/*!
    Creates a CWebManager singleton object.

    \a parent is passed to the QObject constructor.
*/
CWebManager::CWebManager(QObject *parent) : QObject(parent)
{
    metaXmlCode = QMetaEnum::fromType<XmlCode>();
    qRegisterMetaTypeStreamOperators<QList<SUser> >("QList<SUser>");
    Log = CLogger::instance();
    s.beginGroup("Web");
    QByteArray Profils = QByteArray::fromBase64(s.value("Profils","").toByteArray());
    QByteArray Check = QByteArray::fromBase64(s.value("Hash","").toByteArray());
    QCryptographicHash Hash(QCryptographicHash::Md5);
    Hash.addData(Profils);
    QByteArray result = Hash.result();
    int num = s.value("Default",0).toInt();
    if(Profils.isEmpty() && (num>0))
    {
        Log->log(tr("WebManager : Users Profils Settings is empty but default user is set. Reset of default user"),Qt::darkMagenta);
        s.setValue("Default",0);
    }
    else if (result == Check)
    {
       QDataStream stream(Profils);
       stream.setVersion(QDataStream::Qt_5_8);
       Users.clear();
       stream >> Users;
        if (num >= Users.count())
        {
                num = 0;
                s.setValue("Default",0);
        }
    }
    else if (!Check.isEmpty())
    {
        Log->log(tr("WebManager : Users Profils Settings checkum error, repair setting data"),Qt::darkMagenta);
        s.setValue("Default",0);
        s.setValue("Profils","");
        s.setValue("Hash","");
    }
    s.endGroup();
    if (num < Users.count()) CurrentUser = Users.at(num);
    QDomDocument SendHello;
    XmlBuild(SendHello, hello);
}

/*!
    Destroys the signleton instance.
*/
CWebManager::~CWebManager()
{

}

CWebManager* CWebManager::createInstance()
{
    return new CWebManager();
}

CWebManager* CWebManager::instance()
{
    return Singleton<CWebManager>::instance(CWebManager::createInstance);
}

void CWebManager::XmlBuild (QDomDocument& Doc, XmlCode Code)
{
    QDomProcessingInstruction Header = Doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"utf-8\"");
    Doc.appendChild(Header);

    //Root
    QDomElement xmlRoot = Doc.createElement("FFS2Play");
    Doc.appendChild(xmlRoot);

    //Version
    QDomElement xmlVersion = Doc.createElement("version");
    xmlVersion.appendChild(Doc.createTextNode(GetAppVersion().FullVersion));
    xmlRoot.appendChild(xmlVersion);

    //Création du switch
    QDomElement xmlSwitch = Doc.createElement("switch");
    xmlRoot.appendChild(xmlSwitch);

    //Création du data
    QDomElement xmlData = Doc.createElement("data");
    xmlData.appendChild(Doc.createTextNode(metaXmlCode.valueToKey(Code)));
    xmlSwitch.appendChild(xmlData);

    //Création du key
    QDomElement xmlKey = Doc.createElement("key");
    xmlKey.appendChild(Doc.createTextNode(Key));
    xmlSwitch.appendChild(xmlKey);

    switch (Code)
    {
        case hello:
        {
            QDomElement xmlHello = Doc.createElement("hello");
            xmlRoot.appendChild(xmlHello);
            //Création de la balise pilotID
            QDomElement xmlLogin = Doc.createElement("pilotID");
            xmlLogin.appendChild(Doc.createTextNode(CurrentUser.Login));
            xmlHello.appendChild(xmlLogin);
            break;
        }
    }

    QString out;
    QTextStream* stream = new QTextStream (&out);
    stream->setCodec("UTF-8");
    Doc.save(*stream,4);
    Log->log("XMl = " +out);
}

void CWebManager::SaveConfig()
{
    QByteArray profils;
    QDataStream stream(&profils,QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_8);
    stream << Users;
    QCryptographicHash Hash(QCryptographicHash::Md5);
    Hash.addData(profils);
    QByteArray checksum = Hash.result();
    s.beginGroup("Web");
    int num = s.value("Default",0).toInt();
    if ((num==0) && (Users.count()>0)) num = 1;
    s.setValue("Default",num);
    s.setValue("Hash", QString(checksum.toBase64()));
    s.setValue("Profils",QString(profils.toBase64()));
    s.endGroup();
}
