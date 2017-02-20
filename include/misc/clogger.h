#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include <QPlainTextEdit>

enum CL_DEBUG_LEVEL
{
    LEVEL_NONE,
    LEVEL_NORMAL,
    LEVEL_VERBOSE
};

class CLogger : public QObject
{
    Q_OBJECT

private:
    CLogger(QObject* parent = 0);
    static CLogger* createInstance();
    QPlainTextEdit* m_pOutput;
    CL_DEBUG_LEVEL m_eDebugLevel;
public:
    ~CLogger();
    static CLogger* instance();
    void setOutput(QPlainTextEdit* pOutput);
    void log (QString Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
    void setDebugLevel (CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
};

#endif // CLOGGER_H
