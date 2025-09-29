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
    
    // 设置消息处理器 - 基于CommandType枚举进行处理
    ipcClient->setMessageHandler([this](const ais::protocol::CommandMessage &cmd) {
        try {
            // 记录接收到的消息基本信息
            QString msgInfo = QString("收到消息 - 类型: %1, 序列号: %2")
                            .arg(static_cast<int>(cmd.type))
                            .arg(cmd.sequence);
            emit messageReceived(msgInfo);
            
            // 根据CommandType进行不同的处理
            switch (cmd.type) {
            case ais::protocol::CommandType::GET_STATUS:
                // 状态查询消息，通常不需要特殊处理
                emit messageReceived("收到状态查询消息");
                break;
                
            case ais::protocol::CommandType::START_SERVICE:
                // 服务启动消息
                emit messageReceived("收到服务启动消息");
                break;
                
            case ais::protocol::CommandType::STOP_SERVICE:
                // 服务停止消息
                emit messageReceived("收到服务停止消息");
                break;
                
            case ais::protocol::CommandType::GET_SHIP_COUNT:
                // 获取船舶数量消息
                emit messageReceived("收到获取船舶数量消息");
                break;
                
            case ais::protocol::CommandType::SEND_MESSAGE:
                // AIS数据消息 - 这是最重要的消息类型
                handleAISMessage(cmd);
                break;
                
            case ais::protocol::CommandType::CONFIG_UPDATE:
                // 配置更新消息
                handleConfigUpdate(cmd);
                break;
                
            case ais::protocol::CommandType::GET_MESSAGE_STATS:
                // 消息统计查询
                emit messageReceived("收到消息统计查询");
                break;
                
            default:
                // 未知消息类型
                emit messageReceived(QString("收到未知类型消息: %1").arg(static_cast<int>(cmd.type)));
                break;
            }
            
            // 无论什么类型的消息，都发送原始数据用于显示
            emit rawMessageReceived(QString::fromStdString(cmd.data));
            
        } catch (const std::exception &e) {
            QString errorMsg = QString("消息处理异常: %1").arg(e.what());
            emit errorOccurred(errorMsg);
        }
    });
    
    // 设置响应处理器
    ipcClient->setResponseHandler([this](const ais::protocol::ResponseMessage &response) {
        try {
            // 记录响应消息
            QString responseMsg = QString("收到响应 - 状态: %1, 序列号: %2")
                                 .arg(static_cast<int>(response.status))
                                 .arg(response.sequence);
            emit messageReceived(responseMsg);
            
            // 处理响应状态
            if (response.status == ais::protocol::ResponseStatus::SUCCESS) {
                // 成功响应，尝试解析数据
                if (!response.data.empty()) {
                    QVariantMap result = parseMessage(QString::fromStdString(response.data));
                    if (result.contains("status")) {
                        emit serviceStateChanged(result["status"].toBool());
                    }
                    if (result.contains("config")) {
                        emit serviceConfigReceived(result["config"].toMap());
                    }
                    if (result.contains("ship_count")) {
                        emit messageReceived(QString("船舶数量: %1").arg(result["ship_count"].toInt()));
                    }
                }
            } else {
                // 错误响应
                QString errorInfo = QString::fromStdString(response.data);
                emit errorOccurred(QString("服务响应错误: %1").arg(errorInfo));
            }
            
            // 发送原始响应数据
            emit rawMessageReceived(QString::fromStdString(response.data));
            
        } catch (const std::exception &e) {
            emit errorOccurred(QString("响应处理异常: %1").arg(e.what()));
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

// 处理AIS消息
void IPCClientManager::handleAISMessage(const ais::protocol::CommandMessage &cmd)
{
    try {
        // 尝试解析AIS消息数据
        QVariantMap data = parseMessage(QString::fromStdString(cmd.data));
        
        if (data.contains("raw_data") && data.contains("processed_data")) {
            // 标准AIS消息格式
            QString rawData = data["raw_data"].toString();
            QString processedData = data["processed_data"].toString();
            
            emit messageReceived(QString("AIS数据 - 原始: %1...").arg(rawData.left(50)));
            emit aisMessageReceived(rawData, processedData);
            
        } else if (!cmd.data.empty()) {
            // 非标准格式，但包含数据
            emit messageReceived(QString("AIS数据(原始): %1...").arg(QString::fromStdString(cmd.data).left(50)));
            emit rawAisMessageReceived(QString::fromStdString(cmd.data));
            
        } else {
            // 空数据
            emit messageReceived("收到空的AIS消息");
        }
        
    } catch (const std::exception &e) {
        emit errorOccurred(QString("AIS消息处理失败: %1").arg(e.what()));
    }
}

// 处理配置更新消息
void IPCClientManager::handleConfigUpdate(const ais::protocol::CommandMessage &cmd)
{
    try {
        if (!cmd.data.empty()) {
            QVariantMap config = parseMessage(QString::fromStdString(cmd.data));
            emit serviceConfigReceived(config);
            emit messageReceived("收到配置更新消息");
        }
    } catch (const std::exception &e) {
        emit errorOccurred(QString("配置更新处理失败: %1").arg(e.what()));
    }
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
            
            // 发送连接成功消息
            emit messageReceived(QString("成功连接到服务: %1:%2").arg(address).arg(port));
            return true;
        }
    } catch (const std::exception &e) {
        QString errorMsg = QString("连接失败: %1").arg(e.what());
        emit errorOccurred(errorMsg);
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
        emit messageReceived("已断开与服务连接");
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
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::START_SERVICE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "start"})";
    
    sendProtocolCommand(cmd);
}

