#include "service_control_panel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDateTime>
#include <QTabWidget>

ServiceControlPanel::ServiceControlPanel(QWidget *parent)
    : QWidget(parent)
    , serviceRunning(false)
    , ipcConnected(false)
    , serviceProcessName("ais_service")
{
    // 初始化管理器
    processManager = new ProcessManager(this);
    ipcClientManager = new IPCClientManager(this);
    
    // 创建UI
    createUI();
    
    // 连接IPC信号
    connect(ipcClientManager, &IPCClientManager::connectionStateChanged,
            this, &ServiceControlPanel::onIPCConnected);
    connect(ipcClientManager, &IPCClientManager::serviceStateChanged,
            this, &ServiceControlPanel::updateServiceStatus);
    connect(ipcClientManager, &IPCClientManager::serviceConfigReceived,
            this, &ServiceControlPanel::updateServiceConfig);
    connect(ipcClientManager, &IPCClientManager::messageStatsReceived,
            this, &ServiceControlPanel::updateServiceStats);
    connect(ipcClientManager, &IPCClientManager::errorOccurred,
            this, &ServiceControlPanel::onIPCError);
    
    // 创建状态刷新定时器
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &ServiceControlPanel::onRefreshStatus);
    statusTimer->start(3000); // 每3秒刷新一次状态
    
    // 加载默认配置
    loadDefaultConfig();
}

ServiceControlPanel::~ServiceControlPanel()
{
    statusTimer->stop();
}

void ServiceControlPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建标签页容器
    QTabWidget *tabWidget = new QTabWidget(this);
    
    // 创建状态组
    createStatusGroup();
    // 创建控制组
    createControlGroup();
    // 创建进程管理组
    createProcessGroup();
    // 创建配置组
    createConfigGroup();
    // 创建统计信息组
    createStatsGroup();
    
    // 第一页：状态和控制
    QWidget *controlPage = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPage);
    controlLayout->addWidget(statusGroup);
    controlLayout->addWidget(controlGroup);
    controlLayout->addWidget(processGroup);
    controlLayout->addStretch();
    
    // 第二页：配置
    QWidget *configPage = new QWidget(this);
    QVBoxLayout *configLayout = new QVBoxLayout(configPage);
    configLayout->addWidget(configGroup);
    configLayout->addStretch();
    
    // 第三页：统计
    QWidget *statsPage = new QWidget(this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsPage);
    statsLayout->addWidget(statsGroup);
    statsLayout->addStretch();
    
    // 添加标签页
    tabWidget->addTab(controlPage, "控制");
    tabWidget->addTab(configPage, "配置");
    tabWidget->addTab(statsPage, "统计");
    
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

void ServiceControlPanel::createStatusGroup()
{
    statusGroup = new QGroupBox("服务状态", this);
    QGridLayout *layout = new QGridLayout();
    
    // 服务状态
    statusLabel = new QLabel("服务状态:", this);
    statusValueLabel = new QLabel("未知", this);
    statusValueLabel->setStyleSheet("color: gray; font-weight: bold;");
    
    // 连接状态
    connectionLabel = new QLabel("IPC连接:", this);
    connectionValueLabel = new QLabel("未连接", this);
    connectionValueLabel->setStyleSheet("color: red;");
    
    // 进程状态
    processLabel = new QLabel("进程状态:", this);
    processValueLabel = new QLabel("未运行", this);
    processValueLabel->setStyleSheet("color: red;");
    
    // 运行时间
    uptimeLabel = new QLabel("运行时间:", this);
    uptimeValueLabel = new QLabel("00:00:00", this);
    
    // 刷新按钮
    refreshButton = new QPushButton("刷新状态", this);
    connect(refreshButton, &QPushButton::clicked, this, &ServiceControlPanel::onRefreshStatus);
    
    // 布局
    layout->addWidget(statusLabel, 0, 0);
    layout->addWidget(statusValueLabel, 0, 1);
    layout->addWidget(connectionLabel, 0, 2);
    layout->addWidget(connectionValueLabel, 0, 3);
    
    layout->addWidget(processLabel, 1, 0);
    layout->addWidget(processValueLabel, 1, 1);
    layout->addWidget(uptimeLabel, 1, 2);
    layout->addWidget(uptimeValueLabel, 1, 3);
    
    layout->addWidget(refreshButton, 2, 3);
    
    statusGroup->setLayout(layout);
}

