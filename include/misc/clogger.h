#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include <QPlainTextEdit>

class CLogger : public QObject
{
    Q_OBJECT

private:
    CLogger(QObject* parent = 0);
    static CLogger* createInstance();
    QPlainTextEdit* OutPut;
public:
    ~CLogger();
    static CLogger* instance();
    void SetOutPut(QPlainTextEdit* pOutPut);
    void Add (QString Texte="");
};

#endif // CLOGGER_H
