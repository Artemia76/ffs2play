#include "gui/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    ui->pteLogger->setPalette(p);
    ui->pteLogger->appendPlainText("Bienvenue sur FFS2Play");
}

MainWindow::~MainWindow()
{
    delete ui;
}
