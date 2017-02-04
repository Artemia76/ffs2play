#include "misc/clogger.h"
#include "misc/singleton.h"

CLogger::CLogger(QObject* parent):
    QObject(parent)
{
    OutPut = NULL;
}

CLogger* CLogger::createInstance()
{
    return new CLogger();
}

CLogger::~CLogger()
{

}

CLogger* CLogger::instance()
{
    return Singleton<CLogger>::instance(CLogger::createInstance);
}

void CLogger::SetOutPut(QPlainTextEdit* pOutPut)
{
    OutPut = pOutPut;
}

void CLogger::Add (QString Texte)
{
    if (OutPut==NULL) return;
    OutPut->appendPlainText(Texte);
}