void ServiceControlPanel::createControlGroup()
{
    controlGroup = new QGroupBox("服务控制", this);
    QHBoxLayout *layout = new QHBoxLayout();
    
    // 服务控制按钮
    startButton = new QPushButton("启动服务", this);
    stopButton = new QPushButton("停止服务", this);
    restartButton = new QPushButton("重启服务", this);
    connectButton = new QPushButton("连接服务", this);
    disconnectButton = new QPushButton("断开连接", this);
    
    // 设置按钮样式
    startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    restartButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; }");
    connectButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; }");
    disconnectButton->setStyleSheet("QPushButton { background-color: #607D8B; color: white; }");
    
    // 初始状态
    stopButton->setEnabled(false);
    disconnectButton->setEnabled(false);
    restartButton->setEnabled(false);
    
    // 连接信号
    connect(startButton, &QPushButton::clicked, this, &ServiceControlPanel::onStartService);
    connect(stopButton, &QPushButton::clicked, this, &ServiceControlPanel::onStopService);
    connect(restartButton, &QPushButton::clicked, this, &ServiceControlPanel::onRestartService);
    connect(connectButton, &QPushButton::clicked, this, [this]() {
        if (ipcClientManager->connectToServer()) {
            QMessageBox::information(this, "连接成功", "已连接到AIS服务");
        }
    });
    connect(disconnectButton, &QPushButton::clicked, this, [this]() {
        ipcClientManager->disconnectFromServer();
    });
    
    layout->addWidget(startButton);
    layout->addWidget(stopButton);
    layout->addWidget(restartButton);
    layout->addWidget(connectButton);
    layout->addWidget(disconnectButton);
    layout->addStretch();
    
    controlGroup->setLayout(layout);
}

void ServiceControlPanel::createProcessGroup()
{
    processGroup = new QGroupBox("进程管理", this);
    QFormLayout *layout = new QFormLayout();
    
    // 可执行文件路径
    executableLabel = new QLabel("服务程序:", this);
    executableEdit = new QLineEdit(this);
    browseExecutableButton = new QPushButton("浏览...", this);
    
    QHBoxLayout *executableLayout = new QHBoxLayout();
    executableLayout->addWidget(executableEdit);
    executableLayout->addWidget(browseExecutableButton);
    
    // 工作目录
    workingDirLabel = new QLabel("工作目录:", this);
    workingDirEdit = new QLineEdit(this);
    browseWorkingDirButton = new QPushButton("浏览...", this);
    
    QHBoxLayout *workingDirLayout = new QHBoxLayout();
    workingDirLayout->addWidget(workingDirEdit);
    workingDirLayout->addWidget(browseWorkingDirButton);
    
    // 启动参数
    argumentsLabel = new QLabel("启动参数:", this);
    argumentsEdit = new QLineEdit(this);
    argumentsEdit->setPlaceholderText("例如: --port 2333 --config config.json");
    
    // 连接浏览按钮
    connect(browseExecutableButton, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "选择服务程序", "", "可执行文件 (*.exe);;所有文件 (*)");
        if (!path.isEmpty()) {
            executableEdit->setText(path);
            serviceExecutable = path;
        }
    });
    
    connect(browseWorkingDirButton, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "选择工作目录");
        if (!path.isEmpty()) {
            workingDirEdit->setText(path);
            serviceWorkingDir = path;
        }
    });
    
    // 添加到布局
    layout->addRow(executableLabel, executableLayout);
    layout->addRow(workingDirLabel, workingDirLayout);
    layout->addRow(argumentsLabel, argumentsEdit);
    
    processGroup->setLayout(layout);
}

