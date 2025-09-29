#include "ipc_client_manager.h"

#include <QDateTime>
#include <QThread>
#include <QMessageBox>

IPCClientManager::IPCClientManager(QObject *parent)
    : QObject(parent)
    , ipcClient(nullptr)
    , parserManager(new AISParserManager(this))
    , m_serverAddress("127.0.0.1")
    , m_serverPort(2333)
    , m_connected(false)
    , m_autoReconnect(true)
    , reconnectAttempts(0)
{
    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(5000); // 5秒重连间隔
    reconnectTimer->setSingleShot(true);
    connect(reconnectTimer, &QTimer::timeout, this, &IPCClientManager::onReconnectTimeout);
    
    initializeClient();
}

IPCClientManager::~IPCClientManager()
{
    disconnectFromServer();
    if (ipcClient) {
        delete ipcClient;
    }
}

void IPCClientManager::initializeClient()
{
    if (ipcClient) {
        disconnectFromServer();
        delete ipcClient;
    }
    
    ipcClient = new ais::AISClient();
    
    // 设置消息处理器
    ipcClient->setMessageHandler([this](const ais::protocol::CommandMessage &cmd) {
        QString message = QString::fromStdString(cmd.toJson());
        QVariantMap parsed = parseMessage(message);
        
        // 处理不同类型的消息
        if (parsed.contains("type")) {
            QString msgType = parsed["type"].toString();
            if (msgType == "status_update") {
                bool running = parsed["status"].toString() == "running";
                emit serviceStateChanged(running);
            } else if (msgType == "log_message") {
                QString logMsg = parsed["message"].toString();
                emit messageReceived(logMsg);
            }
        }
        
        emit messageReceived(QString("收到消息: %1").arg(message));
    });
    
    // 设置响应处理器
    ipcClient->setResponseHandler([this](const ais::protocol::ResponseMessage &response) {
        QVariantMap result = parseMessage(QString::fromStdString(response.toJson()));
        
        if (result.contains("status")) {
            emit serviceStateChanged(result["status"].toString() == "running");
        }
        
        if (result.contains("config")) {
            emit serviceConfigReceived(result["config"].toMap());
        }
        
        if (result.contains("error")) {
            emit errorOccurred(result["error"].toString());
        }
    });
    
    // 设置错误处理器
    ipcClient->setErrorHandler([this](const std::string &error) {
        QString errorMsg = QString::fromStdString(error);
        emit errorOccurred(errorMsg);
        
        // 自动重连逻辑
        if (m_autoReconnect && m_connected) {
            startReconnectTimer();
        }
    });
}

bool IPCClientManager::connectToServer()
{
    return connectToServer(m_serverAddress, m_serverPort);
}

bool IPCClientManager::connectToServer(const QString &address, quint16 port)
{
    if (isConnected()) {
        disconnectFromServer();
        QThread::msleep(100); // 短暂延迟
    }
    
    stopReconnectTimer();
    
    try {
        bool success = ipcClient->connectToServer(address.toStdString(), port);
        if (success) {
            m_connected = true;
            m_serverAddress = address;
            m_serverPort = port;
            reconnectAttempts = 0;
            emit connectionStateChanged(true);
            return true;
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("连接失败: %1").arg(e.what()));
    }
    
    // 连接失败，启动重连定时器
    if (m_autoReconnect) {
        startReconnectTimer();
    }
    
    return false;
}

void IPCClientManager::disconnectFromServer()
{
    stopReconnectTimer();
    
    if (ipcClient && isConnected()) {
        ipcClient->disconnect();
        m_connected = false;
        emit connectionStateChanged(false);
    }
}

bool IPCClientManager::isConnected() const
{
    return m_connected && ipcClient && ipcClient->isConnected();
}

void IPCClientManager::setServerAddress(const QString &address)
{
    m_serverAddress = address;
}

void IPCClientManager::setServerPort(quint16 port)
{
    m_serverPort = port;
}

