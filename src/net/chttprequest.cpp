#include "net/chttprequest.h"

QNetworkAccessManager* CHttpRequest::m_ptNetworkManager = NULL;

CHttpRequest::CHttpRequest(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<CHttpRequest*>("CHttpRequest*");
    if (m_ptNetworkManager==NULL) m_ptNetworkManager = new QNetworkAccessManager();
    m_request.setRawHeader("User-Agent","FFS2Play");
}

CHttpRequest::~CHttpRequest()
{
    if (m_reply) m_reply->deleteLater();
}

void CHttpRequest::execute(QString URL, QDomDocument& Doc)
{
    m_request.setUrl(QUrl(URL));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml; encoding='utf-8'");
    QByteArray content = Doc.toByteArray();
    m_reply = m_ptNetworkManager->post(m_request,content);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
}

void CHttpRequest::execute(QString URL)
{
    m_request.setUrl(QUrl(URL));
    m_reply = m_ptNetworkManager->get(m_request);
    connect(m_reply,&QNetworkReply::finished,this,&CHttpRequest::callback);
}

void CHttpRequest::callback()
{
    emit(finished(this));
}

QString CHttpRequest::getAsString()
{
    QString Reponse;
    if (m_reply!= NULL) Reponse = m_reply->readAll();
    return Reponse;
}
