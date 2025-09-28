#ifndef CONFIG_PANEL_H
#define CONFIG_PANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

#include "ipc_client_manager.h"

class ConfigPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigPanel(IPCClientManager *ipcManager, QWidget *parent = nullptr);

private slots:
    void onLoadConfig();
    void onSaveConfig();
    void onConfigUpdated(const QJsonObject &config);
    void onConnectionStateChanged(bool connected);

private:
    void createUI();
    void loadUIFromConfig(const QJsonObject &config);
    QJsonObject getConfigFromUI();
    void setUIEnabled(bool enabled);

    IPCClientManager *ipcManager;
    
    // 日志配置
    QCheckBox *enableLoggingCheck;
    QLineEdit *logFileEdit;