#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkDatagram>

#include "singleton/clogger.h"
#include "singleton/scmanager.h"
#include "net/udpserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void onDatagramReceived (QByteArray);

private slots:
    void on_atFSCon_triggered();
    void onSCM_Opened();
    void onSCM_Closed();
private:
    Ui::MainWindow *ui;
    CLogger* Log;
    UDPServer* udpServer;
    CSCManager* scManager;
};

#endif // MAINWINDOW_H
