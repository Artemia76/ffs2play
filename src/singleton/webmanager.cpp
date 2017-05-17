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

#include "singleton/webmanager.h"
#include "singleton/singleton.h"

#include <QCryptographicHash>
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QApplication>

#include "tools/getappversion.h"

/// Build Users Profils to Setting file
/// \brief operator <<
/// \param out
/// \param v
/// \return
///

QDataStream& operator<<(QDataStream& out, const QList<SUser>& v)
{
    out << v.count();
    for (QList<SUser>::const_iterator it = v.constBegin(); it != v.constEnd(); ++it)
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

///
/// \brief CWebManager::CWebManager
/// \param parent
/// Creates a CWebManager singleton object.


CWebManager::CWebManager(QObject *parent) : QObject(parent)
{
    m_bConnected=false;
    m_AIManagement=false;

    metaXmlCode = QMetaEnum::fromType<XmlCode>();
    qRegisterMetaTypeStreamOperators<QList<SUser> >("QList<SUser>");
    Log = CLogger::instance();
    P2P = CP2PManager::instance();
    s.beginGroup("Web");
    QByteArray Profils = QByteArray::fromBase64(s.value("Profils","").toByteArray());
    QByteArray Check = QByteArray::fromBase64(s.value("Hash","").toByteArray());
    QCryptographicHash Hash(QCryptographicHash::Md5);
    Hash.addData(Profils);
    QByteArray result = Hash.result();
    int num = s.value("Default",0).toInt();
    if(Profils.isEmpty() && (num>0))
    {
        Log->log("WebManager : " + tr("Users Profils Settings is empty but default user is set. Reset of default user"),Qt::darkMagenta);
        s.setValue("Default",0);
    }
    else if (result == Check)
    {
        QDataStream stream(Profils);
        stream.setVersion(QDataStream::Qt_5_8);
        Users.clear();
        stream >> Users;
        if ((num > Users.count()) || (num <1))
        {
                (Users.count()> 0) ? num = 1 : num =0;
                s.setValue("Default",num);
        }
    }
    else if (!Check.isEmpty())
    {
        Log->log("WebManager : " + tr("Users Profils Settings checkum error, repair setting datas"),Qt::darkMagenta);
        s.setValue("Default",0);
        s.setValue("Profils","");
        s.setValue("Hash","");
    }
    s.endGroup();
    CurrentUser = num;
#ifdef QT_DEBUG
    Log->log("WebManager : " + tr("AES Key generated = ") + QString(Crypto.EncryptionKeyString()),Qt::darkBlue,LEVEL_NORMAL);
#endif
    connect(this, SIGNAL(FireHelloCallback()),this, SLOT( SendVerify()));
    connect(this, SIGNAL(FireConnected()),this, SLOT( OnConnected()));
    connect(this, SIGNAL(FireDisconnected()),this, SLOT( OnDisconnected()));
    ATCTimer.setInterval(5000);
    LiveUpdateTimer.setInterval(30000);
    connect(&ATCTimer, SIGNAL(timeout()),this,SLOT(OnATCUpdate()));
    connect(&LiveUpdateTimer, SIGNAL(timeout()),this,SLOT(OnLiveUpdate()));
    LiveUpdateTimer.start();
}

///
/// \brief CWebManager::~CWebManager
///

CWebManager::~CWebManager()
{

}

///
/// \brief CWebManager::createInstance
/// \return
///

CWebManager* CWebManager::createInstance()
{
    return new CWebManager();
}

///
/// \brief CWebManager::instance
/// \return
///

CWebManager* CWebManager::instance()
{
    return Singleton<CWebManager>::instance(CWebManager::createInstance);
}

///
/// \brief CWebManager::XmlBuild
/// \param Doc
/// \param Code
///

void CWebManager::XmlBuild (QDomDocument& Doc, XmlCode Code)
{
    QDomProcessingInstruction Header = Doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
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
            QDomElement xmlLogin = Doc.createElement("pilotID");
            xmlLogin.appendChild(Doc.createTextNode(Users[CurrentUser-1].Login));
            xmlHello.appendChild(xmlLogin);
            break;
        }
        case verify:
        {
            QDomElement xmlVerify = Doc.createElement("verify");
            xmlRoot.appendChild(xmlVerify);
            QDomElement xmlAES = Doc.createElement("AES");
            xmlAES.appendChild(Doc.createTextNode(m_sCrypted_AESKey));
            xmlVerify.appendChild(xmlAES);
            QDomElement xmlLogin = Doc.createElement("pilotID");
            xmlLogin.appendChild(Doc.createTextNode(Users[CurrentUser-1].Login));
            xmlVerify.appendChild(xmlLogin);
            QDomElement xmlPassword = Doc.createElement("password");
            xmlPassword.appendChild(Doc.createTextNode(Crypto.EncryptToBase64(Users[CurrentUser-1].Password)));
            xmlVerify.appendChild(xmlPassword);
            QDomElement xmlPort = Doc.createElement("port");
            xmlPort.appendChild(Doc.createTextNode(QString::number(P2P->getPort())));
            xmlVerify.appendChild(xmlPort);
            QDomElement xmlLocalIP = Doc.createElement("local_ip");
            xmlLocalIP.appendChild(Doc.createTextNode(P2P->getLocalIPs()));
            xmlVerify.appendChild(xmlLocalIP);
            break;
        }
        case liveupdate:
        {
            break;
        }
        case atc:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

///
/// \brief CWebManager::SaveConfig
///

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
    int num=CurrentUser;
    if (Users.count()==0) num = 0;
    s.setValue("Default",num);
    s.setValue("Hash", QString(checksum.toBase64()));
    s.setValue("Profils",QString(profils.toBase64()));
    s.endGroup();
}

