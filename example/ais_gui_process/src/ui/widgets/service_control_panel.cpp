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

ServiceControlPanel::ServiceControlPanel(QWidget *parent)
    : QWidget(parent)
    , serviceRunning(false)
{
    createUI();
    
    // 创建状态刷新定时器
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &ServiceControlPanel::onRefreshStatus);
    statusTimer->start(5000); // 每5秒刷新一次状态
}

void ServiceControlPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建状态组
    createStatusGroup();
    // 创建控制组
    createControlGroup();
    // 创建配置组
    createConfigGroup();
    // 创建统计信息组
    createStatsGroup();
    
    mainLayout->addWidget(statusGroup);
    mainLayout->addWidget(controlGroup);
    mainLayout->addWidget(configGroup);
    mainLayout->addWidget(statsGroup);
    mainLayout->addStretch();
    
    setLayout(mainLayout);
}

void ServiceControlPanel::createStatusGroup()
{
    statusGroup = new QGroupBox("服务状态", this);
    QGridLayout *layout = new QGridLayout();
    
    statusLabel = new QLabel("状态:", this);
    statusValueLabel = new QLabel("未知", this);
    statusValueLabel->setStyleSheet("color: gray;");
    
    uptimeLabel = new QLabel("运行时间:", this);
    uptimeValueLabel = new QLabel("00:00:00", this);
    
    versionLabel = new QLabel("版本:", this);
    versionValueLabel = new QLabel("1.0.0", this);
    
    refreshButton = new QPushButton("刷新", this);
    connect(refreshButton, &QPushButton::clicked, this, &ServiceControlPanel::onRefreshStatus);
    
    layout->addWidget(statusLabel, 0, 0);
    layout->addWidget(statusValueLabel, 0, 1);
    layout->addWidget(uptimeLabel, 0, 2);
    layout->addWidget(uptimeValueLabel, 0, 3);
    layout->addWidget(versionLabel, 1, 0);
    layout->addWidget(versionValueLabel, 1, 1);
    layout->addWidget(refreshButton, 1, 3);
    
    statusGroup->setLayout(layout);
}

void ServiceControlPanel::createControlGroup()
{
    controlGroup = new QGroupBox("服务控制", this);
    QHBoxLayout *layout = new QHBoxLayout();
    
    startButton = new QPushButton("启动服务", this);
    stopButton = new QPushButton("停止服务", this);
    restartButton = new QPushButton("重启服务", this);
    
    startButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    stopButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    restartButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; }");
    
    connect(startButton, &QPushButton::clicked, this, &ServiceControlPanel::onStartService);
    connect(stopButton, &QPushButton::clicked, this, &ServiceControlPanel::onStopService);
    connect(restartButton, &QPushButton::clicked, this, &ServiceControlPanel::onRestartService);
    
    layout->addWidget(startButton);
    layout->addWidget(stopButton);
    layout->addWidget(restartButton);
    layout->addStretch();
    
    controlGroup->setLayout(layout);
}

void ServiceControlPanel::createConfigGroup()
{
    configGroup = new QGroupBox("服务配置", this);
    QVBoxLayout *layout = new QVBoxLayout();
    
    currentConfigLabel = new QLabel("当前配置:", this);
    
    configTextEdit = new QTextEdit(this);
    configTextEdit->setReadOnly(true);
    configTextEdit->setMaximumHeight(100);
    configTextEdit->setFontFamily("Courier New");
    
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
    
    messagesReceivedLabel = new QLabel("接收消息:", this);
    messagesReceivedValueLabel = new QLabel("0", this);
    
    messagesParsedLabel = new QLabel("解析消息:", this);
    messagesParsedValueLabel = new QLabel("0", this);
    
    connectionsLabel = new QLabel("活动连接:", this);
    connectionsValueLabel = new QLabel("0", this);
    
    memoryUsageLabel = new QLabel("内存使用:", this);
    memoryUsageValueLabel = new QLabel("0 MB", this);
    
    cpuUsageLabel = new QLabel("CPU使用:", this);
    cpuUsageBar = new QProgressBar(this);
    cpuUsageBar->setRange(0, 100);
    cpuUsageBar->setValue(0);
    
    layout->addWidget(messagesReceivedLabel, 0, 0);
    layout->addWidget(messagesReceivedValueLabel, 0, 1);
    layout->addWidget(messagesParsedLabel, 0, 2);
    layout->addWidget(messagesParsedValueLabel, 0, 3);
    
    layout->addWidget(connectionsLabel, 1, 0);
    layout->addWidget(connectionsValueLabel, 1, 1);
    layout->addWidget(memoryUsageLabel, 1, 2);
    layout->addWidget(memoryUsageValueLabel, 1, 3);
    
    layout->addWidget(cpuUsageLabel, 2, 0);
    layout->addWidget(cpuUsageBar, 2, 1, 1, 3);
    
    statsGroup->setLayout(layout);
}

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

void ServiceControlPanel::onStartService()
{
    emit startServiceRequested();
}

void ServiceControlPanel::onStopService()
{
    emit stopServiceRequested();
}

void ServiceControlPanel::onRestartService()
{
    emit restartServiceRequested();
}

void ServiceControlPanel::onApplyConfig()
{
    emit configChanged(collectConfig());
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
    ConfigDialog dialog(this);
    dialog.setConfig(collectConfig());
    
    if (dialog.exec() == QDialog::Accepted) {
        QVariantMap newConfig = dialog.getConfig();
        updateServiceConfig(newConfig);
        emit configChanged(newConfig);
    }
}

