#ifndef DIALOGPROFILS_H
#define DIALOGPROFILS_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSettings>
#include <QAbstractButton>

#include "singleton/clogger.h"
#include "singleton/webmanager.h"

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
    CLogger*        Log;
    CWebManager*    WebManager;
    QSettings       s;
    void lineEditEnable(bool flag);
private slots:
    void on_btnAdd_pressed();
    void on_btnDelete_pressed();
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_lineEdit_ProfilName_textChanged(const QString &arg1);
};

#endif // DIALOGPROFILS_H