void ServiceControlPanel::createConfigGroup()
{
    configGroup = new QGroupBox("服务配置", this);
    QVBoxLayout *layout = new QVBoxLayout();
    
    currentConfigLabel = new QLabel("当前配置:", this);
    
    configTextEdit = new QTextEdit(this);
    configTextEdit->setReadOnly(true);
    configTextEdit->setMaximumHeight(150);
    configTextEdit->setFontFamily("Courier New");
    configTextEdit->setFontPointSize(9);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    showConfigDialogButton = new QPushButton("编辑配置", this);
    applyConfigButton = new QPushButton("应用配置", this);
    saveConfigButton = new QPushButton("保存配置", this);
    loadConfigButton = new QPushButton("加载配置", this);
    
    connect(showConfigDialogButton, &QPushButton::clicked, this, &ServiceControlPanel::onShowConfigDialog);
    connect(applyConfigButton, &QPushButton::clicked, this, &ServiceControlPanel::onApplyConfig);
    connect(saveConfigButton, &QPushButton::clicked, this, &ServiceControlPanel::onSaveConfig);
    connect(loadConfigButton, &QPushButton::clicked, this, &ServiceControlPanel::onLoadConfig);
    
    buttonLayout->addWidget(showConfigDialogButton);
    buttonLayout->addWidget(applyConfigButton);
    buttonLayout->addWidget(saveConfigButton);
    buttonLayout->addWidget(loadConfigButton);
    buttonLayout->addStretch();
    
    layout->addWidget(currentConfigLabel);
    layout->addWidget(configTextEdit);
    layout->addLayout(buttonLayout);
    
    configGroup->setLayout(layout);
}

void ServiceControlPanel::createStatsGroup()
{
    statsGroup = new QGroupBox("服务统计", this);
    QGridLayout *layout = new QGridLayout();
    
    // 消息统计
    messagesReceivedLabel = new QLabel("接收消息:", this);
    messagesReceivedValueLabel = new QLabel("0", this);
    
    messagesParsedLabel = new QLabel("解析消息:", this);
    messagesParsedValueLabel = new QLabel("0", this);
    
    // 连接统计
    connectionsLabel = new QLabel("活动连接:", this);
    connectionsValueLabel = new QLabel("0", this);
    
    // 船舶统计
    shipCountLabel = new QLabel("船舶数量:", this);
    shipCountValueLabel = new QLabel("0", this);
    
    // 资源使用
    memoryUsageLabel = new QLabel("内存使用:", this);
    memoryUsageValueLabel = new QLabel("0 MB", this);
    
    cpuUsageLabel = new QLabel("CPU使用:", this);
    cpuUsageBar = new QProgressBar(this);
    cpuUsageBar->setRange(0, 100);
    cpuUsageBar->setValue(0);
    cpuUsageBar->setFormat("%p%");
    
    // 布局
    layout->addWidget(messagesReceivedLabel, 0, 0);
    layout->addWidget(messagesReceivedValueLabel, 0, 1);
    layout->addWidget(messagesParsedLabel, 0, 2);
    layout->addWidget(messagesParsedValueLabel, 0, 3);
    
    layout->addWidget(connectionsLabel, 1, 0);
    layout->addWidget(connectionsValueLabel, 1, 1);
    layout->addWidget(shipCountLabel, 1, 2);
    layout->addWidget(shipCountValueLabel, 1, 3);
    
    layout->addWidget(memoryUsageLabel, 2, 0);
    layout->addWidget(memoryUsageValueLabel, 2, 1);
    layout->addWidget(cpuUsageLabel, 2, 2);
    layout->addWidget(cpuUsageBar, 2, 3);
    
    statsGroup->setLayout(layout);
}