void ServiceControlPanel::onRefreshStatus()
{
    // 这里可以发送状态查询命令
    // 在实际实现中，应该通过IPC客户端发送状态查询请求
}

void ServiceControlPanel::onServiceStatsUpdated(const QVariantMap &stats)
{
    updateStatsDisplay(stats);
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
}

void ServiceControlPanel::updateStatsDisplay(const QVariantMap &stats)
{
    if (stats.contains("messages_received")) {
        messagesReceivedValueLabel->setText(QString::number(stats["messages_received"].toInt()));
    }
    
    if (stats.contains("messages_parsed")) {
        messagesParsedValueLabel->setText(QString::number(stats["messages_parsed"].toInt()));
    }
    
    if (stats.contains("active_connections")) {
        connectionsValueLabel->setText(QString::number(stats["active_connections"].toInt()));
    }
    
    if (stats.contains("memory_usage")) {
        memoryUsageValueLabel->setText(QString("%1 MB").arg(stats["memory_usage"].toInt()));
    }
    
    if (stats.contains("cpu_usage")) {
        int cpuUsage = stats["cpu_usage"].toInt();
        cpuUsageBar->setValue(cpuUsage);
        cpuUsageBar->setFormat(QString("%1%").arg(cpuUsage));
    }
    
    if (stats.contains("uptime")) {
        uptimeValueLabel->setText(stats["uptime"].toString());
    }
}

QVariantMap ServiceControlPanel::getCurrentConfig() const
{
    return currentConfig;
}

void ServiceControlPanel::showConfigDialog()
{
    onShowConfigDialog();
}

// ConfigDialog 实现
ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    createUI();
    setWindowTitle("服务配置");
    resize(500, 400);
}

void ConfigDialog::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    listenAddressEdit = new QLineEdit(this);
    listenAddressEdit->setText("127.0.0.1");
    
    listenPortSpin = new QSpinBox(this);
    listenPortSpin->setRange(1024, 65535);
    listenPortSpin->setValue(2333);
    
    maxConnectionsSpin = new QSpinBox(this);
    maxConnectionsSpin->setRange(1, 1000);
    maxConnectionsSpin->setValue(100);
    
    enableLoggingCheck = new QCheckBox("启用日志", this);
    enableLoggingCheck->setChecked(true);
    
    logLevelCombo = new QComboBox(this);
    logLevelCombo->addItems({"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"});
    logLevelCombo->setCurrentText("INFO");
    
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
    
    formLayout->addRow("监听地址:", listenAddressEdit);
    formLayout->addRow("监听端口:", listenPortSpin);
    formLayout->addRow("最大连接数:", maxConnectionsSpin);
    formLayout->addRow("", enableLoggingCheck);
    formLayout->addRow("日志级别:", logLevelCombo);
    formLayout->addRow("数据目录:", dataDirectoryEdit);
    formLayout->addRow("", validateChecksumCheck);
    formLayout->addRow("", enableMultipartCheck);
    formLayout->addRow("多部分超时:", multipartTimeoutSpin);
    formLayout->addRow("缓冲区大小:", bufferSizeSpin);
    formLayout->addRow("", autoReconnectCheck);
    formLayout->addRow("重连间隔:", reconnectIntervalSpin);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    setLayout(mainLayout);
}

QVariantMap ConfigDialog::getConfig() const
{
    QVariantMap config;
    config["listen_address"] = listenAddressEdit->text();
    config["listen_port"] = listenPortSpin->value();
    config["max_connections"] = maxConnectionsSpin->value();
    config["enable_logging"] = enableLoggingCheck->isChecked();
    config["log_level"] = logLevelCombo->currentText();
    config["data_directory"] = dataDirectoryEdit->text();
    config["validate_checksum"] = validateChecksumCheck->isChecked();
    config["enable_multipart"] = enableMultipartCheck->isChecked();
    config["multipart_timeout"] = multipartTimeoutSpin->value();
    config["buffer_size"] = bufferSizeSpin->value();
    config["auto_reconnect"] = autoReconnectCheck->isChecked();
    config["reconnect_interval"] = reconnectIntervalSpin->value();
    
    return config;
}

void ConfigDialog::setConfig(const QVariantMap &config)
{
    if (config.contains("listen_address")) {
        listenAddressEdit->setText(config["listen_address"].toString());
    }
    if (config.contains("listen_port")) {
        listenPortSpin->setValue(config["listen_port"].toInt());
    }
    if (config.contains("max_connections")) {
        maxConnectionsSpin->setValue(config["max_connections"].toInt());
    }
    if (config.contains("enable_logging")) {
        enableLoggingCheck->setChecked(config["enable_logging"].toBool());
    }
    if (config.contains("log_level")) {
        logLevelCombo->setCurrentText(config["log_level"].toString());
    }
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
    if (config.contains("buffer_size")) {
        bufferSizeSpin->setValue(config["buffer_size"].toInt());
    }
    if (config.contains("auto_reconnect")) {
        autoReconnectCheck->setChecked(config["auto_reconnect"].toBool());
    }
    if (config.contains("reconnect_interval")) {
        reconnectIntervalSpin->setValue(config["reconnect_interval"].toInt());
    }
}