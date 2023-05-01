#include "mainwindow.h"

#include <QApplication>
#include "core/UnitTest.h"

int main(int argc, char *argv[])
{
    testGeometry();
    testTransform();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
