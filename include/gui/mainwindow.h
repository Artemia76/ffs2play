#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCodec>
#include <QSettings>
#include <QTimer>

#include "singleton/clogger.h"
#include "singleton/scmanager.h"
#include "net/chttprequest.h"
#include "singleton/p2pmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_atFSCon_triggered();
    void onSCM_Opened();
    void onSCM_Closed();
    void onUpdate();
private:
    Ui::MainWindow *ui;
    CLogger* Log;
    CSCManager* scManager;
    CP2PManager* P2PManager;
    QSettings s;
    QTimer Update;

    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
