#include "ipc_client_manager.h"

#include <QDateTime>
#include <QThread>

IPCClientManager::IPCClientManager(QObject *parent)
    : QObject(parent)
    , parserManager_(new AISParserManager(this))
    , serverAddress_("127.0.0.1")
    , serverPort_(2333)
    , connected_(false)
    , autoReconnect_(true)
    , reconnectAttempts_(0)
{
    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setInterval(5000); // 5秒重连间隔
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &IPCClientManager::onReconnectTimeout);

    initializeClient();
}

IPCClientManager::~IPCClientManager()
{
    disconnectFromServer();
    cleanupClient();
}

void IPCClientManager::initializeClient()
{
    cleanupClient();

    // 创建TCPClientSession实例
    clientSession_ = std::make_unique<ais::TCPClientSession>(
        serverAddress_.toStdString(),
        static_cast<uint16_t>(serverPort_));

    // 设置连接超时
    clientSession_->setConnectTimeout(5000);
    clientSession_->enableAutoReconnect(true, 3000);

    // 设置消息处理器 - 处理服务端主动推送的消息
    clientSession_->setMessageHandler([this](const ais::protocol::CommandMessage &cmd)
                                      { handleServerMessage(cmd); });

    // 设置响应处理器 - 处理客户端命令的响应
    clientSession_->setResponseHandler([this](const ais::protocol::ResponseMessage &response)
                                       { handleServerResponse(response); });

    // 设置错误处理器
    clientSession_->setErrorHandler([this](const std::string &error)
                                    { handleConnectionError(error); });
}

void IPCClientManager::cleanupClient()
{
    if (clientSession_)
    {
        clientSession_->stop();
        clientSession_.reset();
    }
}

// 处理服务端主动推送的消息
void IPCClientManager::handleServerMessage(const ais::protocol::CommandMessage &cmd)
{
    try
    {
        QString msgInfo = QString("收到服务端消息 - 类型: %1, 序列号: %2")
                              .arg(static_cast<int>(cmd.type))
                              .arg(cmd.sequence);
        emit messageReceived(msgInfo);

        // 根据CommandType进行不同的处理
        switch (cmd.type)
        {
        case ais::protocol::CommandType::SEND_MESSAGE:
            // AIS数据消息 - 服务端主动推送的AIS数据
            handleAISMessage(cmd);
            break;

        case ais::protocol::CommandType::CONFIG_UPDATE:
            // 配置更新消息 - 服务端推送的配置变更
            handleConfigUpdate(cmd);
            break;

        case ais::protocol::CommandType::HEARTBEAT:
            // 心跳消息
            emit messageReceived("收到服务端心跳");
            break;

        default:
            // 其他消息类型
            emit messageReceived(QString("收到服务端消息[类型%1]").arg(static_cast<int>(cmd.type)));
            break;
        }

        // 发送原始数据用于显示
        emit rawMessageReceived(QString::fromStdString(cmd.data));
    }
    catch (const std::exception &e)
    {
        QString errorMsg = QString("服务端消息处理异常: %1").arg(e.what());
        emit errorOccurred(errorMsg);
    }
}

// 处理服务端响应
void IPCClientManager::handleServerResponse(const ais::protocol::ResponseMessage &response)
{
    try
    {
        QString responseMsg = QString("收到服务响应 - 状态: %1, 序列号: %2")
                                  .arg(static_cast<int>(response.status))
                                  .arg(response.sequence);
        emit messageReceived(responseMsg);

        // 处理响应状态
        if (response.status == ais::protocol::ResponseStatus::SUCCESS)
        {
            handleSuccessResponse(response);
        }
        else
        {
            handleErrorResponse(response);
        }

        // 发送原始响应数据
        emit rawMessageReceived(QString::fromStdString(response.data));
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("响应处理异常: %1").arg(e.what()));
    }
}

// 处理连接错误
void IPCClientManager::handleConnectionError(const std::string &error)
{
    QString errorMsg = QString::fromStdString(error);
    emit errorOccurred(errorMsg);

    // 连接错误时自动重连
    if (autoReconnect_ && connected_)
    {
        connected_ = false;
        emit connectionStateChanged(false);
        startReconnectTimer();
    }
}

