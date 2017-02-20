#include "net/udpserver.h"

#include <QTextCodec>
#include <QNetworkDatagram>

UDPServer::UDPServer(QObject *parent) : QObject(parent)
{
    Logger = CLogger::instance();
    Logger->log("Création du serveur UDP", Qt::darkBlue,LEVEL_NORMAL);
    m_pListenner = new QUdpSocket(this);
    if (m_pListenner->bind (QHostAddress::LocalHost, 7755))
    {
        connect(m_pListenner, SIGNAL(readyRead()),
                    this, SLOT(incomingData()));
    }

}

UDPServer::~UDPServer()
{

}

void UDPServer::incomingData()
{
    while (m_pListenner->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_pListenner->receiveDatagram();
        Logger->log("recu un datagram : " + QTextCodec::codecForMib(106)->toUnicode(datagram.data().toHex()));
        emit fireIncomingData(datagram.data());
    }
}