void ServiceControlPanel::loadDefaultConfig()
{
    // 设置默认服务配置
    serviceExecutable = "./ais_service";
    serviceWorkingDir = "./";
    serviceArguments = {"--port", "2333"};
    
    executableEdit->setText(serviceExecutable);
    workingDirEdit->setText(serviceWorkingDir);
    argumentsEdit->setText(serviceArguments.join(" "));
    
    // 设置默认服务配置
    currentConfig["listen_address"] = "127.0.0.1";
    currentConfig["listen_port"] = 2333;
    currentConfig["max_connections"] = 100;
    currentConfig["enable_logging"] = true;
    currentConfig["log_level"] = "INFO";
    currentConfig["data_directory"] = "./data";
    currentConfig["validate_checksum"] = true;
    currentConfig["enable_multipart"] = true;
    currentConfig["multipart_timeout"] = 300;
    currentConfig["buffer_size"] = 65536;
    currentConfig["auto_reconnect"] = true;
    currentConfig["reconnect_interval"] = 5;
    
    updateServiceConfig(currentConfig);
}

// ==================== 服务控制方法 ====================

void ServiceControlPanel::onStartService()
{
    if (startServiceProcess()) {
        QMessageBox::information(this, "启动成功", "服务进程启动成功");
        // 等待进程启动后尝试连接
        QTimer::singleShot(2000, this, [this]() {
            ipcClientManager->connectToServer();
        });
    } else {
        QMessageBox::warning(this, "启动失败", "服务进程启动失败");
    }
}

void ServiceControlPanel::onStopService()
{
    if (stopServiceProcess()) {
        QMessageBox::information(this, "停止成功", "服务进程已停止");
    } else {
        QMessageBox::warning(this, "停止失败", "服务进程停止失败");
    }
}

void ServiceControlPanel::onRestartService()
{
    if (stopServiceProcess()) {
        // 等待进程完全停止
        QTimer::singleShot(1000, this, [this]() {
            if (startServiceProcess()) {
                QMessageBox::information(this, "重启成功", "服务进程重启成功");
                // 重新连接
                QTimer::singleShot(2000, this, [this]() {
                    ipcClientManager->connectToServer();
                });
            }
        });
    }
}

bool ServiceControlPanel::startServiceProcess()
{
    if (serviceExecutable.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先设置服务程序路径");
        return false;
    }
    
    // todo 启动进程和启动服务应该拆分，流程为
    // 启动进程（或外部已手动启动进程）-> 连接进程服务 -> 查询服务（获取服务端监听udp处理ais数据流程状态）-> 操作启动，停止重启服务 -> 停止进程
    return processManager->startAttachedProcess(
        serviceExecutable,
        serviceArguments,
        serviceWorkingDir,
        serviceProcessName
    );
}

bool ServiceControlPanel::stopServiceProcess()
{
    return processManager->stopAttachedProcess(serviceProcessName);
}

bool ServiceControlPanel::isServiceProcessRunning()
{
    return processManager->isProcessRunning(serviceProcessName);
}

// ==================== 配置管理方法 ====================

void ServiceControlPanel::onApplyConfig()
{
    QVariantMap newConfig = collectConfig();
    updateServiceConfig(newConfig);
    emit configChanged(newConfig);
}

void ServiceControlPanel::onSaveConfig()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存配置", "", "JSON文件 (*.json);;所有文件 (*)");
    if (!filename.isEmpty()) {
        saveConfigToFile(filename);
    }
}

void ServiceControlPanel::onLoadConfig()
{
    QString filename = QFileDialog::getOpenFileName(this, "加载配置", "", "JSON文件 (*.json);;所有文件 (*)");
    if (!filename.isEmpty()) {
        loadConfigFromFile(filename);
    }
}

void ServiceControlPanel::onShowConfigDialog()
{
    ServiceConfigDialog dialog(this);
    dialog.setConfig(collectConfig());
    
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap newConfig = dialog.getConfig();
        updateServiceConfig(newConfig);
        emit configChanged(newConfig);
    }
}

QVariantMap ServiceControlPanel::collectConfig() const
{
    return currentConfig;
}

void ServiceControlPanel::loadConfigFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开配置文件");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, "错误", "配置文件格式无效");
        return;
    }
    
    QVariantMap config = doc.object().toVariantMap();
    updateServiceConfig(config);
    emit configChanged(config);
}

void ServiceControlPanel::saveConfigToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存配置文件");
        return;
    }
    
    QJsonDocument doc(QJsonObject::fromVariantMap(currentConfig));
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    QMessageBox::information(this, "成功", "配置已保存到文件");
}

