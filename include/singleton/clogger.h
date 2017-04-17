#ifndef CLOGGER_H
#define CLOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QSettings>

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
    QPlainTextEdit* m_Output;
    CL_DEBUG_LEVEL  m_DebugLevel;
    QSettings       s;

private slots:
    void            _log (const QString& Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);

public:
                    ~CLogger();
    static CLogger* instance();
    void            setOutput(QPlainTextEdit* pOutput);
    void            setDebugLevel (CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
    void            log (const QString& Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);

signals:
    void            fireLog (QString Texte="", QColor pColor = Qt::black,CL_DEBUG_LEVEL pLevel=LEVEL_NONE);
};

#endif // CLOGGER_H
