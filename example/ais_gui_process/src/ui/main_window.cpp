#include "main_window.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , serviceRunning(false)
    , connectedToService(false)
{
    setWindowTitle("AIS GUI Control - 未连接");
    setMinimumSize(1000, 700);
    
    // 创建业务组件
    ipcClientManager = new IPCClientManager(this);
    
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
    ipcClientManager->disconnectFromServer();
}

void MainWindow::createUI()
{
    // 创建标签页容器
    tabWidget = new QTabWidget(this);
    
    // 创建各个功能面板
    serviceControlPanel = new ServiceControlPanel(ipcClientManager, this);
    messageDisplayPanel = new MessageDisplayPanel(this);
    configPanel = new ConfigPanel(ipcClientManager, this);
    nmeaParserPanel = new NMEAParserPanel(this);
    
    // 添加标签页
    tabWidget->addTab(serviceControlPanel, "服务控制");
    tabWidget->addTab(messageDisplayPanel, "消息显示");
    tabWidget->addTab(configPanel, "配置管理");
    tabWidget->addTab(nmeaParserPanel, "NMEA解析");
    
    setCentralWidget(tabWidget);
}

void MainWindow::createMenu()
{
    QMenu *fileMenu = menuBar()->addMenu("文件");
    
    QAction *connectAction = new QAction("连接服务", this);
    connect(connectAction, &QAction::triggered, this, [this]() {
        ipcClientManager->connectToServer("127.0.0.1", 2333);
    });
    
    QAction *disconnectAction = new QAction("断开连接", this);
    connect(disconnectAction, &QAction::triggered, this, [this]() {
        ipcClientManager->disconnectFromServer();
    });
    
    QAction *exitAction = new QAction("退出", this);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    fileMenu->addAction(connectAction);
    fileMenu->addAction(disconnectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    QMenu *viewMenu = menuBar()->addMenu("视图");
    QAction *toggleTrayAction = new QAction("显示/隐藏系统托盘", this);
    connect(toggleTrayAction, &QAction::triggered, this, [this]() {
        systemTrayIcon->setVisible(!systemTrayIcon->isVisible());
    });
    viewMenu->addAction(toggleTrayAction);
}

void MainWindow::createSystemTray()
{
    systemTrayIcon = new QSystemTrayIcon(this);
    systemTrayIcon->setIcon(QIcon(":/icons/ais_icon.png"));
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
    connect(ipcClientManager, &IPCClientManager::connected, 
            this, &MainWindow::onConnectionStateChanged);
    connect(ipcClientManager, &IPCClientManager::disconnected, 
            this, &MainWindow::onConnectionStateChanged);
    connect(ipcClientManager, &IPCClientManager::messageReceived, 
            this, &MainWindow::onMessageReceived);
    connect(ipcClientManager, &IPCClientManager::serviceStateChanged, 
            this, &MainWindow::onServiceStateChanged);
    connect(ipcClientManager, &IPCClientManager::errorOccurred, 
            this, [this](const QString &error) {
        statusBar()->showMessage("错误: " + error);
    });
}

void MainWindow::onServiceStateChanged(bool running)
{
    serviceRunning = running;
    updateWindowTitle();
    statusBar()->showMessage(running ? "服务运行中" : "服务已停止");
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
        // 连接成功后请求服务状态
        ipcClientManager->getServiceStatus();
    } else {
        statusBar()->showMessage("未连接");
    }
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
        event->accept();
    }
}