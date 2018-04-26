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

#include <QByteArray>

#include "tools/getappversion.h"

namespace ffs2play
{

	///
	/// \brief CWebManager::CWebManager
	/// \param parent
	/// Creates a CWebManager singleton object.


	CWebManager::CWebManager(QObject *parent) : QObject(parent)
	{
		m_bConnected=false;
		m_AIManagement=false;

		m_metaXmlCode = QMetaEnum::fromType<XmlCode>();
		qRegisterMetaTypeStreamOperators<QVector<SUser> >("QVector<SUser>");
		m_log = CLogger::instance();
		m_P2P = CP2PManager::instance();
		m_simManager = simbasemanager::instance();
		m_settings.beginGroup("Web");
		QByteArray Profils = QByteArray::fromBase64(m_settings.value("Profils","").toByteArray());
		QByteArray Check = QByteArray::fromBase64(m_settings.value("Hash","").toByteArray());
		QCryptographicHash Hash(QCryptographicHash::Md5);
		Hash.addData(Profils);
		QByteArray result = Hash.result();
		int num = m_settings.value("Default",0).toInt();
		if(Profils.isEmpty() && (num>0))
		{
			m_log->log("WebManager : " + tr("Users Profils Settings is empty but default user is set. Reset of default user"),Qt::darkMagenta);
			m_settings.setValue("Default",0);
		}
		else if (result == Check)
		{
			QDataStream stream(Profils);
			stream.setVersion(QDataStream::Qt_5_9);
			m_users.clear();
			stream >> m_users;
			if ((num > m_users.count()) || (num <1))
			{
					(m_users.count()> 0) ? num = 1 : num =0;
					m_settings.setValue("Default",num);
			}
		}
		else if (!Check.isEmpty())
		{
			m_log->log("WebManager : " + tr("Users Profils Settings checkum error, repair setting datas"),Qt::darkMagenta);
			m_settings.setValue("Default",0);
			m_settings.setValue("Profils","");
			m_settings.setValue("Hash","");
		}
		m_settings.endGroup();
		m_currentUser = num;
	#ifdef FFS_DEBUG
		m_log->log("WebManager : " + tr("AES Key generated = ") + QString(m_crypto.EncryptionKeyString()),Qt::darkBlue,LEVEL_NORMAL);
	#endif
		connect(this, SIGNAL(FireHelloCallback()),this, SLOT( SendVerify()));

		m_ATCTimer.setInterval(5000);
		m_liveUpdateTimer.setInterval(30000);
		connect(&m_ATCTimer, SIGNAL(timeout()),this,SLOT(OnATCUpdate()));
		connect(&m_liveUpdateTimer, SIGNAL(timeout()),this,SLOT(OnLiveUpdate()));
		m_liveUpdateTimer.start();
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
	/// \brief CWebManager::SaveConfig
	///

	void CWebManager::SaveConfig()
	{
		QByteArray profils;
		QDataStream stream(&profils,QIODevice::WriteOnly);
		stream.setVersion(QDataStream::Qt_5_9);
		stream << m_users;
		QCryptographicHash Hash(QCryptographicHash::Md5);
		Hash.addData(profils);
		QByteArray checksum = Hash.result();
		m_settings.beginGroup("Web");
		int num=m_currentUser;
		if (m_users.count()==0) num = 0;
		m_settings.setValue("Default",num);
		m_settings.setValue("Hash", QString(checksum.toBase64()));
		m_settings.setValue("Profils",QString(profils.toBase64()));
		m_settings.endGroup();
	}

	///
	/// \brief CWebManager::SetCurrentUser
	/// \param num
	///

	void CWebManager::SetCurrentUser(int pNum)
	{
		if ((pNum<1) || (pNum>m_users.count())) return;
		m_currentUser = pNum;
		//P2P->setCallsign(getCallSign());
	}

	///
	/// \brief CWebManager::connect_disconnect
	///

	void CWebManager::connect_disconnect()
	{
		if (m_bConnected) Disconnect();
		else Connect();
	}

	///
	/// \brief CWebManager::XmlBuild
	/// \param Doc
	/// \param Code
	///

	void CWebManager::XmlBuild (QDomDocument& pDoc, XmlCode pCode)
	{
		QDomProcessingInstruction Header = pDoc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
		pDoc.appendChild(Header);

		//Root
		QDomElement xmlRoot = pDoc.createElement("FFS2Play");
		pDoc.appendChild(xmlRoot);

		//Version
		QDomElement xmlVersion = pDoc.createElement("version");
		xmlVersion.appendChild(pDoc.createTextNode(GetAppVersion().FullVersion));
		xmlRoot.appendChild(xmlVersion);

		//Création du switch
		QDomElement xmlSwitch = pDoc.createElement("switch");
		xmlRoot.appendChild(xmlSwitch);

		//Création du data
		QDomElement xmlData = pDoc.createElement("data");
		xmlData.appendChild(pDoc.createTextNode(m_metaXmlCode.valueToKey(pCode)));
		xmlSwitch.appendChild(xmlData);

		//Création du key
		QDomElement xmlKey = pDoc.createElement("key");
		xmlKey.appendChild(pDoc.createTextNode(m_key));
		xmlSwitch.appendChild(xmlKey);

		switch (pCode)
		{
			case hello:
			{
				QDomElement xmlHello = pDoc.createElement("hello");
				xmlRoot.appendChild(xmlHello);
				QDomElement xmlLogin = pDoc.createElement("pilotID");
				xmlLogin.appendChild(pDoc.createTextNode(m_users[m_currentUser-1].Login));
				xmlHello.appendChild(xmlLogin);
				break;
			}
			case verify:
			{
				QDomElement xmlVerify = pDoc.createElement("verify");
				xmlRoot.appendChild(xmlVerify);
				QDomElement xmlAES = pDoc.createElement("AES");
				xmlAES.appendChild(pDoc.createTextNode(m_sCrypted_AESKey));
				xmlVerify.appendChild(xmlAES);
				QDomElement xmlLogin = pDoc.createElement("pilotID");
				xmlLogin.appendChild(pDoc.createTextNode(m_users[m_currentUser-1].Login));
				xmlVerify.appendChild(xmlLogin);
				QDomElement xmlPassword = pDoc.createElement("password");
				xmlPassword.appendChild(pDoc.createTextNode(m_crypto.EncryptToBase64(m_users[m_currentUser-1].Password)));
				xmlVerify.appendChild(xmlPassword);
				QDomElement xmlPort = pDoc.createElement("port");
				xmlPort.appendChild(pDoc.createTextNode(QString::number(m_P2P->getPort())));
				xmlVerify.appendChild(xmlPort);
				QDomElement xmlLocalIP = pDoc.createElement("local_ip");
				xmlLocalIP.appendChild(pDoc.createTextNode(m_P2P->getLocalIPs()));
				xmlVerify.appendChild(xmlLocalIP);
				break;
			}
			case liveupdate:
			{
				CAircraftState State = m_simManager->getAircraftState();
				QDomElement xmlLiveUpdate = pDoc.createElement("liveupdate");
				xmlRoot.appendChild(xmlLiveUpdate);
				QDomElement xmlRegistration = pDoc.createElement("registration");
				xmlRegistration.appendChild(pDoc.createTextNode(State.Title));
				xmlLiveUpdate.appendChild(xmlRegistration);
				QDomElement xmlLatitude = pDoc.createElement("latitude");
				xmlLatitude.appendChild(pDoc.createTextNode(QString::number(State.Latitude,'f',5)));
				xmlLiveUpdate.appendChild(xmlLatitude);
				QDomElement xmlLongitude = pDoc.createElement("longitude");
				xmlLongitude.appendChild(pDoc.createTextNode(QString::number(State.Longitude,'f',5)));
				xmlLiveUpdate.appendChild(xmlLongitude);
				QDomElement xmlHeading = pDoc.createElement("heading");
				xmlHeading.appendChild(pDoc.createTextNode(QString::number(State.Heading,'f',0)));
				xmlLiveUpdate.appendChild(xmlHeading);
				QDomElement xmlAltitude = pDoc.createElement("altitude");
				xmlAltitude.appendChild(pDoc.createTextNode(QString::number(State.Altitude,'f',0)));
				xmlLiveUpdate.appendChild(xmlAltitude);
				QDomElement xmlGroundSpeed = pDoc.createElement("groundSpeed");
				xmlGroundSpeed.appendChild(pDoc.createTextNode(QString::number(State.GroundSpeed,'f',0)));
				xmlLiveUpdate.appendChild(xmlGroundSpeed);
				QDomElement xmlIASpeed = pDoc.createElement("iaspeed");
				xmlIASpeed.appendChild(pDoc.createTextNode(QString::number(State.IASSpeed,'f',0)));
				xmlLiveUpdate.appendChild(xmlIASpeed);
				QDomElement xmlSquawk = pDoc.createElement("squawk");
				xmlSquawk.appendChild(pDoc.createTextNode(QString::number(State.Squawk,'f',0)));
				xmlLiveUpdate.appendChild(xmlSquawk);
				QDomElement xmlOnGround = pDoc.createElement("onground");
				xmlOnGround.appendChild(pDoc.createTextNode(QString::number(State.OnGround,'f',0)));
				xmlLiveUpdate.appendChild(xmlOnGround);
				QMetaEnum version = QMetaEnum::fromType<SimConnector::SIM_VERSION>();
				QDomElement xmlSim = pDoc.createElement("sim");
				xmlSim.appendChild(pDoc.createTextNode( version.valueToKey(m_simManager->getVersion())));
				xmlLiveUpdate.appendChild(xmlSim);
				break;
			}
			case atc:
			{
				CAircraftState State = m_simManager->getAircraftState();
				QDomElement xmlAtc = pDoc.createElement("atc");
				xmlRoot.appendChild(xmlAtc);
				QDomElement xmlLatitude = pDoc.createElement("latitude");
				xmlLatitude.appendChild(pDoc.createTextNode(QString::number(State.Latitude,'f',5)));
				xmlAtc.appendChild(xmlLatitude);
				QDomElement xmlLongitude = pDoc.createElement("longitude");
				xmlLongitude.appendChild(pDoc.createTextNode(QString::number(State.Longitude,'f',5)));
				xmlAtc.appendChild(xmlLongitude);
				QDomElement xmlHeading = pDoc.createElement("heading");
				xmlHeading.appendChild(pDoc.createTextNode(QString::number(State.Heading,'f',0)));
				xmlAtc.appendChild(xmlHeading);
				QDomElement xmlAltitude = pDoc.createElement("altitude");
				xmlAltitude.appendChild(pDoc.createTextNode(QString::number(State.Altitude,'f',0)));
				xmlAtc.appendChild(xmlAltitude);
				QDomElement xmlGroundSpeed = pDoc.createElement("groundSpeed");
				xmlAtc.appendChild(pDoc.createTextNode(QString::number(State.GroundSpeed,'f',0)));
				xmlAtc.appendChild(xmlGroundSpeed);
				QDomElement xmlIASpeed = pDoc.createElement("iaspeed");
				xmlIASpeed.appendChild(pDoc.createTextNode(QString::number(State.IASSpeed,'f',0)));
				xmlAtc.appendChild(xmlIASpeed);
				QDomElement xmlSquawk = pDoc.createElement("squawk");
				xmlSquawk.appendChild(pDoc.createTextNode(QString::number(State.Squawk,'f',0)));
				xmlAtc.appendChild(xmlSquawk);
				break;
			}
			default:
			{
				break;
			}
		}
	}
}
