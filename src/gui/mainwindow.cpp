#include "gui/mainwindow.h"
#include "ui_mainwindow.h"
#include "net/chttprequest.h"
#include "gui/dialogprofils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QPalette p = ui->pteLogger->palette();
    p.setColor(QPalette::Text,Qt::darkBlue);
    Log = CLogger::instance();
    Log->setOutput(ui->pteLogger);
    Log->setDebugLevel(LEVEL_VERBOSE);
    ui->pteLogger->setPalette(p);
    Log->log("Bienvenue sur FFS2Play",Qt::blue);
    // Initialisation Simconnect Manager
    scManager = CSCManager::instance();
    connect(scManager, SIGNAL(opened()),this, SLOT(onSCM_Opened()));
    connect(scManager, SIGNAL(closed()),this, SLOT(onSCM_Closed()));
    // Init window position and size
    s.beginGroup("MainWindow");
    resize(s.value("size", QSize(400,400)).toSize());
    move(s.value("pos", QPoint(200,200)).toPoint());
    s.endGroup();
    //Lancement du timer pour mise à jour de l'interface
    connect(&Update,&QTimer::timeout,this,&MainWindow::onUpdate);
    Update.start(5000);
    P2PManager = CP2PManager::instance();
    P2PManager->init(true);

    WebManager = CWebManager::instance();
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
    P2PManager->init(false);
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

void MainWindow::onUpdate()
{

}

void MainWindow::on_afServeurCon_triggered()
{

}

void MainWindow::on_afProfil_triggered()
{
    DialogProfils* dialog = new DialogProfils();
    dialog->show();
}