// ==================== 状态更新方法 ====================

void ServiceControlPanel::updateServiceStatus(bool running)
{
    serviceRunning = running;
    
    if (running) {
        statusValueLabel->setText("运行中");
        statusValueLabel->setStyleSheet("color: green; font-weight: bold;");
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        restartButton->setEnabled(true);
    } else {
        statusValueLabel->setText("已停止");
        statusValueLabel->setStyleSheet("color: red; font-weight: bold;");
        startButton->setEnabled(true);
        stopButton->setEnabled(false);
        restartButton->setEnabled(false);
    }
}

void ServiceControlPanel::updateServiceConfig(const QVariantMap &config)
{
    currentConfig = config;
    
    // 更新配置显示
    QJsonDocument doc(QJsonObject::fromVariantMap(config));
    configTextEdit->setText(doc.toJson(QJsonDocument::Indented));
}

void ServiceControlPanel::updateServiceStats(const QVariantMap &stats)
{
    currentStats = stats;
    updateStatsDisplay(stats);
}

void ServiceControlPanel::onIPCConnected(bool connected)
{
    ipcConnected = connected;
    updateConnectionStatus(connected);
    
    if (connected) {
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);
        // 连接成功后获取服务状态和配置
        ipcClientManager->getServiceStatus();
        ipcClientManager->getServiceConfig();
        ipcClientManager->getMessageStats();
    } else {
        connectButton->setEnabled(true);
        disconnectButton->setEnabled(false);
        updateServiceStatus(false);
    }
}

void ServiceControlPanel::onIPCError(const QString &error)
{
    QMessageBox::warning(this, "IPC错误", error);
}

void ServiceControlPanel::onRefreshStatus()
{
    // 更新进程状态
    bool processRunning = isServiceProcessRunning();
    updateProcessStatus(processRunning);
    
    // 如果已连接，刷新服务状态
    if (ipcConnected) {
        ipcClientManager->getServiceStatus();
        ipcClientManager->getMessageStats();
        ipcClientManager->getShipCount();
    }
}

void ServiceControlPanel::updateConnectionStatus(bool connected)
{
    if (connected) {
        connectionValueLabel->setText("已连接");
        connectionValueLabel->setStyleSheet("color: green;");
    } else {
        connectionValueLabel->setText("未连接");
        connectionValueLabel->setStyleSheet("color: red;");
    }
}

void ServiceControlPanel::updateProcessStatus(bool running)
{
    if (running) {
        processValueLabel->setText("运行中");
        processValueLabel->setStyleSheet("color: green;");
    } else {
        processValueLabel->setText("未运行");
        processValueLabel->setStyleSheet("color: red;");
    }
}

void ServiceControlPanel::updateStatsDisplay(const QVariantMap &stats)
{
    if (stats.contains("messages_received")) {
        messagesReceivedValueLabel->setText(QString::number(stats["messages_received"].toInt()));
    }
    
    if (stats.contains("messages_processed")) {
        messagesParsedValueLabel->setText(QString::number(stats["messages_processed"].toInt()));
    }
    
    if (stats.contains("active_connections")) {
        connectionsValueLabel->setText(QString::number(stats["active_connections"].toInt()));
    }
    
    if (stats.contains("ship_count")) {
        shipCountValueLabel->setText(QString::number(stats["ship_count"].toInt()));
    }
    
    if (stats.contains("memory_usage")) {
        memoryUsageValueLabel->setText(QString("%1 MB").arg(stats["memory_usage"].toInt()));
    }
    
    if (stats.contains("cpu_usage")) {
        int cpuUsage = stats["cpu_usage"].toInt();
        cpuUsageBar->setValue(cpuUsage);
    }
}

// ==================== 公共方法 ====================

void ServiceControlPanel::setServiceExecutable(const QString &executablePath)
{
    serviceExecutable = executablePath;
    executableEdit->setText(executablePath);
}

void ServiceControlPanel::setServiceWorkingDir(const QString &workingDir)
{
    serviceWorkingDir = workingDir;
    workingDirEdit->setText(workingDir);
}

