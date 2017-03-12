#ifndef DIALOGPROFILS_H
#define DIALOGPROFILS_H

#include <QDialog>
#include "singleton/clogger.h"

namespace Ui {
class DialogProfils;
}

class DialogProfils : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProfils(QWidget *parent = 0);
    ~DialogProfils();

private:
    Ui::DialogProfils *ui;
    CLogger*    Log;
private slots:
    void accept();
};

#endif // DIALOGPROFILS_H
