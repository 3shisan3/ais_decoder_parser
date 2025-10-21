#include "processmanager.h"

#include <QDateTime>
#include <QTimer>
#if QT_VERSION_MAJOR < 6
#include <QRegExp>
using CurRegExp = QRegExp;
#else
#include <QRegularExpression>
using CurRegExp = QRegularExpression;
#endif

ProcessManager::ProcessManager(QObject *parent) 
    : QObject(parent)
{
    logInfo("进程管理器已初始化");
}

ProcessManager::~ProcessManager()
{
    logDebug("销毁进程管理器");
    emergencyStopAll();
}

// ==================== 日志辅助函数 ====================
void ProcessManager::logDebug(const QString &message, const QString &processName)
{
    emit logMessage(Debug, message, processName);
}

void ProcessManager::logInfo(const QString &message, const QString &processName)
{
    emit logMessage(Info, message, processName);
}

void ProcessManager::logWarning(const QString &message, const QString &processName)
{
    emit logMessage(Warning, message, processName);
}

void ProcessManager::logError(const QString &message, const QString &processName)
{
    emit logMessage(Error, message, processName);
}

void ProcessManager::logCritical(const QString &message, const QString &processName)
{
    emit logMessage(Critical, message, processName);
}

// ==================== 主要功能实现 ====================
bool ProcessManager::startAttachedProcess(const QString &processPath, 
                                        const QStringList &arguments,
                                        const QString &workingDir,
                                        const QString &processName)
{
    // 参数验证
    if (processPath.isEmpty()) {
        logError("进程路径不能为空");
        return false;
    }

    QString actualProcessName = processName;
    if (actualProcessName.isEmpty()) {
        actualProcessName = extractProcessNameFromPath(processPath);
        if (actualProcessName.isEmpty()) {
            logError(QString("无法从路径中提取进程名称: %1").arg(processPath));
            return false;
        }
        logDebug(QString("从路径提取进程名称: %1").arg(actualProcessName));
    }

    if (!validateProcessName(actualProcessName)) {
        logError(QString("进程名称无效: %1").arg(actualProcessName));
        return false;
    }

    // 检查是否已存在同名进程
    if (m_attachedProcesses.contains(actualProcessName)) {
        logWarning(QString("进程已被管理: %1").arg(actualProcessName), actualProcessName);
        return false;
    }

    // 检查可执行文件是否存在
    QFileInfo fileInfo(processPath);
    if (!fileInfo.exists() || !fileInfo.isExecutable()) {
        logError(QString("可执行文件不存在或不可执行: %1").arg(processPath));
        return false;
    }

    // 创建 QProcess 对象
    QProcess *process = new QProcess(this);
    ProcessInfo pInfo;
    pInfo.process = process;
    pInfo.fullPath = processPath;
    pInfo.arguments = arguments;
    pInfo.startTime = QDateTime::currentMSecsSinceEpoch();

    // 使用兼容Qt5和Qt6的信号槽连接方式
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5连接方式
    connect(process, SIGNAL(started()), this, SLOT(onQProcessStarted()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), 
            this, SLOT(onQProcessFinished(int, QProcess::ExitStatus)));
    connect(process, SIGNAL(errorOccurred(QProcess::ProcessError)), 
            this, SLOT(onQProcessErrorOccurred(QProcess::ProcessError)));
#else
    // Qt6连接方式
    connect(process, &QProcess::started, this, &ProcessManager::onQProcessStarted);
    connect(process, &QProcess::finished, this, &ProcessManager::onQProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &ProcessManager::onQProcessErrorOccurred);
#endif

    // 这些信号在Qt5和Qt6中保持相同
    connect(process, &QProcess::readyReadStandardOutput, 
            this, &ProcessManager::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, 
            this, &ProcessManager::onReadyReadStandardError);

    // 设置工作目录
    QString actualWorkingDir = workingDir;
    if (actualWorkingDir.isEmpty()) {
        actualWorkingDir = fileInfo.path();
    }
    process->setWorkingDirectory(actualWorkingDir);

    // 存储进程信息
    m_attachedProcesses[actualProcessName] = pInfo;

    logInfo(QString("启动进程 - 路径: %1, 参数: %2, 工作目录: %3")
           .arg(processPath)
           .arg(arguments.join(" "))
           .arg(actualWorkingDir), actualProcessName);

    // 启动进程
    process->start(processPath, arguments);

    // 等待进程启动（最多3秒）
    if (!process->waitForStarted(3000)) {
        logError(QString("进程启动失败: %1").arg(process->errorString()), actualProcessName);
        m_attachedProcesses.remove(actualProcessName);
        process->deleteLater();
        return false;
    }

    logInfo(QString("进程启动成功 (PID: %1)").arg(process->processId()), actualProcessName);
    return true;
}

