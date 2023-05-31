#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QMap<QString,QString>>("QMap<QString,QString>");
    MainWindow w;
    w.move(0,0);
    w.show();

    return a.exec();
}
