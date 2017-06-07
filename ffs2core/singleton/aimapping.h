#ifndef AIMAPPING_H
#define AIMAPPING_H

#include <QObject>
#include "singleton/clogger.h"

struct AIResol
{
    QString Titre;
    double  CG_Height;
    double  Pitch;
};

class AIMapping : public QObject
{
    Q_OBJECT
private:
    explicit AIMapping(QObject *parent = nullptr);
    static AIMapping* createInstance();
    CLogger*     Log;

public:
    ~AIMapping();
    static AIMapping* instance();
signals:

public slots:
};

#endif // AIMAPPING_H
