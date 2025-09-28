#include "service_control_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

ServiceControlPanel::ServiceControlPanel(IPCClientManager *ipcManager, QWidget *parent)
    : QWidget(parent)
    , ipcManager(ipcManager)
    , serviceRunning(false)
    , connectedToService(false)
{
    createUI();
    connectSignals();
    updateUI();
}

void ServiceControlPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 状态显示组
    QGroupBox *statusGroup = new QGroupBox("服务状态", this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    connectionLabel = new QLabel("连接状态: 未连接", this);
    statusLabel = new QLabel("服务状态: 未知", this);
    
    statusLayout->addWidget(connectionLabel);
    statusLayout->addWidget(statusLabel);
    statusGroup->setLayout(statusLayout);
    
    // 控制按钮组
    QGroupBox *controlGroup = new QGroupBox("服务控制", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);
    
    startButton = new QPushButton("启动服务", this);
    stopButton = new QPushButton("停止服务", this);
    refreshButton = new QPushButton("刷新状态", this);
    
    controlLayout->addWidget(startButton);
    controlLayout->addWidget(stopButton);
    controlLayout->addWidget(refreshButton);
    controlGroup->setLayout(controlLayout);
    
    mainLayout->addWidget(statusGroup);
    mainLayout->addWidget(controlGroup);
    mainLayout->addStretch();
    
    setLayout(mainLayout);
    
    connect(startButton, &QPushButton::clicked, this, &ServiceControlPanel::onStartService);
    connect(stopButton, &QPushButton::clicked, this, &ServiceControlPanel::onStopService);
    connect(refreshButton, &QPushButton::clicked, this, &ServiceControlPanel::onRefreshStatus);
}

void ServiceControlPanel::connectSignals()
{
    connect(ipcManager, &IPCClientManager::serviceStateChanged, 
            this, &ServiceControlPanel::onServiceStateChanged);
    connect(ipcManager, &IPCClientManager::connected, 
            this, [this]() { onConnectionStateChanged(true); });
    connect(ipcManager, &IPCClientManager::disconnected, 
            this, [this]() { onConnectionStateChanged(false); });
}

void ServiceControlPanel::onServiceStateChanged(bool running)
{
    serviceRunning = running;
    updateUI();
}

void ServiceControlPanel::onConnectionStateChanged(bool connected)
{
    connectedToService = connected;
    updateUI();
}

void ServiceControlPanel::onStartService()
{
    if (!connectedToService) {
        QMessageBox::warning(this, "警告", "请先连接到AIS服务");
        return;
    }
    ipcManager->startService();
}

void ServiceControlPanel::onStopService()
{
    if (!connectedToService) {
        QMessageBox::warning(this, "警告", "请先连接到AIS服务");
        return;
    }
    ipcManager->stopService();
}

void ServiceControlPanel::onRefreshStatus()
{
    if (!connectedToService) {
        QMessageBox::warning(this, "警告", "请先连接到AIS服务");
        return;
    }
    ipcManager->getServiceStatus();
}

void ServiceControlPanel::updateUI()
{
    connectionLabel->setText(QString("连接状态: %1")
                            .arg(connectedToService ? "已连接" : "未连接"));
    
    if (connectedToService) {
        statusLabel->setText(QString("服务状态: %1")
                           .arg(serviceRunning ? "运行中" : "已停止"));
    } else {
        statusLabel->setText("服务状态: 未知");
    }
    
    startButton->setEnabled(connectedToService && !serviceRunning);
    stopButton->setEnabled(connectedToService && serviceRunning);
    refreshButton->setEnabled(connectedToService);
}