///
/// \brief CWebManager::SetCurrentUser
/// \param num
///

void CWebManager::SetCurrentUser(int num)
{
    if ((num<1) || (num>Users.count())) return;
    CurrentUser = num;
}

void CWebManager::connect_disconnect()
{
    if (m_bConnected) Disconnect();
    else Connect();
}

///
/// \brief CWebManager::Connect
///

void CWebManager::Connect()
{
    if (m_bConnected) return;
    if (CurrentUser == 0) return;
    QDomDocument SendHello;
    XmlBuild(SendHello, hello);
    CHttpRequest* Request = new CHttpRequest(this);
    connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( HelloCallback (CHttpRequest*)));
    Request->execute(Users[CurrentUser-1].URL+"/action.php",SendHello);
}

///
/// \brief CWebManager::Disconnect
///

void CWebManager::Disconnect()
{
    if (!m_bConnected) return;
    ATCTimer.stop();
    OnLiveUpdate();
    QDomDocument SendLogout;
    XmlBuild(SendLogout, logout);
    CHttpRequest* Request = new CHttpRequest(this);
    connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( LogoutCallback(CHttpRequest*)));
    Request->execute(Users[CurrentUser-1].URL+"/action.php",SendLogout);
}

///
/// \brief CWebManager::HelloCallback
/// \param Request
///

void CWebManager::HelloCallback (CHttpRequest* Request)
{
#ifdef QT_DEBUG
    Log->log("WebManager : " + tr("Get HelloCallback"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
    QDomDocument Doc;
    if (Request->getAsXml(Doc))
    {
        QDomNode NodeCert = Doc.elementsByTagName("certificat").at(0);
        QString SCert = NodeCert.firstChild().nodeValue();
        m_sCrypted_AESKey = Crypto.RSA_EncryptToBase64(SCert, Crypto.EncryptionKey());
#ifdef QT_DEBUG
        Log->log("WebManager: AES Key encrypted with RSA = " + m_sCrypted_AESKey,Qt::darkBlue,LEVEL_NORMAL);
#endif
        emit (FireHelloCallback());
    }
    Request->deleteLater();
}


///
/// \brief CWebManager::SendVerify
///

void CWebManager::SendVerify()
{
    QDomDocument SendVerify;
    XmlBuild(SendVerify, verify);
    CHttpRequest* Request = new CHttpRequest(this);
    connect(Request, SIGNAL(finished(CHttpRequest*)),this, SLOT( ConnectCallback (CHttpRequest*)));
    Request->execute(Users[CurrentUser-1].URL+"/action.php",SendVerify);
}

///
/// \brief CWebManager::ConnectCallback
/// \param Request
///

void CWebManager::ConnectCallback(CHttpRequest *Request)
{
#ifdef QT_DEBUG
    Log->log("WebManager : " + tr("Get ConnectCallback"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
    QDomDocument Doc;
    if (Request->getAsXml(Doc))
    {
        bool bError = CheckError(Doc);
        QDomElement Element = Doc.elementsByTagName("loginStatus").at(0).toElement();
        if (!Element.isNull())
        {
            m_bConnected = Element.firstChild().nodeValue().toInt()==1;
            if (m_bConnected && (!bError))
            {
                Log->log(tr("FFS2Play is connected on server"));
                QDomElement xmlExternalIP = Doc.elementsByTagName("your_ip").at(0).toElement();
                if (!xmlExternalIP.isNull())
                {
                    QHostAddress ExternalIP (xmlExternalIP.firstChild().nodeValue());
#ifdef QT_DEBUG
                    if (ExternalIP.isNull())
                    {
                        Log->log("WebManager : " + tr("Get invalid external IP"),Qt::darkMagenta,LEVEL_VERBOSE);
                    }
                    else
                    {
                        Log->log("WebManager : " + tr("Get external IP = ") + ExternalIP.toString(),Qt::darkMagenta,LEVEL_VERBOSE);
                    }
#endif
                }
                QDomElement xmlKey = Doc.elementsByTagName("key").at(0).toElement();
                if (!xmlKey.isNull())
                {
                    Key = xmlKey.firstChild().nodeValue();
                    OnLiveUpdate();
                }
                QDomElement xmlAtc = Doc.elementsByTagName("atc").at(0).toElement();
                if (!xmlAtc.isNull())
                {
                    if (xmlAtc.firstChild().nodeValue().toInt()==1)
                    {
                        ATCTimer.start();
                    }
                }
                QDomElement AIManagement = Doc.elementsByTagName("AI_Management").at(0).toElement();
                if (!AIManagement.isNull())
                {
                    m_AIManagement = (AIManagement.firstChild().nodeValue().toInt()==1);
                }
                emit (FireConnected());
            }
        }

    }
    Request->deleteLater();
}

bool CWebManager::CheckError(QDomDocument& Doc)
{
    QDomElement Element = Doc.elementsByTagName("erreur").at(0).toElement();
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
                QApplication::beep();
                break;
            case 2:
                Couleur = Qt::darkRed;
                //Disconnect();
                break;
        }
        Log->log(tr("Server : ") + Element.firstChild().nodeValue(),Couleur);
        if (Level >= 2) return true;
    }
    return false;
}

void CWebManager::OnATCUpdate()
{
#ifdef QT_DEBUG
    Log->log("WebManager : " + tr("Get ATCUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

void CWebManager::OnLiveUpdate()
{
#ifdef QT_DEBUG
    Log->log("WebManager : " + tr("Get LiveUpdate"),Qt::darkBlue,LEVEL_VERBOSE);
#endif
}

