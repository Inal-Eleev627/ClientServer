#include "mainwindow.h"
#include "clientdialog.h"

#include <QApplication>
#include <boost/asio.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    return a.exec();
}
