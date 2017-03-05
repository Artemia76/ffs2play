#include "net/udpserver.h"

#include <QTextCodec>
#include <QNetworkDatagram>

UDPServer::UDPServer(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<const QNetworkDatagram*>("const QNetworkDatagram*");
    Logger = CLogger::instance();
    Logger->log("UPDServer : " + tr("constructor"), Qt::darkBlue,LEVEL_NORMAL);
    m_running = false;
    m_pListenner = new QUdpSocket(this);
    connect(m_pListenner, SIGNAL(readyRead()),
                this, SLOT(incomingData()));
    m_port = 0;
}

void UDPServer::setPort(int pPort)
{
    if (!m_running) m_port = pPort;
}

void UDPServer::start ()
{
    if ((!m_running) && (m_pListenner!=NULL))
    {
        if (m_pListenner->isOpen()) stop();
        m_running = m_pListenner->bind (QHostAddress::LocalHost, m_port);
        Logger->log("UPDServer : " + tr("listenning on port ") + QString::number(m_port), Qt::darkBlue,LEVEL_NORMAL);
    }
}

void UDPServer::stop ()
{
    if ((m_running) && (m_pListenner !=NULL))
    {
        m_pListenner->close();
        m_running = false;
    }
}

UDPServer::~UDPServer()
{
    stop();
}

void UDPServer::incomingData()
{
    while (m_pListenner->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = m_pListenner->receiveDatagram();
        Logger->log("UPDServer : " + tr("receive datagram = ") +
                    QTextCodec::codecForMib(106)->toUnicode(datagram.data().toHex()),
                    Qt::darkBlue,LEVEL_VERBOSE);
        emit fireIncomingData(&datagram);
    }
}
