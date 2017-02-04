#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "misc/clogger.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    Log->SetOutPut(ui->pteLogger);
    ui->pteLogger->setPalette(p);
    Log->Add("Bienvenue sur FFS2Play");
}

MainWindow::~MainWindow()
{
    delete ui;
}
