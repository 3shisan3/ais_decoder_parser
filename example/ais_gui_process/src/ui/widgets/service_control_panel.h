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
#include <QTabWidget>

#include "core/processmanager.h"
#include "core/ipc_client_manager.h"

/**
 * @brief 服务控制面板 - 集成进程管理和IPC通信
 * 
 * 提供完整的服务生命周期管理：
 * - 通过ProcessManager管理服务进程的启动/停止
 * - 通过IPCClientManager与服务进程通信
 * - 实时监控服务状态和统计信息
 * - 动态配置服务参数
 */
class ServiceControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceControlPanel(QWidget *parent = nullptr);
    ~ServiceControlPanel();

    // 设置服务可执行文件路径
    void setServiceExecutable(const QString &executablePath);
    
    // 设置服务工作目录
    void setServiceWorkingDir(const QString &workingDir);
    
    // 设置服务启动参数
    void setServiceArguments(const QStringList &arguments);
    
    // 获取当前配置
    QVariantMap getCurrentConfig() const;

public slots:
    // 更新服务运行状态
    void updateServiceStatus(bool running);
    
    // 更新服务配置
    void updateServiceConfig(const QVariantMap &config);
    
    // 更新服务统计信息
    void updateServiceStats(const QVariantMap &stats);
    
    // 显示配置对话框
    void showConfigDialog();

signals:
    // 服务控制信号
    void startServiceRequested();
    void stopServiceRequested();
    void restartServiceRequested();
    void configChanged(const QVariantMap &config);

private slots:
    // 服务控制按钮槽函数
    void onStartService();
    void onStopService();
    void onRestartService();
    
    // 配置管理槽函数
    void onApplyConfig();
    void onSaveConfig();
    void onLoadConfig();
    void onShowConfigDialog();
    
    // 状态刷新槽函数
    void onRefreshStatus();
    
    // IPC连接状态处理
    void onIPCConnected(bool connected);
    void onIPCError(const QString &error);

private:
    void createUI();
    void createStatusGroup();
    void createControlGroup();
    void createConfigGroup();
    void createStatsGroup();
    void createProcessGroup();
    
    // 配置管理
    QVariantMap collectConfig() const;
    void loadConfigFromFile(const QString &filename);
    void saveConfigToFile(const QString &filename);
    void loadDefaultConfig();  // 添加缺失的方法声明
    
    // 进程管理
    bool startServiceProcess();
    bool stopServiceProcess();
    bool isServiceProcessRunning();
    
    // 状态更新
    void updateConnectionStatus(bool connected);
    void updateProcessStatus(bool running);
    void updateStatsDisplay(const QVariantMap &stats);
    
    // UI组件 - 状态组
    QGroupBox *statusGroup;
    QLabel *statusLabel;
    QLabel *statusValueLabel;
    QLabel *connectionLabel;
    QLabel *connectionValueLabel;
    QLabel *uptimeLabel;
    QLabel *uptimeValueLabel;
    QLabel *processLabel;
    QLabel *processValueLabel;
    QPushButton *refreshButton;
    
    // UI组件 - 控制组
    QGroupBox *controlGroup;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *restartButton;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    
    // UI组件 - 进程管理组
    QGroupBox *processGroup;
    QLabel *executableLabel;
    QLineEdit *executableEdit;
    QLabel *workingDirLabel;
    QLineEdit *workingDirEdit;
    QLabel *argumentsLabel;
    QLineEdit *argumentsEdit;
    QPushButton *browseExecutableButton;
    QPushButton *browseWorkingDirButton;
    
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
    QLabel *shipCountLabel;
    QLabel *shipCountValueLabel;
    
    // 管理器实例
    ProcessManager *processManager;
    IPCClientManager *ipcClientManager;
    
    // 状态变量
    bool serviceRunning;
    bool ipcConnected;
    QTimer *statusTimer;
    QVariantMap currentConfig;
    QVariantMap currentStats;
    
    // 服务进程配置
    QString serviceExecutable;
    QString serviceWorkingDir;
    QStringList serviceArguments;
    QString serviceProcessName;
};

/**
 * @brief 服务配置对话框
 * 
 * 提供图形化界面配置AIS服务参数
 */
class ServiceConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ServiceConfigDialog(QWidget *parent = nullptr);
    
    // 获取/设置配置
    QVariantMap getConfig() const;
    void setConfig(const QVariantMap &config);

private:
    void createUI();
    
    // 网络配置
    QLineEdit *listenAddressEdit;
    QSpinBox *listenPortSpin;
    QSpinBox *serverPortSpin;
    QSpinBox *maxConnectionsSpin;
    
    // 日志配置
    QCheckBox *enableLoggingCheck;
    QComboBox *logLevelCombo;
    QLineEdit *logDirectoryEdit;
    
    // 数据配置
    QLineEdit *dataDirectoryEdit;
    QCheckBox *validateChecksumCheck;
    QCheckBox *enableMultipartCheck;
    QSpinBox *multipartTimeoutSpin;
    
    // 性能配置
    QSpinBox *bufferSizeSpin;
    QCheckBox *autoReconnectCheck;
    QSpinBox *reconnectIntervalSpin;
    QSpinBox *heartbeatIntervalSpin;
};

#endif // SERVICE_CONTROL_PANEL_H