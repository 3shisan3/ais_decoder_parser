#include "ui/main_window.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("AIS GUI Control");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("SSZC");
    
    // 设置 Fusion 样式
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // 创建主窗口
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}