QString IPCClientManager::serverAddress() const
{
    return m_serverAddress;
}

quint16 IPCClientManager::serverPort() const
{
    return m_serverPort;
}

void IPCClientManager::sendStartCommand()
{
    QVariantMap command;
    command["type"] = "control";
    command["action"] = "start";
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

void IPCClientManager::sendStopCommand()
{
    QVariantMap command;
    command["type"] = "control";
    command["action"] = "stop";
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

void IPCClientManager::getServiceStatus()
{
    QVariantMap command;
    command["type"] = "query";
    command["action"] = "status";
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

void IPCClientManager::getServiceConfig()
{
    QVariantMap command;
    command["type"] = "query";
    command["action"] = "config";
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

void IPCClientManager::updateServiceConfig(const QVariantMap &config)
{
    QVariantMap command;
    command["type"] = "control";
    command["action"] = "update_config";
    command["config"] = config;
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

void IPCClientManager::sendCustomCommand(const QString &commandType, const QVariantMap &params)
{
    QVariantMap command;
    command["type"] = "custom";
    command["action"] = commandType;
    command["params"] = params;
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendCommand(command);
}

AISParserManager* IPCClientManager::getParserManager() const
{
    return parserManager;
}

void IPCClientManager::sendCommand(const QVariantMap &command)
{
    if (!isConnected()) {
        emit errorOccurred("未连接到服务器");
        return;
    }
    
    try {
        QJsonDocument doc(QJsonObject::fromVariantMap(command));
        QString jsonStr = doc.toJson(QJsonDocument::Compact);
        ipcClient->sendCommand(jsonStr.toStdString());
    } catch (const std::exception &e) {
        emit errorOccurred(QString("发送命令失败: %1").arg(e.what()));
    }
}

QVariantMap IPCClientManager::parseMessage(const QString &message)
{
    QVariantMap result;
    
    try {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
            result = doc.object().toVariantMap();
        }
    } catch (...) {
        // 解析失败，返回原始消息
        result["raw"] = message;
    }
    
    return result;
}

void IPCClientManager::startReconnectTimer()
{
    reconnectAttempts++;
    if (reconnectAttempts <= 10) { // 最多重试10次
        reconnectTimer->start();
        emit messageReceived(QString("尝试重新连接... (%1/10)").arg(reconnectAttempts));
    } else {
        emit errorOccurred("重连次数过多，停止自动重连");
    }
}

void IPCClientManager::stopReconnectTimer()
{
    reconnectTimer->stop();
    reconnectAttempts = 0;
}

void IPCClientManager::onReconnectTimeout()
{
    if (m_autoReconnect) {
        connectToServer();
    }
}

void IPCClientManager::onClientConnected()
{
    m_connected = true;
    emit connectionStateChanged(true);
    // 连接成功时认为服务在运行
    emit serviceStateChanged(true);

    stopReconnectTimer();
}

void IPCClientManager::onClientDisconnected()
{
    m_connected = false;
    emit connectionStateChanged(false);

    // 断开连接时服务停止
    emit serviceStateChanged(false);
    
    if (m_autoReconnect) {
        startReconnectTimer();
    }
}

void IPCClientManager::onClientError(const QString &errorMessage)
{
    emit errorOccurred(errorMessage);
    
    if (m_autoReconnect && m_connected) {
        startReconnectTimer();
    }
}

void IPCClientManager::onClientMessageReceived(const QString &message)
{
    QVariantMap parsed = parseMessage(message);

    // 只要收到任何消息且连接正常，就认为服务在运行中
    if (m_connected) {
        emit serviceStateChanged(true);
    }
    
    if (parsed.contains("type")) {
        QString msgType = parsed["type"].toString();
        if (msgType == "status_update") {
            bool running = parsed["status"].toString() == "running";
            emit serviceStateChanged(running);
        } else if (msgType == "config_update") {
            emit serviceConfigReceived(parsed["config"].toMap());
        }
    }
    
    emit messageReceived(message);
}