bool ProcessManager::stopAttachedProcess(const QString &processName, int gracefulTimeoutMs)
{
    logInfo(QString("停止进程 (超时: %1ms)").arg(gracefulTimeoutMs), processName);

    if (!m_attachedProcesses.contains(processName)) {
        logWarning("进程未被本管理器管理，尝试系统级停止", processName);
        return forceStopProcessSystemWide(processName);
    }

    ProcessInfo &pInfo = m_attachedProcesses[processName];
    QProcess *process = pInfo.process;

    if (process->state() == QProcess::NotRunning) {
        logDebug("进程已处于停止状态", processName);
        m_attachedProcesses.remove(processName);
        process->deleteLater();
        return true;
    }

    // === 分级终止策略 ===

    // 阶段1: 优雅终止
    logDebug("阶段1: 优雅终止", processName);
    process->terminate();
    
    if (process->waitForFinished(gracefulTimeoutMs)) {
        logInfo("进程优雅终止成功", processName);
        m_attachedProcesses.remove(processName);
        process->deleteLater();
        return true;
    }

    // 阶段2: QProcess 强制终止
    logWarning("优雅终止失败，尝试强制终止", processName);
    process->kill();
    
    if (process->waitForFinished(3000)) {
        logInfo("进程强制终止成功", processName);
        m_attachedProcesses.remove(processName);
        process->deleteLater();
        return true;
    }

    // 阶段3: 系统级强制终止
    logError("QProcess方法失败，使用系统级强制停止", processName);
    
    // 清理 QProcess 资源
    if (process->state() != QProcess::NotRunning) {
        process->kill();
        process->waitForFinished(1000);
    }
    m_attachedProcesses.remove(processName);
    process->deleteLater();

    return forceStopProcessSystemWide(processName);
}

bool ProcessManager::forceStopProcessSystemWide(const QString &processName)
{
    logInfo("开始系统级强制停止", processName);
    
    bool success = false;
    
    // 方法1: 尝试通过 PID 文件停止
    std::string pidFile = ProcessUtils::getPidFile(processName.toStdString());
    int pidFromFile = ProcessUtils::readPidFile(pidFile);
    
    if (pidFromFile != -1) {
        logDebug(QString("从PID文件找到进程ID: %1").arg(pidFromFile), processName);
        if (ProcessUtils::stopProcessByPid(pidFromFile, false)) {
            logInfo(QString("通过PID文件停止成功: %1").arg(pidFromFile), processName);
            ProcessUtils::removePidFile(pidFile);
            success = true;
        }
    }
    
    // 方法2: 通过进程名优雅停止
    if (!success) {
        logDebug("尝试通过进程名优雅停止", processName);
        if (ProcessUtils::stopProcess(processName.toStdString(), false)) {
            logInfo("通过进程名优雅停止成功", processName);
            success = true;
        }
    }
    
    // 方法3: 强制停止
    if (!success) {
        logWarning("优雅停止失败，使用强制停止", processName);
        if (ProcessUtils::stopProcess(processName.toStdString(), true)) {
            logInfo("强制停止成功", processName);
            success = true;
        }
    }
    
    // 最终清理
    if (success) {
        ProcessUtils::removePidFile(pidFile);
        
        // 延迟验证进程是否确实停止
        QTimer::singleShot(2000, [this, processName]() {
            if (!ProcessUtils::isProcessRunning(processName.toStdString())) {
                logDebug("进程验证已停止", processName);
            } else {
                logWarning("进程可能仍在运行", processName);
            }
        });
    } else {
        logCritical("所有停止方法都失败", processName);
    }
    
    return success;
}

bool ProcessManager::isProcessRunning(const QString &processName)
{
    // 首先检查自己管理的进程
    if (m_attachedProcesses.contains(processName)) {
        QProcess::ProcessState state = m_attachedProcesses[processName].process->state();
        bool running = (state == QProcess::Running || state == QProcess::Starting);
        logDebug(QString("进程运行状态: %1").arg(running ? "运行中" : "未运行"), processName);
        return running;
    }
    
    // 系统级检查
    bool running = ProcessUtils::isProcessRunning(processName.toStdString());
    logDebug(QString("系统级检查进程状态: %1").arg(running ? "运行中" : "未运行"), processName);
    return running;
}

bool ProcessManager::stopProcess(const QString &processName, bool force)
{
    logInfo(QString("停止进程 (强制模式: %1)").arg(force ? "是" : "否"), processName);
    
    // 先尝试停止自己管理的进程
    if (m_attachedProcesses.contains(processName)) {
        return stopAttachedProcess(processName, force ? 1000 : 5000);
    }
    
    // 系统级停止
    bool success = ProcessUtils::stopProcess(processName.toStdString(), force);
    if (success) {
        logInfo("系统级停止成功", processName);
    } else {
        logError("系统级停止失败", processName);
    }
    return success;
}

