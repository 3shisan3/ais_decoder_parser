#include <QApplication>

#include "main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用信息
    QApplication::setApplicationName("AIS Data Generator");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("MarineTech");
    
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}