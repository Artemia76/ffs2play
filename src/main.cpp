#include "gui/mainwindow.h"
#include "net/udpserver.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QCoreApplication::setOrganizationName("FFS2P");
    QCoreApplication::setOrganizationDomain("ffsimulateur2.fr");
    QCoreApplication::setApplicationName("FFS2Play");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
