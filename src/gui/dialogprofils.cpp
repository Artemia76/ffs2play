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
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    lineEditEnable(false);
}

DialogProfils::~DialogProfils()
{
    delete ui;
}

void DialogProfils::lineEditEnable(bool flag)
{
    ui->lineEdit_ProfilName->setEnabled(flag);
    ui->lineEdit_Login->setEnabled(flag);
    ui->lineEdit_Password->setEnabled(flag);
    ui->lineEdit_URL->setEnabled(flag);
}

void DialogProfils::on_btnAdd_pressed()
{
    SUser User;
    User.Profil = "Profil";
    int Row = ui->listWidget->currentRow();
    WebManager->Users.insert(Row+1,User);
    QListWidgetItem* item = new QListWidgetItem("Profil");
    ui->listWidget->insertItem(Row+1,item);
    ui->listWidget->setCurrentRow(Row+1);
}

void DialogProfils::on_btnDelete_pressed()
{
    QListWidgetItem *item = ui->listWidget->takeItem(ui->listWidget->currentRow());
    delete item;
}

void DialogProfils::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if ((ui->listWidget->count() >0) && (current!=NULL))
    {
        SUser user = WebManager->Users.at(ui->listWidget->currentRow());
        ui->lineEdit_Login->setText(user.Login);
        ui->lineEdit_Password->setText(user.Password);
        ui->lineEdit_ProfilName->setText(user.Profil);
        ui->lineEdit_URL->setText(user.URL);
        lineEditEnable(true);
    }
    else
    {
        ui->lineEdit_Login->clear();
        ui->lineEdit_Password->clear();
        ui->lineEdit_ProfilName->clear();
        ui->lineEdit_URL->clear();
        lineEditEnable(false);
    }
}

void DialogProfils::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button == ui->buttonBox->button(QDialogButtonBox::Apply))
    {
       Log->log("Apply button");
    }
    if (button == ui->buttonBox->button(QDialogButtonBox::Ok))
    {
       Log->log("Ok button");
    }
    if (button == ui->buttonBox->button(QDialogButtonBox::Cancel))
    {
       Log->log("Cancel button");
    }
}

void DialogProfils::on_lineEdit_ProfilName_textChanged(const QString &arg1)
{

}