int ProcessManager::getProcessId(const QString &processName)
{
    // 先查自己管理的进程
    if (m_attachedProcesses.contains(processName)) {
        qint64 pid = m_attachedProcesses[processName].process->processId();
        int result = (pid > 0) ? static_cast<int>(pid) : -1;
        logDebug(QString("获取进程ID: %1").arg(result), processName);
        return result;
    }
    
    // 系统级查找
    int pid = ProcessUtils::getProcessId(processName.toStdString());
    logDebug(QString("系统级获取进程ID: %1").arg(pid), processName);
    return pid;
}

QProcess* ProcessManager::getAttachedProcess(const QString &processName)
{
    QProcess* result = m_attachedProcesses.contains(processName) ? 
                      m_attachedProcesses[processName].process : nullptr;
    logDebug(QString("获取进程对象: %1").arg(result ? "成功" : "失败"), processName);
    return result;
}

void ProcessManager::emergencyStopAll()
{
    logWarning("开始紧急停止所有进程");
    
    QStringList processNames = m_attachedProcesses.keys();
    for (const QString &name : processNames) {
        logInfo(QString("紧急停止进程: %1").arg(name));
        stopAttachedProcess(name, 1000);
    }
    
    logInfo("紧急停止完成");
}

void ProcessManager::onReadyReadStandardOutput()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;
    
    QString processName;
    for (auto it = m_attachedProcesses.begin(); it != m_attachedProcesses.end(); ++it) {
        if (it.value().process == process) {
            processName = it.key();
            break;
        }
    }
    
    if (!processName.isEmpty()) {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        emit processStandardOutput(processName, output);
    }
}

void ProcessManager::onReadyReadStandardError()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;
    
    QString processName;
    for (auto it = m_attachedProcesses.begin(); it != m_attachedProcesses.end(); ++it) {
        if (it.value().process == process) {
            processName = it.key();
            break;
        }
    }
    
    if (!processName.isEmpty()) {
        QString errorOutput = QString::fromLocal8Bit(process->readAllStandardError());
        emit processErrorOutput(processName, errorOutput);
        logWarning(QString("进程错误输出: %1").arg(errorOutput.trimmed()), processName);
    }
}

void ProcessManager::onQProcessStarted()
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;
    
    QString processName;
    for (auto it = m_attachedProcesses.begin(); it != m_attachedProcesses.end(); ++it) {
        if (it.value().process == process) {
            processName = it.key();
            break;
        }
    }
    
    if (!processName.isEmpty()) {
        logDebug("收到进程启动信号", processName);
        emit processStarted(processName);
    }
}

void ProcessManager::onQProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;
    
    QString processName;
    for (auto it = m_attachedProcesses.begin(); it != m_attachedProcesses.end(); ++it) {
        if (it.value().process == process) {
            processName = it.key();
            m_attachedProcesses.remove(processName);
            break;
        }
    }
    
    if (!processName.isEmpty()) {
        logInfo(QString("进程结束 - 退出码: %1, 状态: %2")
               .arg(exitCode)
               .arg(exitStatus == QProcess::NormalExit ? "正常" : "崩溃"), processName);
        emit processFinished(processName, exitCode, exitStatus);
        process->deleteLater();
    }
}

void ProcessManager::onQProcessErrorOccurred(QProcess::ProcessError error)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;
    
    QString processName;
    for (auto it = m_attachedProcesses.begin(); it != m_attachedProcesses.end(); ++it) {
        if (it.value().process == process) {
            processName = it.key();
            break;
        }
    }
    
    if (!processName.isEmpty()) {
        QString errorStr;
        switch (error) {
        case QProcess::FailedToStart: errorStr = "启动失败"; break;
        case QProcess::Crashed: errorStr = "进程崩溃"; break;
        case QProcess::Timedout: errorStr = "操作超时"; break;
        case QProcess::WriteError: errorStr = "写入错误"; break;
        case QProcess::ReadError: errorStr = "读取错误"; break;
        default: errorStr = "未知错误"; break;
        }
        
        logError(QString("进程错误: %1 - %2").arg(errorStr).arg(process->errorString()), processName);
        emit processErrorOccurred(processName, error);
    }
}

QString ProcessManager::extractProcessNameFromPath(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.baseName();
}

bool ProcessManager::validateProcessName(const QString &processName)
{
    if (processName.isEmpty()) return false;
    
    // 检查非法字符
    CurRegExp invalidChars("[\\\\/:*?\"<>|]");
    return !processName.contains(invalidChars);
}