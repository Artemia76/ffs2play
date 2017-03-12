#include "include/gui/dialogprofils.h"
#include "ui_dialogprofils.h"

DialogProfils::DialogProfils(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfils)
{
    ui->setupUi(this);
    Log = CLogger::instance();
}

DialogProfils::~DialogProfils()
{
    delete ui;
}

void DialogProfils::accept()
{
    Log->log("accepted");
}
