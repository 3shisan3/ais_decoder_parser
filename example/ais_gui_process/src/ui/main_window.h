#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QSystemTrayIcon>

#include "service_control_panel.h"
#include "message_display_panel.h"
#include "config_panel.h"
#include "nmea_parser_panel.h"
#include "ipc_client_manager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onServiceStateChanged(bool running);
    void onMessageReceived(const QString &message);
    void onConnectionStateChanged(bool connected);
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();
    void hideWindow();

private:
    void createUI();
    void createMenu();
    void createSystemTray();
    void connectSignals();
    void updateWindowTitle();

    // UI组件
    QTabWidget *tabWidget;
    ServiceControlPanel *serviceControlPanel;
    MessageDisplayPanel *messageDisplayPanel;
    ConfigPanel *configPanel;
    NMEAParserPanel *nmeaParserPanel;
    
    // 业务组件
    IPCClientManager *ipcClientManager;
    
    // 系统托盘
    QSystemTrayIcon *systemTrayIcon;
    QMenu *trayMenu;
    
    bool serviceRunning;
    bool connectedToService;
};

#endif // MAIN_WINDOW_H