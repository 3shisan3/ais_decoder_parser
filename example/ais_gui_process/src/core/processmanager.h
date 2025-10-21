#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include <QTimer>
#include <QFileInfo>

#include "utils/process_utils.h"

/**
 * @brief 进程管理整合类 - 结合 QProcess 和 ProcessUtils 的优势
 *
 * 所有日志信息都通过信号发出，方便界面显示或文件记录
 */
class ProcessManager : public QObject
{
    Q_OBJECT

public:
    // 日志级别枚举
    enum LogLevel
    {
        Debug,   // 调试信息
        Info,    // 一般信息
        Warning, // 警告信息
        Error,   // 错误信息
        Critical // 严重错误
    };
    Q_ENUM(LogLevel) // 让枚举在Qt的元对象系统中可用

    explicit ProcessManager(QObject *parent = nullptr);
    ~ProcessManager();

    /**
     * @brief 启动一个需要交互的进程（使用 QProcess）
     * @param processPath 进程路径
     * @param arguments 启动参数
     * @param workingDir 工作目录
     * @param processName 进程名称
     * @return 成功返回true，失败返回false
     */
    bool startAttachedProcess(const QString &processPath,
                              const QStringList &arguments = QStringList(),
                              const QString &workingDir = QString(),
                              const QString &processName = QString());

    /**
     * @brief 停止由本管理器启动的进程（分级终止策略）
     * @param processName 进程名称
     * @param gracefulTimeoutMs 优雅终止超时时间
     * @return 成功返回true，失败返回false
     */
    bool stopAttachedProcess(const QString &processName, int gracefulTimeoutMs = 5000);

    /**
     * @brief 系统级进程检查（使用 ProcessUtils）
     * @param processName 进程名称
     * @return 正在运行返回true，否则返回false
     */
    bool isProcessRunning(const QString &processName);

    /**
     * @brief 系统级进程停止（使用 ProcessUtils）
     * @param processName 进程名称
     * @param force 是否强制终止
     * @return 成功返回true，失败返回false
     */
    bool stopProcess(const QString &processName, bool force = false);

    /**
     * @brief 获取进程ID
     * @param processName 进程名称
     * @return 进程ID，未找到返回-1
     */
    int getProcessId(const QString &processName);

    /**
     * @brief 获取管理的 QProcess 对象指针
     * @param processName 进程名称
     * @return QProcess指针，未找到返回nullptr
     */
    QProcess *getAttachedProcess(const QString &processName);

    /**
     * @brief 紧急停止所有管理的进程
     */
    void emergencyStopAll();

signals:
    /**
     * @brief 统一的日志输出信号
     * @param level 日志级别
     * @param message 日志消息
     * @param processName 相关的进程名称（可为空）
     */
    void logMessage(ProcessManager::LogLevel level, const QString &message, const QString &processName);

    /**
     * @brief 进程启动成功信号
     * @param processName 进程名称
     */
    void processStarted(const QString &processName);

    /**
     * @brief 进程结束信号
     * @param processName 进程名称
     * @param exitCode 退出码
     * @param exitStatus 退出状态
     */
    void processFinished(const QString &processName, int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief 进程错误信号
     * @param processName 进程名称
     * @param error 错误类型
     */
    void processErrorOccurred(const QString &processName, QProcess::ProcessError error);

    /**
     * @brief 进程标准输出信号
     * @param processName 进程名称
     * @param output 输出内容
     */
    void processStandardOutput(const QString &processName, const QString &output);

    /**
     * @brief 进程错误输出信号
     * @param processName 进程名称
     * @param output 错误输出内容
     */
    void processErrorOutput(const QString &processName, const QString &output);

private slots:
    // QProcess信号槽
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onQProcessStarted();
    void onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onQProcessErrorOccurred(QProcess::ProcessError error);

private:
    // 日志辅助函数
    void logDebug(const QString &message, const QString &processName = QString());
    void logInfo(const QString &message, const QString &processName = QString());
    void logWarning(const QString &message, const QString &processName = QString());
    void logError(const QString &message, const QString &processName = QString());
    void logCritical(const QString &message, const QString &processName = QString());

    // 进程管理辅助函数
    bool forceStopProcessSystemWide(const QString &processName);
    QString extractProcessNameFromPath(const QString &filePath);
    bool validateProcessName(const QString &processName);

    // 进程信息结构
    struct ProcessInfo
    {
        QProcess *process;
        QString fullPath;
        qint64 startTime;
        QStringList arguments;

        ProcessInfo() : process(nullptr), startTime(0) {}
    };

    QMap<QString, ProcessInfo> m_attachedProcesses; // 管理的进程映射表
};

#endif // PROCESSMANAGER_H