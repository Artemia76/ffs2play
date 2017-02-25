#ifndef SIMCONNECT_H
#define SIMCONNECT_H

#include <QObject>

class simconnect : public QObject
{
    Q_OBJECT
public:
    explicit simconnect(QObject *parent = 0);

signals:

public slots:
};

#endif // SIMCONNECT_H