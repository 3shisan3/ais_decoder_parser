#include "config_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>

ConfigPanel::ConfigPanel(IPCClientManager *ipcManager, QWidget *parent)
    : QWidget(parent)
    , ipcManager(ipcManager)
{
    createUI();
    connectSignals();
    setUIEnabled(false); // 初始状态禁用
}

void ConfigPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *loadButton = new QPushButton("加载配置", this);
    QPushButton *saveButton = new QPushButton("保存配置", this);
    QPushButton *applyButton = new QPushButton("应用配置", this);
    
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addStretch();
    
    // 日志配置组
    QGroupBox *loggerGroup = new QGroupBox("日志配置", this);
    QFormLayout *loggerLayout = new QFormLayout(loggerGroup);
    
    enableLoggingCheck = new QCheckBox("启用日志", this);
    logFileEdit = new QLineEdit(this);
    QPushButton *browseLogButton = new QPushButton("浏览...", this);
    
    QHBoxLayout *logFileLayout = new QHBoxLayout();
    logFileLayout->addWidget(logFileEdit);
    logFileLayout->addWidget(browseLogButton);
    
    loggerLayout->addRow("启用日志:", enableLoggingCheck);
    loggerLayout->addRow("日志文件:", logFileLayout);
    
    // 解析器配置组
    QGroupBox *parserGroup = new QGroupBox("解析器配置", this);
    QFormLayout *parserLayout = new QFormLayout(parserGroup);
    
    QCheckBox *validateChecksumCheck = new QCheckBox("验证校验和", this);
    QCheckBox *enableMultipartCheck = new QCheckBox("启用多部分重组", this);
    QSpinBox *maxMultipartAgeSpin = new QSpinBox(this);
    maxMultipartAgeSpin->setRange(1, 3600);
    maxMultipartAgeSpin->setSuffix("秒");
    
    parserLayout->addRow("验证校验和:", validateChecksumCheck);
    parserLayout->addRow("多部分重组:", enableMultipartCheck);
    parserLayout->addRow("最大保留时间:", maxMultipartAgeSpin);
    
    // 存储配置组
    QGroupBox *storageGroup = new QGroupBox("存储配置", this);
    QFormLayout *storageLayout = new QFormLayout(storageGroup);
    
    QCheckBox *saveSwitchCheck = new QCheckBox("启用本地存储", this);
    QComboBox *storageTypeCombo = new QComboBox(this);
    storageTypeCombo->addItems({"NONE", "DATABASE", "CSV", "MEMORY"});
    QLineEdit *storagePathEdit = new QLineEdit(this);
    QPushButton *browseStorageButton = new QPushButton("浏览...", this);
    
    QHBoxLayout *storagePathLayout = new QHBoxLayout();
    storagePathLayout->addWidget(storagePathEdit);
    storagePathLayout->addWidget(browseStorageButton);
    
    storageLayout->addRow("启用存储:", saveSwitchCheck);
    storageLayout->addRow("存储类型:", storageTypeCombo);
    storageLayout->addRow("存储路径:", storagePathLayout);
    
    // 通信配置组
    QGroupBox *commGroup = new QGroupBox("通信配置", this);
    QFormLayout *commLayout = new QFormLayout(commGroup);
    
    QSpinBox *subPortSpin = new QSpinBox(this);
    subPortSpin->setRange(1, 65535);
    QLineEdit *sendIPEdit = new QLineEdit(this);
    QSpinBox *sendPortSpin = new QSpinBox(this);
    sendPortSpin->setRange(1, 65535);
    QSpinBox *msgSaveSizeSpin = new QSpinBox(this);
    msgSaveSizeSpin->setRange(-1, 1000000);
    msgSaveSizeSpin->setSpecialValueText("无限制");
    QSpinBox *msgSaveTimeSpin = new QSpinBox(this);
    msgSaveTimeSpin->setRange(-1, 86400);
    msgSaveTimeSpin->setSpecialValueText("永久");
    msgSaveTimeSpin->setSuffix("秒");
    
    commLayout->addRow("监听端口:", subPortSpin);
    commLayout->addRow("目标IP:", sendIPEdit);
    commLayout->addRow("目标端口:", sendPortSpin);
    commLayout->addRow("消息保存数量:", msgSaveSizeSpin);
    commLayout->addRow("消息保存时间:", msgSaveTimeSpin);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(loggerGroup);
    mainLayout->addWidget(parserGroup);
    mainLayout->addWidget(storageGroup);
    mainLayout->addWidget(commGroup);
    mainLayout->addStretch();
    
    setLayout(mainLayout);
    
    // 连接信号
    connect(loadButton, &QPushButton::clicked, this, &ConfigPanel::onLoadConfig);
    connect(saveButton, &QPushButton::clicked, this, &ConfigPanel::onSaveConfig);
    connect(applyButton, &QPushButton::clicked, this, [this]() {
        ipcManager->updateConfig(getConfigFromUI());
    });
    
    connect(browseLogButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "选择日志文件", 
                                                      logFileEdit->text(),
                                                      "日志文件 (*.log);;所有文件 (*)");
        if (!fileName.isEmpty()) {
            logFileEdit->setText(fileName);
        }
    });
    
    connect(browseStorageButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "选择存储文件", 
                                                      storagePathEdit->text(),
                                                      "CSV文件 (*.csv);;所有文件 (*)");
        if (!fileName.isEmpty()) {
            storagePathEdit->setText(fileName);
        }
    });
}

void ConfigPanel::connectSignals()
{
    connect(ipcManager, &IPCClientManager::configUpdated, 
            this, &ConfigPanel::onConfigUpdated);
    connect(ipcManager, &IPCClientManager::connected, 
            this, [this]() { setUIEnabled(true); });
    connect(ipcManager, &IPCClientManager::disconnected, 
            this, [this]() { setUIEnabled(false); });
}

void ConfigPanel::onLoadConfig()
{
    if (!ipcManager->isConnected()) {
        QMessageBox::warning(this, "警告", "请先连接到AIS服务");
        return;
    }
    // 这里应该发送获取配置的命令
    // 简化实现：假设服务端会主动推送配置
}

void ConfigPanel::onSaveConfig()
{
    QJsonObject config = getConfigFromUI();
    QJsonDocument doc(config);
    
    QString fileName = QFileDialog::getSaveFileName(this, "保存配置", 
                                                   "ais_config.json",
                                                   "JSON文件 (*.json);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(doc.toJson());
            file.close();
        }
    }
}

void ConfigPanel::onConfigUpdated(const QJsonObject &config)
{
    loadUIFromConfig(config);
}

void ConfigPanel::loadUIFromConfig(const QJsonObject &config)
{
    // 这里需要根据实际的配置结构解析JSON并更新UI
    // 简化实现
    QJsonObject logger = config["logger"].toObject();
    enableLoggingCheck->setChecked(logger["enableLogging"].toBool());
    logFileEdit->setText(logger["logFile"].toString());
    
    // 其他配置项的加载...
}

QJsonObject ConfigPanel::getConfigFromUI()
{
    QJsonObject config;
    
    QJsonObject logger;
    logger["enableLogging"] = enableLoggingCheck->isChecked();
    logger["logFile"] = logFileEdit->text();
    config["logger"] = logger;
    
    // 其他配置项的获取...
    
    return config;
}

void ConfigPanel::setUIEnabled(bool enabled)
{
    // 设置所有控件的启用状态
    QList<QWidget*> widgets = findChildren<QWidget*>();
    for (QWidget *widget : widgets) {
        if (widget->objectName().isEmpty()) { // 避免禁用布局控件
            widget->setEnabled(enabled);
        }
    }
}