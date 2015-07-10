#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	//a.setStyle("Fusion");
    MainWindow w;
    w.show();
    
    return a.exec();
}
