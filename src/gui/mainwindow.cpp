#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "misc/clogger.h"

#include <QTextCodec>

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
    Log->log("Bienvenue sur FFS2Play",Qt::red);
    Log->log("Test de couleur en bleu",Qt::blue);
    //Log->setDebugLevel(LEVEL_NORMAL);
    server = new UDPServer();
    connect(server, SIGNAL(fireIncomingData(QByteArray)),this, SLOT(onDatagramReceived(QByteArray)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDatagramReceived (QByteArray pData)
{
    QDataStream data(pData);
    QString Message;
    data >> Message;
    Log->log(QTextCodec::codecForMib(106)->toUnicode(pData),Qt::green);
}
