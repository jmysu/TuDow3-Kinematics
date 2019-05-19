#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //Get the dimension available on this screen
    QRect screenSize = QGuiApplication::primaryScreen()->geometry();
    QRect mySize = QRect(0,0,1024,720); //720 is optimized for UI
    w.setFixedSize(mySize.width(), mySize.height());
    w.move((screenSize.width()-mySize.width())/2, (screenSize.height()-mySize.height())/2);
    w.show();

    return a.exec();
}
