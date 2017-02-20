#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkDatagram>

#include "misc/clogger.h"
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

private:
    Ui::MainWindow *ui;
    CLogger* Log;
    UDPServer* server;
};

#endif // MAINWINDOW_H
