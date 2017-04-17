#include "singleton/clogger.h"
#include "singleton/singleton.h"

CLogger::CLogger(QObject* parent):
    QObject(parent)
{
    qRegisterMetaType<CL_DEBUG_LEVEL>("CL_DEBUG_LEVEL");
    m_Output = NULL;
    s.beginGroup("Log");
    m_DebugLevel = (CL_DEBUG_LEVEL)s.value("DebugLevel",LEVEL_NONE).toLongLong();
    s.endGroup();
    connect (this,SIGNAL(fireLog(QString,QColor,CL_DEBUG_LEVEL)),this,SLOT(_log(QString,QColor,CL_DEBUG_LEVEL)));
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

void CLogger::setOutput(QPlainTextEdit* pOutput)
{
    m_Output = pOutput;
}

void CLogger::setDebugLevel(CL_DEBUG_LEVEL pLevel)
{
    if (pLevel < LEVEL_NONE ) m_DebugLevel=LEVEL_NONE;
    else if (pLevel > LEVEL_VERBOSE) m_DebugLevel=LEVEL_VERBOSE;
    else m_DebugLevel = pLevel;
    s.beginGroup("Log");
    s.setValue("DebugLevel",(long)m_DebugLevel);
    s.endGroup();
}

void CLogger::_log (const QString& Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    if ((m_Output==NULL) || (pLevel > m_DebugLevel)) return;
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(pColor));
    m_Output->mergeCurrentCharFormat(fmt);
    m_Output->appendPlainText(Texte);
}

void CLogger::log (const QString& Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    emit(fireLog(Texte,pColor,pLevel));
}
