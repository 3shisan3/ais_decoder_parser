#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QAction>

#include "ui/widgets/message_display_panel.h"
#include "ui/widgets/nmea_parser_panel.h"
#include "ui/widgets/service_control_panel.h"
#include "core/ipc_client_manager.h"

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
    void onServiceStatsUpdated(const QVariantMap &stats);
    void onShipCountReceived(int count);
    void onMessageReceived(const QString &message);
    void onAisMessageReceived(const QString &rawData, const QString &processedData);
    void onRawAisMessageReceived(const QString &rawData);
    void onConnectionStateChanged(bool connected);
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();
    void hideWindow();
    void onConnectToService();
    void onDisconnectFromService();
    void onServiceConfigChanged(const QVariantMap &config);
    void onErrorOccurred(const QString &errorMessage);

private:
    void createUI();
    void createMenu();
    void createSystemTray();
    void connectSignals();
    void updateWindowTitle();
    void initializeIPCClient();
    void loadSettings();
    void saveSettings();
    void initializeServiceControl();
    void updateServiceControlStatus();

    // UI组件
    QTabWidget *tabWidget;
    MessageDisplayPanel *messageDisplayPanel;
    NMEAParserPanel *nmeaParserPanel;
    ServiceControlPanel *serviceControlPanel;
    
    // IPC客户端管理
    IPCClientManager *ipcClientManager;
    
    // 系统托盘
    QSystemTrayIcon *systemTrayIcon;
    QMenu *trayMenu;
    
    // 状态变量
    bool serviceRunning;
    bool connectedToService;
    
    // 菜单动作
    QAction *connectAction;
    QAction *disconnectAction;
};

#endif // MAIN_WINDOW_H