void ServiceControlPanel::setServiceArguments(const QStringList &arguments)
{
    serviceArguments = arguments;
    argumentsEdit->setText(arguments.join(" "));
}

QVariantMap ServiceControlPanel::getCurrentConfig() const
{
    return currentConfig;
}

void ServiceControlPanel::showConfigDialog()
{
    onShowConfigDialog();
}

// ==================== ServiceConfigDialog 实现 ====================

ServiceConfigDialog::ServiceConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    createUI();
    setWindowTitle("服务配置");
    resize(600, 500);
}

void ServiceConfigDialog::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建标签页
    QTabWidget *tabWidget = new QTabWidget(this);
    
    // 网络配置页
    QWidget *networkTab = new QWidget(this);
    QFormLayout *networkLayout = new QFormLayout(networkTab);
    
    listenAddressEdit = new QLineEdit(this);
    listenAddressEdit->setText("127.0.0.1");
    
    listenPortSpin = new QSpinBox(this);
    listenPortSpin->setRange(1024, 65535);
    listenPortSpin->setValue(2333);
    
    serverPortSpin = new QSpinBox(this);
    serverPortSpin->setRange(1024, 65535);
    serverPortSpin->setValue(2334);
    
    maxConnectionsSpin = new QSpinBox(this);
    maxConnectionsSpin->setRange(1, 1000);
    maxConnectionsSpin->setValue(100);
    
    networkLayout->addRow("监听地址:", listenAddressEdit);
    networkLayout->addRow("监听端口:", listenPortSpin);
    networkLayout->addRow("服务端口:", serverPortSpin);
    networkLayout->addRow("最大连接数:", maxConnectionsSpin);
    
    // 日志配置页
    QWidget *logTab = new QWidget(this);
    QFormLayout *logLayout = new QFormLayout(logTab);
    
    enableLoggingCheck = new QCheckBox("启用日志", this);
    enableLoggingCheck->setChecked(true);
    
    logLevelCombo = new QComboBox(this);
    logLevelCombo->addItems({"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"});
    logLevelCombo->setCurrentText("INFO");
    
    logDirectoryEdit = new QLineEdit(this);
    logDirectoryEdit->setText("./logs");
    
    logLayout->addRow("", enableLoggingCheck);
    logLayout->addRow("日志级别:", logLevelCombo);
    logLayout->addRow("日志目录:", logDirectoryEdit);
    
    // 数据配置页
    QWidget *dataTab = new QWidget(this);
    QFormLayout *dataLayout = new QFormLayout(dataTab);
    
    dataDirectoryEdit = new QLineEdit(this);
    dataDirectoryEdit->setText("./data");
    
    validateChecksumCheck = new QCheckBox("验证校验和", this);
    validateChecksumCheck->setChecked(true);
    
    enableMultipartCheck = new QCheckBox("启用多部分消息重组", this);
    enableMultipartCheck->setChecked(true);
    
    multipartTimeoutSpin = new QSpinBox(this);
    multipartTimeoutSpin->setRange(1, 600);
    multipartTimeoutSpin->setValue(300);
    multipartTimeoutSpin->setSuffix(" 秒");
    
    dataLayout->addRow("数据目录:", dataDirectoryEdit);
    dataLayout->addRow("", validateChecksumCheck);
    dataLayout->addRow("", enableMultipartCheck);
    dataLayout->addRow("多部分超时:", multipartTimeoutSpin);
    
    // 性能配置页
    QWidget *performanceTab = new QWidget(this);
    QFormLayout *performanceLayout = new QFormLayout(performanceTab);
    
    bufferSizeSpin = new QSpinBox(this);
    bufferSizeSpin->setRange(1024, 1048576);
    bufferSizeSpin->setValue(65536);
    bufferSizeSpin->setSuffix(" 字节");
    
    autoReconnectCheck = new QCheckBox("自动重连", this);
    autoReconnectCheck->setChecked(true);
    
    reconnectIntervalSpin = new QSpinBox(this);
    reconnectIntervalSpin->setRange(1, 60);
    reconnectIntervalSpin->setValue(5);
    reconnectIntervalSpin->setSuffix(" 秒");
    
    heartbeatIntervalSpin = new QSpinBox(this);
    heartbeatIntervalSpin->setRange(1, 300);
    heartbeatIntervalSpin->setValue(30);
    heartbeatIntervalSpin->setSuffix(" 秒");
    
    performanceLayout->addRow("缓冲区大小:", bufferSizeSpin);
    performanceLayout->addRow("", autoReconnectCheck);
    performanceLayout->addRow("重连间隔:", reconnectIntervalSpin);
    performanceLayout->addRow("心跳间隔:", heartbeatIntervalSpin);
    
    // 添加标签页
    tabWidget->addTab(networkTab, "网络");
    tabWidget->addTab(logTab, "日志");
    tabWidget->addTab(dataTab, "数据");
    tabWidget->addTab(performanceTab, "性能");
    
    // 按钮框
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

