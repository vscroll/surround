#include "mainwindow.h"
#include <QApplication>
//#include <QWSServer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOverrideCursor(Qt::BlankCursor);
    //QWSServer::setCursorVisible(false);
    MainWindow w;
    w.show();

    return a.exec();
}