// 处理成功响应
void IPCClientManager::handleSuccessResponse(const ais::protocol::ResponseMessage &response)
{
    try
    {
        QVariantMap result = parseMessage(QString::fromStdString(response.data));

        // 根据响应数据内容分发处理
        if (result.contains("is_running"))
        {
            // 服务状态响应
            bool isRunning = result["is_running"].toBool();
            emit serviceStateChanged(isRunning);
            emit messageReceived(QString("服务状态: %1").arg(isRunning ? "运行中" : "已停止"));
        }
        else if (result.contains("ship_count"))
        {
            // 船舶数量响应
            int shipCount = result["ship_count"].toInt();
            emit messageReceived(QString("当前船舶数量: %1").arg(shipCount));
            emit shipCountReceived(shipCount);
        }
        else if (result.contains("messages_processed") || result.contains("messages_received"))
        {
            // 消息统计响应
            int processed = result.value("messages_processed", 0).toInt();
            int received = result.value("messages_received", 0).toInt();
            int sent = result.value("messages_sent", 0).toInt();
            
            QVariantMap stats;
            stats["received"] = received;
            stats["processed"] = processed;
            stats["sent"] = sent;
            
            emit messageReceived(QString("消息统计 - 接收: %1, 处理: %2, 发送: %3")
                                .arg(received).arg(processed).arg(sent));
            emit messageStatsReceived(stats);
        }
        else if (result.contains("success"))
        {
            // 操作结果响应
            bool success = result["success"].toBool();
            QString message = result.value("message", "").toString();
            if (success)
            {
                emit messageReceived(QString("操作成功: %1").arg(message));
            }
            else
            {
                emit errorOccurred(QString("操作失败: %1").arg(message));
            }
        }
        else if (result.contains("config"))
        {
            // 配置信息响应
            QVariantMap config = result["config"].toMap();
            emit serviceConfigReceived(config);
            emit messageReceived("收到服务配置信息");
        }
        else if (result.contains("logs"))
        {
            // 服务日志响应
            QVariantList logs = result["logs"].toList();
            emit messageReceived(QString("收到服务日志，共%1条").arg(logs.size()));
            // 可以进一步处理日志显示
        }
        else if (result.contains("alive"))
        {
            // 心跳响应
            emit messageReceived("服务端心跳正常");
        }
        else
        {
            // 未知成功响应格式
            emit messageReceived("收到成功响应");
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("成功响应解析失败: %1").arg(e.what()));
    }
}

// 处理错误响应
void IPCClientManager::handleErrorResponse(const ais::protocol::ResponseMessage &response)
{
    try
    {
        QVariantMap result = parseMessage(QString::fromStdString(response.data));
        QString errorInfo = result.value("error", QString::fromStdString(response.data)).toString();

        QString statusText;
        switch (response.status)
        {
        case ais::protocol::ResponseStatus::ERR:
            statusText = "服务器错误";
            break;
        case ais::protocol::ResponseStatus::INVALID_COMMAND:
            statusText = "无效命令";
            break;
        case ais::protocol::ResponseStatus::SERVICE_BUSY:
            statusText = "服务繁忙";
            break;
        case ais::protocol::ResponseStatus::NOT_CONNECTED:
            statusText = "服务未连接";
            break;
        default:
            statusText = "未知错误";
            break;
        }

        emit errorOccurred(QString("%1: %2").arg(statusText).arg(errorInfo));
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("错误响应解析失败: %1").arg(e.what()));
    }
}

// 处理AIS消息
void IPCClientManager::handleAISMessage(const ais::protocol::CommandMessage &cmd)
{
    try
    {
        QVariantMap data = parseMessage(QString::fromStdString(cmd.data));

        if (data.contains("raw_data") && data.contains("processed_data"))
        {
            // 标准AIS消息格式
            QString rawData = data["raw_data"].toString();
            QString processedData = data["processed_data"].toString();

            emit messageReceived(QString("收到AIS数据 - 原始: %1...").arg(rawData.left(50)));

            // 发射AIS数据信号
            emit aisMessageReceived(rawData, processedData);
            emit rawAisMessageReceived(rawData);

            // 可选：使用解析器进一步处理
            if (parserManager_)
            {
                QVariantMap parseResult = parserManager_->parseNMEAString(rawData);
                if (parseResult["success"].toBool())
                {
                    emit messageReceived("AIS数据解析成功");
                }
            }
        }
        else if (!cmd.data.empty())
        {
            // 非标准格式，但包含数据
            emit messageReceived(QString("收到AIS数据(原始): %1...").arg(QString::fromStdString(cmd.data).left(50)));
            emit rawAisMessageReceived(QString::fromStdString(cmd.data));
        }
        else
        {
            // 空数据
            emit messageReceived("收到空的AIS消息");
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("AIS消息处理失败: %1").arg(e.what()));
    }
}

