#include "gui/mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    Log->setOutput(ui->pteLogger);
    ui->pteLogger->setPalette(p);
    Log->log("Bienvenue sur FFS2Play",Qt::blue);
    udpServer = new UDPServer();
    connect(udpServer, SIGNAL(fireIncomingData(QByteArray)),this, SLOT(onDatagramReceived(QByteArray)));
    // Initialisation Simconnect Manager
    scManager = CSCManager::instance();
    connect(scManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
    connect(scManager, SIGNAL(closed()),this, SLOT(onSCM_Closed()));
    s.beginGroup("MainWindow");
    resize(s.value("size", QSize(400,400)).toSize());
    move(s.value("pos", QPoint(200,200)).toPoint());
    s.endGroup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    s.beginGroup("MainWindow");
    s.setValue("size", size());
    s.setValue("pos", pos());
    s.endGroup();
}

void MainWindow::onDatagramReceived (QByteArray pData)
{
    QDataStream data(pData);
    QString Message;
    data >> Message;
    Log->log(QTextCodec::codecForMib(106)->toUnicode(pData),Qt::green);
}

void MainWindow::on_atFSCon_triggered()
{
    scManager->open_close();
}

void MainWindow::onSCM_Opened()
{
    ui->atFSCon->setIcon(QIcon(":/gfx/plane_green.png"));
}

void MainWindow::onSCM_Closed()
{
    ui->atFSCon->setIcon(QIcon(":/gfx/plane_gris.png"));
}
