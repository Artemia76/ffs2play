#ifndef CHTTPREQUEST_H
#define CHTTPREQUEST_H

#include <QObject>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>

class CHttpRequest : public QObject
{
    Q_OBJECT
public:
    explicit CHttpRequest(QObject *parent = 0);
    ~CHttpRequest();
    void execute(QString URL, QDomDocument& Doc);
    void execute(QString URL);
    QString getAsString();
private:
    QNetworkRequest m_request;
    QNetworkReply* m_reply;
    static QNetworkAccessManager* m_ptNetworkManager;
private slots:
    void callback();
signals:
    void finished(CHttpRequest*);
};

#endif // CHTTPREQUEST_H