// 处理配置更新消息
void IPCClientManager::handleConfigUpdate(const ais::protocol::CommandMessage &cmd)
{
    try
    {
        if (!cmd.data.empty())
        {
            QVariantMap config = parseMessage(QString::fromStdString(cmd.data));
            emit serviceConfigReceived(config);
            emit messageReceived("收到服务配置更新");
        }
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("配置更新处理失败: %1").arg(e.what()));
    }
}

bool IPCClientManager::connectToServer()
{
    return connectToServer(serverAddress_, serverPort_);
}

bool IPCClientManager::connectToServer(const QString &address, quint16 port)
{
    if (isConnected())
    {
        disconnectFromServer();
        QThread::msleep(100); // 短暂延迟
    }

    stopReconnectTimer();

    // 重新初始化客户端会话
    serverAddress_ = address;
    serverPort_ = port;
    initializeClient();

    try
    {
        bool success = clientSession_->start();
        if (success)
        {
            connected_ = true;
            reconnectAttempts_ = 0;
            emit connectionStateChanged(true);

            // 发送连接成功消息
            emit messageReceived(QString("成功连接到AIS服务: %1:%2").arg(address).arg(port));

            // 连接成功后立即获取服务状态
            QTimer::singleShot(100, this, &IPCClientManager::getServiceStatus);

            return true;
        }
        else
        {
            emit errorOccurred("启动客户端会话失败");
        }
    }
    catch (const std::exception &e)
    {
        QString errorMsg = QString("连接失败: %1").arg(e.what());
        emit errorOccurred(errorMsg);
    }

    // 连接失败，启动重连定时器
    if (autoReconnect_)
    {
        startReconnectTimer();
    }

    return false;
}

void IPCClientManager::disconnectFromServer()
{
    stopReconnectTimer();

    if (clientSession_)
    {
        clientSession_->stop();
        if (connected_)
        {
            connected_ = false;
            emit connectionStateChanged(false);
            emit messageReceived("已断开与AIS服务连接");
        }
    }
}

bool IPCClientManager::isConnected() const
{
    return connected_ && clientSession_ && clientSession_->isConnected();
}

void IPCClientManager::setServerAddress(const QString &address)
{
    serverAddress_ = address;
}

void IPCClientManager::setServerPort(quint16 port)
{
    serverPort_ = port;
}

QString IPCClientManager::serverAddress() const
{
    return serverAddress_;
}

quint16 IPCClientManager::serverPort() const
{
    return serverPort_;
}

void IPCClientManager::setConnectTimeout(int timeoutMs)
{
    if (clientSession_)
    {
        clientSession_->setConnectTimeout(timeoutMs);
    }
}

void IPCClientManager::enableAutoReconnect(bool enable, uint32_t intervalMs)
{
    autoReconnect_ = enable;
    if (clientSession_)
    {
        clientSession_->enableAutoReconnect(enable, intervalMs);
    }
}

// 服务控制命令实现
void IPCClientManager::sendStartCommand()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::START_SERVICE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "start"})";

    sendProtocolCommand(cmd);
    emit messageReceived("发送启动服务命令");
}

void IPCClientManager::sendStopCommand()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::STOP_SERVICE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "stop"})";

    sendProtocolCommand(cmd);
    emit messageReceived("发送停止服务命令");
}

void IPCClientManager::getServiceStatus()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_STATUS;
    cmd.sequence = generateSequence();
    cmd.data = "{}";

    sendProtocolCommand(cmd);
    emit messageReceived("发送获取服务状态命令");
}

void IPCClientManager::getServiceConfig()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::CONFIG_UPDATE;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "get_config"})";
    
    sendProtocolCommand(cmd);
    emit messageReceived("发送获取服务配置命令");
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
    emit messageReceived("发送更新服务配置命令");
}

void IPCClientManager::getShipCount()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_SHIP_COUNT;
    cmd.sequence = generateSequence();
    cmd.data = "{}";
    
    sendProtocolCommand(cmd);
    emit messageReceived("发送获取船舶数量命令");
}

