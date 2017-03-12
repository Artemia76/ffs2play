#include "singleton/webmanager.h"
#include "singleton/singleton.h"

#include <QCryptographicHash>
#include <QByteArray>

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


CWebManager::CWebManager(QObject *parent) : QObject(parent)
{
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
    }
    else if (!Check.isEmpty())
    {
        Log->log(tr("WebManager : Users Profils Settings checkum error, repair setting data"),Qt::darkMagenta);
        s.setValue("Default",0);
        s.setValue("Profils","");
        s.setValue("Hash","");
    }
    s.endGroup();
    if (num < Users.count()) CurentUser = Users.at(num);
}

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
