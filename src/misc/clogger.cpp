#include "misc/clogger.h"
#include "misc/singleton.h"

CLogger::CLogger(QObject* parent):
    QObject(parent)
{
    m_pOutput = NULL;
    m_eDebugLevel = LEVEL_NONE;
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
    m_pOutput = pOutput;
}

void CLogger::setDebugLevel(CL_DEBUG_LEVEL pLevel)
{
    if (pLevel < LEVEL_NONE ) m_eDebugLevel=LEVEL_NONE;
    else if (pLevel > LEVEL_VERBOSE) m_eDebugLevel=LEVEL_VERBOSE;
    else m_eDebugLevel = pLevel;
}

void CLogger::log (QString Texte,QColor pColor, CL_DEBUG_LEVEL pLevel)
{
    if ((m_pOutput==NULL) || (pLevel > m_eDebugLevel)) return;
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(pColor));
    m_pOutput->mergeCurrentCharFormat(fmt);
    m_pOutput->appendPlainText(Texte);
}

