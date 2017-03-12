#include "include/gui/dialogprofils.h"
#include "ui_dialogprofils.h"

DialogProfils::DialogProfils(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfils)
{
    ui->setupUi(this);
    Log = CLogger::instance();
    WebManager = CWebManager::instance();

    // Fill tree with Profil List content
    for (uint i=0; i < WebManager->Users.count();i++)
    {
        QListWidgetItem* item = new QListWidgetItem(WebManager->Users.at(i).Profil);
        item->setData(Qt::UserRole,i);
        ui->listWidget->addItem(item);
    }
    s.beginGroup("Web");
    int num = s.value("Default",0).toInt();
    s.endGroup();
    if ((num>0) && (num<=ui->listWidget->count()))
    {
         QListWidgetItem *item  = ui->listWidget->item(num-1);
         ui->listWidget->setCurrentItem(item);
    }
}

DialogProfils::~DialogProfils()
{
    delete ui;
}

void DialogProfils::accept()
{
    Log->log("accepted");
}

void DialogProfils::on_btnAdd_pressed()
{
    QListWidgetItem* item = new QListWidgetItem("Profil");
    ui->listWidget->addItem(item);
    item->setSelected(true);
}

void DialogProfils::on_btnDelete_pressed()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    delete item;
}

void DialogProfils::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    SUser user = WebManager->Users.at(current->data(Qt::UserRole).toUInt());
    ui->lineEdit_Login->setText(user.Login);
    ui->lineEdit_Password->setText(user.Password);
    ui->lineEdit_ProfilName->setText(user.Profil);
    ui->lineEdit_URL->setText(user.URL);
}