void IPCClientManager::getMessageStats()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_MESSAGE_STATS;
    cmd.sequence = generateSequence();
    cmd.data = "{}";
    
    sendProtocolCommand(cmd);
    emit messageReceived("发送获取消息统计命令");
}

void IPCClientManager::toggleServiceLogs()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::CHANGE_SERVICE_LOGS;
    cmd.sequence = generateSequence();
    cmd.data = R"({"action": "toggle_logs"})";
    
    sendProtocolCommand(cmd);
    emit messageReceived("发送切换服务日志命令");
}

void IPCClientManager::sendHeartbeat()
{
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::HEARTBEAT;
    cmd.sequence = generateSequence();
    cmd.data = R"({"client_time": ")" +
        QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() + R"("})";
    
    sendProtocolCommand(cmd);
    emit messageReceived("发送心跳检测");
}

void IPCClientManager::sendCustomCommand(const QString &commandType, const QVariantMap &params)
{
    QVariantMap command;
    command["action"] = commandType;
    command["params"] = params;
    command["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(QJsonObject::fromVariantMap(command));
    QString jsonStr = doc.toJson(QJsonDocument::Compact);
    
    ais::protocol::CommandMessage cmd;
    cmd.type = ais::protocol::CommandType::GET_MESSAGE_STATS; // 使用统计类型作为自定义命令
    cmd.sequence = generateSequence();
    cmd.data = jsonStr.toStdString();
    
    sendProtocolCommand(cmd);
    emit messageReceived(QString("发送自定义命令: %1").arg(commandType));
}

AISParserManager* IPCClientManager::getParserManager() const
{
    return parserManager_;
}

void IPCClientManager::sendProtocolCommand(const ais::protocol::CommandMessage &cmd)
{
    if (!isConnected())
    {
        emit errorOccurred("未连接到服务器，无法发送命令");
        return;
    }

    try
    {
        QString cmdTypeText;
        switch (cmd.type)
        {
        case ais::protocol::CommandType::START_SERVICE:
            cmdTypeText = "启动服务";
            break;
        case ais::protocol::CommandType::STOP_SERVICE:
            cmdTypeText = "停止服务";
            break;
        case ais::protocol::CommandType::GET_STATUS:
            cmdTypeText = "获取状态";
            break;
        case ais::protocol::CommandType::GET_SHIP_COUNT:
            cmdTypeText = "获取船舶数量";
            break;
        case ais::protocol::CommandType::GET_MESSAGE_STATS:
            cmdTypeText = "获取消息统计";
            break;
        case ais::protocol::CommandType::CONFIG_UPDATE:
            cmdTypeText = "配置更新";
            break;
        case ais::protocol::CommandType::HEARTBEAT:
            cmdTypeText = "心跳检测";
            break;
        case ais::protocol::CommandType::CHANGE_SERVICE_LOGS:
            cmdTypeText = "切换日志";
            break;
        default:
            cmdTypeText = "未知命令";
            break;
        }

        emit messageReceived(QString("发送命令 - 类型: %1, 序列号: %2")
                                 .arg(cmdTypeText)
                                 .arg(cmd.sequence));

        clientSession_->sendCommand(cmd);
    }
    catch (const std::exception &e)
    {
        emit errorOccurred(QString("发送命令失败: %1").arg(e.what()));
    }
}

QVariantMap IPCClientManager::parseMessage(const QString &message)
{
    QVariantMap result;

    if (message.trimmed().isEmpty())
    {
        return result;
    }

    try
    {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isNull() && doc.isObject())
        {
            result = doc.object().toVariantMap();
        }
        else if (!doc.isNull() && doc.isArray())
        {
            result["array_data"] = doc.array().toVariantList();
        }
        else
        {
            result["raw_text"] = message;
        }
    }
    catch (...)
    {
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
    reconnectAttempts_++;
    if (reconnectAttempts_ <= 10)
    {
        reconnectTimer_->start();
        emit messageReceived(QString("尝试重新连接... (%1/10)").arg(reconnectAttempts_));
    }
    else
    {
        emit errorOccurred("重连次数过多，停止自动重连");
        autoReconnect_ = false;
    }
}

void IPCClientManager::stopReconnectTimer()
{
    reconnectTimer_->stop();
    reconnectAttempts_ = 0;
}

void IPCClientManager::onReconnectTimeout()
{
    if (autoReconnect_ && !isConnected())
    {
        emit messageReceived("自动重连中...");
        connectToServer();
    }
}