QVariantMap ServiceConfigDialog::getConfig() const
{
    QVariantMap config;
    
    // 网络配置
    config["listen_address"] = listenAddressEdit->text();
    config["listen_port"] = listenPortSpin->value();
    config["server_port"] = serverPortSpin->value();
    config["max_connections"] = maxConnectionsSpin->value();
    
    // 日志配置
    config["enable_logging"] = enableLoggingCheck->isChecked();
    config["log_level"] = logLevelCombo->currentText();
    config["log_directory"] = logDirectoryEdit->text();
    
    // 数据配置
    config["data_directory"] = dataDirectoryEdit->text();
    config["validate_checksum"] = validateChecksumCheck->isChecked();
    config["enable_multipart"] = enableMultipartCheck->isChecked();
    config["multipart_timeout"] = multipartTimeoutSpin->value();
    
    // 性能配置
    config["buffer_size"] = bufferSizeSpin->value();
    config["auto_reconnect"] = autoReconnectCheck->isChecked();
    config["reconnect_interval"] = reconnectIntervalSpin->value();
    config["heartbeat_interval"] = heartbeatIntervalSpin->value();
    
    return config;
}

void ServiceConfigDialog::setConfig(const QVariantMap &config)
{
    // 网络配置
    if (config.contains("listen_address")) {
        listenAddressEdit->setText(config["listen_address"].toString());
    }
    if (config.contains("listen_port")) {
        listenPortSpin->setValue(config["listen_port"].toInt());
    }
    if (config.contains("server_port")) {
        serverPortSpin->setValue(config["server_port"].toInt());
    }
    if (config.contains("max_connections")) {
        maxConnectionsSpin->setValue(config["max_connections"].toInt());
    }
    
    // 日志配置
    if (config.contains("enable_logging")) {
        enableLoggingCheck->setChecked(config["enable_logging"].toBool());
    }
    if (config.contains("log_level")) {
        logLevelCombo->setCurrentText(config["log_level"].toString());
    }
    if (config.contains("log_directory")) {
        logDirectoryEdit->setText(config["log_directory"].toString());
    }
    
    // 数据配置
    if (config.contains("data_directory")) {
        dataDirectoryEdit->setText(config["data_directory"].toString());
    }
    if (config.contains("validate_checksum")) {
        validateChecksumCheck->setChecked(config["validate_checksum"].toBool());
    }
    if (config.contains("enable_multipart")) {
        enableMultipartCheck->setChecked(config["enable_multipart"].toBool());
    }
    if (config.contains("multipart_timeout")) {
        multipartTimeoutSpin->setValue(config["multipart_timeout"].toInt());
    }
    
    // 性能配置
    if (config.contains("buffer_size")) {
        bufferSizeSpin->setValue(config["buffer_size"].toInt());
    }
    if (config.contains("auto_reconnect")) {
        autoReconnectCheck->setChecked(config["auto_reconnect"].toBool());
    }
    if (config.contains("reconnect_interval")) {
        reconnectIntervalSpin->setValue(config["reconnect_interval"].toInt());
    }
    if (config.contains("heartbeat_interval")) {
        heartbeatIntervalSpin->setValue(config["heartbeat_interval"].toInt());
    }
}