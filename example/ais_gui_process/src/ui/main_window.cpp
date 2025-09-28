#include "main_window.h"

#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDateTime>
#include <QSettings>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serviceRunning(false)
    , connectedToService(false)
{
    setWindowTitle("AIS GUI Control - 未连接");
    setMinimumSize(1200, 800);
    
    // 加载设置
    loadSettings();
    
    // 初始化IPC客户端
    initializeIPCClient();
    
    // 创建UI
    createUI();
    createMenu();
    createSystemTray();
    connectSignals();
    
    // 更新状态栏
    statusBar()->showMessage("就绪");
}

MainWindow::~MainWindow()
{
    saveSettings();
    
    if (ipcClientManager) {
        ipcClientManager->disconnectFromServer();
        delete ipcClientManager;
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    // 窗口几何信息
    if (settings.contains("window/geometry")) {
        restoreGeometry(settings.value("window/geometry").toByteArray());
    }
    if (settings.contains("window/state")) {
        restoreState(settings.value("window/state").toByteArray());
    }
    
    // 连接设置
    QString serverAddress = settings.value("connection/server_address", "127.0.0.1").toString();
    int serverPort = settings.value("connection/server_port", 2333).toInt();
    
    // 保存到成员变量，在initializeIPCClient中使用
    // 这里简化处理，实际应该在initializeIPCClient中加载
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    // 保存窗口状态
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    
    // 保存连接设置
    if (ipcClientManager) {
        settings.setValue("connection/server_address", ipcClientManager->serverAddress());
        settings.setValue("connection/server_port", ipcClientManager->serverPort());
    }
}

void MainWindow::initializeIPCClient()
{
    QSettings settings;
    QString serverAddress = settings.value("connection/server_address", "127.0.0.1").toString();
    int serverPort = settings.value("connection/server_port", 2333).toInt();
    
    ipcClientManager = new IPCClientManager(this);
    ipcClientManager->setServerAddress(serverAddress);
    ipcClientManager->setServerPort(serverPort);
}

void MainWindow::createUI()
{
    // 创建标签页容器
    tabWidget = new QTabWidget(this);
    
    // 创建各个功能面板
    messageDisplayPanel = new MessageDisplayPanel(this);
    nmeaParserPanel = new NMEAParserPanel(this);
    serviceControlPanel = new ServiceControlPanel(this);
    
    // 设置解析器管理器到NMEA解析面板
    if (ipcClientManager->getParserManager()) {
        nmeaParserPanel->setParserManager(ipcClientManager->getParserManager());
    }
    
    // 添加标签页
    tabWidget->addTab(messageDisplayPanel, "消息显示");
    tabWidget->addTab(nmeaParserPanel, "NMEA解析");
    tabWidget->addTab(serviceControlPanel, "服务控制");
    
    setCentralWidget(tabWidget);
}

void MainWindow::createMenu()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件");
    
    connectAction = new QAction("连接服务", this);
    connect(connectAction, &QAction::triggered, this, &MainWindow::onConnectToService);
    
    disconnectAction = new QAction("断开连接", this);
    disconnectAction->setEnabled(false);
    connect(disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnectFromService);
    
    QAction *settingsAction = new QAction("设置", this);
    connect(settingsAction, &QAction::triggered, this, [this]() {
        // 打开设置对话框
    });
    
    QAction *exitAction = new QAction("退出", this);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    fileMenu->addAction(connectAction);
    fileMenu->addAction(disconnectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(settingsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图");
    QAction *toggleTrayAction = new QAction("显示/隐藏系统托盘", this);
    connect(toggleTrayAction, &QAction::triggered, this, [this]() {
        systemTrayIcon->setVisible(!systemTrayIcon->isVisible());
    });
    viewMenu->addAction(toggleTrayAction);
    
    // 工具菜单
    QMenu *toolsMenu = menuBar()->addMenu("工具");
    QAction *configAction = new QAction("服务配置", this);
    connect(configAction, &QAction::triggered, this, [this]() {
        serviceControlPanel->showConfigDialog();
    });
    toolsMenu->addAction(configAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助");
    QAction *aboutAction = new QAction("关于", this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于 AIS GUI Control", 
                          "AIS GUI Control v1.0.0\n\n"
                          "AIS数据解析和监控图形界面程序\n"
                          "版权所有 © 2024 SSZC");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::createSystemTray()
{
    systemTrayIcon = new QSystemTrayIcon(this);
    systemTrayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    systemTrayIcon->setToolTip("AIS GUI Control");
    
    trayMenu = new QMenu(this);
    QAction *showAction = new QAction("显示窗口", this);
    QAction *hideAction = new QAction("隐藏窗口", this);
    QAction *quitAction = new QAction("退出", this);
    
    connect(showAction, &QAction::triggered, this, &MainWindow::showWindow);
    connect(hideAction, &QAction::triggered, this, &MainWindow::hideWindow);
    connect(quitAction, &QAction::triggered, this, &QMainWindow::close);
    
    trayMenu->addAction(showAction);
    trayMenu->addAction(hideAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);
    
    systemTrayIcon->setContextMenu(trayMenu);
    systemTrayIcon->show();
    
    connect(systemTrayIcon, &QSystemTrayIcon::activated, 
            this, &MainWindow::onSystemTrayActivated);
}

void MainWindow::connectSignals()
{
    // 连接IPC客户端信号
    connect(ipcClientManager, &IPCClientManager::connectionStateChanged,
            this, &MainWindow::onConnectionStateChanged);
    connect(ipcClientManager, &IPCClientManager::messageReceived,
            this, &MainWindow::onMessageReceived);
    connect(ipcClientManager, &IPCClientManager::serviceStateChanged,
            this, &MainWindow::onServiceStateChanged);
    connect(ipcClientManager, &IPCClientManager::serviceConfigReceived,
            serviceControlPanel, &ServiceControlPanel::updateServiceConfig);
    connect(ipcClientManager, &IPCClientManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    
    // 连接服务控制面板信号
    connect(serviceControlPanel, &ServiceControlPanel::configChanged,
            this, &MainWindow::onServiceConfigChanged);
    connect(serviceControlPanel, &ServiceControlPanel::startServiceRequested,
            this, [this]() {
                if (connectedToService) {
                    ipcClientManager->sendStartCommand();
                } else {
                    QMessageBox::warning(this, "错误", "未连接到服务");
                }
            });
    connect(serviceControlPanel, &ServiceControlPanel::stopServiceRequested,
            this, [this]() {
                if (connectedToService) {
                    ipcClientManager->sendStopCommand();
                } else {
                    QMessageBox::warning(this, "错误", "未连接到服务");
                }
            });
    connect(serviceControlPanel, &ServiceControlPanel::restartServiceRequested,
            this, [this]() {
                if (connectedToService) {
                    ipcClientManager->sendStopCommand();
                    // 稍后发送启动命令
                    QTimer::singleShot(1000, this, [this]() {
                        ipcClientManager->sendStartCommand();
                    });
                } else {
                    QMessageBox::warning(this, "错误", "未连接到服务");
                }
            });
}

void MainWindow::onConnectToService()
{
    if (!ipcClientManager->connectToServer()) {
        QMessageBox::warning(this, "连接失败", "无法连接到AIS服务进程");
        return;
    }
    
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
}

void MainWindow::onDisconnectFromService()
{
    ipcClientManager->disconnectFromServer();
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
}

void MainWindow::onServiceStateChanged(bool running)
{
    serviceRunning = running;
    updateWindowTitle();
    
    QString status = running ? "服务运行中" : "服务已停止";
    statusBar()->showMessage(status);
    
    // 更新服务控制面板状态
    serviceControlPanel->updateServiceStatus(running);
    
    // 在系统托盘中显示状态
    if (systemTrayIcon->isVisible()) {
        systemTrayIcon->showMessage("AIS服务状态", status, 
                                   running ? QSystemTrayIcon::Information : QSystemTrayIcon::Warning);
    }
}

void MainWindow::onMessageReceived(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    messageDisplayPanel->addMessage(timestamp + " - " + message);
}

void MainWindow::onConnectionStateChanged(bool connected)
{
    connectedToService = connected;
    updateWindowTitle();
    
    if (connected) {
        statusBar()->showMessage("已连接到AIS服务");
        // 连接成功后请求服务状态和配置
        ipcClientManager->getServiceStatus();
        ipcClientManager->getServiceConfig();
    } else {
        statusBar()->showMessage("未连接");
        serviceControlPanel->updateServiceStatus(false);
    }
    
    // 更新连接按钮状态
    connectAction->setEnabled(!connected);
    disconnectAction->setEnabled(connected);
}

void MainWindow::onServiceConfigChanged(const QVariantMap &config)
{
    if (connectedToService) {
        ipcClientManager->updateServiceConfig(config);
    } else {
        QMessageBox::warning(this, "错误", "未连接到服务，无法更新配置");
    }
}

void MainWindow::onErrorOccurred(const QString &errorMessage)
{
    statusBar()->showMessage("错误: " + errorMessage);
    QMessageBox::warning(this, "错误", errorMessage);
}

void MainWindow::updateWindowTitle()
{
    QString title = "AIS GUI Control - ";
    
    if (connectedToService) {
        title += serviceRunning ? "已连接 (运行中)" : "已连接 (已停止)";
    } else {
        title += "未连接";
    }
    
    setWindowTitle(title);
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) {
            hideWindow();
        } else {
            showWindow();
        }
    }
}

void MainWindow::showWindow()
{
    show();
    activateWindow();
    raise();
}

void MainWindow::hideWindow()
{
    hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (systemTrayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        // 断开连接并清理资源
        if (ipcClientManager) {
            ipcClientManager->disconnectFromServer();
        }
        event->accept();
    }
}