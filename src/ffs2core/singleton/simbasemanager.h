#ifndef SIMBASEMANAGER_H
#define SIMBASEMANAGER_H

#include <QObject>

class simbasemanager : public QObject
{
    Q_OBJECT
public:
    explicit simbasemanager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SIMBASEMANAGER_H