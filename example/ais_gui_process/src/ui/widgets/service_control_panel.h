#ifndef SERVICE_CONTROL_PANEL_H
#define SERVICE_CONTROL_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>

class ServiceControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceControlPanel(QWidget *parent = nullptr);
    
    void updateServiceStatus(bool running);
    void updateServiceConfig(const QVariantMap &config);
    void showConfigDialog();
    QVariantMap getCurrentConfig() const;

signals:
    void startServiceRequested();
    void stopServiceRequested();
    void configChanged(const QVariantMap &config);
    void restartServiceRequested();

private slots:
    void onStartService();
    void onStopService();
    void onRestartService();
    void onApplyConfig();
    void onSaveConfig();
    void onLoadConfig();
    void onShowConfigDialog();
    void onRefreshStatus();
    void onServiceStatsUpdated(const QVariantMap &stats);

private:
    void createUI();
    void createStatusGroup();
    void createControlGroup();
    void createConfigGroup();
    void createStatsGroup();
    QVariantMap collectConfig() const;
    void loadConfigFromFile(const QString &filename);
    void saveConfigToFile(const QString &filename);
    void updateStatsDisplay(const QVariantMap &stats);
    
    // UI组件 - 状态组
    QGroupBox *statusGroup;
    QLabel *statusLabel;
    QLabel *statusValueLabel;
    QLabel *uptimeLabel;
    QLabel *uptimeValueLabel;
    QLabel *versionLabel;
    QLabel *versionValueLabel;
    QPushButton *refreshButton;
    
    // UI组件 - 控制组
    QGroupBox *controlGroup;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *restartButton;
    
    // UI组件 - 配置组
    QGroupBox *configGroup;
    QLabel *currentConfigLabel;
    QTextEdit *configTextEdit;
    QPushButton *showConfigDialogButton;
    QPushButton *applyConfigButton;
    QPushButton *saveConfigButton;
    QPushButton *loadConfigButton;
    
    // UI组件 - 统计信息组
    QGroupBox *statsGroup;
    QLabel *messagesReceivedLabel;
    QLabel *messagesReceivedValueLabel;
    QLabel *messagesParsedLabel;
    QLabel *messagesParsedValueLabel;
    QLabel *connectionsLabel;
    QLabel *connectionsValueLabel;
    QLabel *memoryUsageLabel;
    QLabel *memoryUsageValueLabel;
    QProgressBar *cpuUsageBar;
    QLabel *cpuUsageLabel;
    
    // 配置对话框
    QDialog *configDialog;
    QLineEdit *listenAddressEdit;
    QSpinBox *listenPortSpin;
    QSpinBox *maxConnectionsSpin;
    QCheckBox *enableLoggingCheck;
    QComboBox *logLevelCombo;
    QLineEdit *dataDirectoryEdit;
    QCheckBox *validateChecksumCheck;
    QCheckBox *enableMultipartCheck;
    QSpinBox *multipartTimeoutSpin;
    QSpinBox *bufferSizeSpin;
    QCheckBox *autoReconnectCheck;
    QSpinBox *reconnectIntervalSpin;
    
    // 状态变量
    bool serviceRunning;
    QTimer *statusTimer;
    QVariantMap currentConfig;
    QVariantMap currentStats;
};

// 配置对话框类
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    QVariantMap getConfig() const;
    void setConfig(const QVariantMap &config);

private:
    void createUI();
    
    QLineEdit *listenAddressEdit;
    QSpinBox *listenPortSpin;
    QSpinBox *maxConnectionsSpin;
    QCheckBox *enableLoggingCheck;
    QComboBox *logLevelCombo;
    QLineEdit *dataDirectoryEdit;
    QCheckBox *validateChecksumCheck;
    QCheckBox *enableMultipartCheck;
    QSpinBox *multipartTimeoutSpin;
    QSpinBox *bufferSizeSpin;
    QCheckBox *autoReconnectCheck;
    QSpinBox *reconnectIntervalSpin;
};

#endif // SERVICE_CONTROL_PANEL_H