void IPCClientManager::sendStopCommand()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::STOP_SERVICE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "stop"})";
    
    sendProtocolCommand(cmd);
}

void IPCClientManager::getServiceStatus()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_STATUS;
    cmd.sequence = generateSequence();
    cmd.data = "{}";
    
    sendProtocolCommand(cmd);
}

void IPCClientManager::getServiceConfig()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::CONFIG_UPDATE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "get_config"})";
    
    sendProtocolCommand(cmd);
}

void IPCClientManager::updateServiceConfig(const QVariantMap &config)
{
    QJsonDocument doc(QJsonObject::fromVariantMap(config));
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::CONFIG_UPDATE;
    cmd.sequence = generateSequence();
    cmd.data = jsonStr.toStdString();
    
    sendProtocolCommand(cmd);
}

void IPCClientManager::sendCustomCommand(const QString &commandType, const QVariantMap &params)
{
    QVariantMap command;
    command["action"] = commandType;
    command["params"] = params;
    
    QJsonDocument doc(QJsonObject::fromVariantMap(command));
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_MESSAGE_STATS; // 使用统计类型作为自定义命令
    cmd.sequence = generateSequence();
    cmd.data = jsonStr.toStdString();
    
    sendProtocolCommand(cmd);
}

AISParserManager* IPCClientManager::getParserManager() const
{
    return parserManager;
}

void IPCClientManager::sendProtocolCommand(const ais::protocol::CommandMessage &cmd)
{
    if (!isConnected()) {
        emit errorOccurred("未连接到服务器");
        return;
    }
    
    try {
        // 记录发送的命令
        emit messageReceived(QString("发送命令 - 类型: %1, 序列号: %2")
                           .arg(static_cast<int>(cmd.type))
                           .arg(cmd.sequence));
        
        ipcClient->sendCommand(cmd);
    } catch (const std::exception &e) {
        emit errorOccurred(QString("发送命令失败: %1").arg(e.what()));
    }
}

QVariantMap IPCClientManager::parseMessage(const QString &message)
{
    QVariantMap result;
    
    if (message.trimmed().isEmpty()) {
        return result;
    }
    
    try {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
            result = doc.object().toVariantMap();
        } else {
            // 如果不是JSON，将整个消息作为原始文本
            result["raw_text"] = message;
        }
    } catch (...) {
        // 解析失败，返回原始消息
        result["raw_text"] = message;
    }
    
    return result;
}

uint32_t IPCClientManager::generateSequence()
{
    static uint32_t sequence = 0;
    return ++sequence;
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