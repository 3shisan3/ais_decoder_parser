#include "ipc_server_manager.h"
#include "logger_define.h"

#include <sstream>
#include <system_error>

// 平台特定的头文件包含
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

namespace ais
{

IPCServerManager::IPCServerManager()
    : serverSocket_(-1)
    , running_(false)
{
#ifdef _WIN32
    // Windows平台需要初始化Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

IPCServerManager::~IPCServerManager()
{
    stop();
    
#ifdef _WIN32
    WSACleanup();
#endif
}

bool IPCServerManager::start(int port, StatusCallback statusCallback, 
                      LogCallback logCallback, ServiceControlCallback serviceControlCallback,
                      LogControlCallback logControlCallback)
{
    if (running_) {
        LOG_WARNING("IPC Manager is already running");
        return true;
    }
    
    // 设置回调函数
    statusCallback_ = std::move(statusCallback);
    logCallback_ = std::move(logCallback);
    serviceControlCallback_ = std::move(serviceControlCallback);
    logControlCallback_ = std::move(logControlCallback);
    
    // 初始化服务器socket
    if (!initializeServerSocket(port)) {
        LOG_ERROR("Failed to initialize server socket on port {}", port);
        return false;
    }
    
    running_ = true;
    
    // 启动接受连接线程
    acceptThread_ = std::thread(&IPCServerManager::acceptThread, this);
    
    LOG_INFO("IPC Manager started successfully on port {}", port);
    return true;
}

void IPCServerManager::stop()
{
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // 关闭所有客户端连接
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        for (auto& [sessionId, session] : sessions_) {
            if (session) {
                session->stop();
                LOG_DEBUG("Stopped session: {}", sessionId);
            }
        }
        sessions_.clear();
    }
    
    // 关闭服务器socket以唤醒accept线程
    cleanupServer();
    
    // 等待接受连接线程结束
    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    
    LOG_INFO("IPC Manager stopped");
}

bool IPCServerManager::isRunning() const
{
    return running_;
}

size_t IPCServerManager::getConnectionCount() const
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    return sessions_.size();
}

void IPCServerManager::broadcastAISData(const std::string& rawData, const std::string& processedData)
{
    if (!running_) {
        return;
    }
    
    protocol::CommandMessage msg;
    msg.type = protocol::CommandType::SEND_MESSAGE;
    msg.sequence = 0; // 广播消息序列号为0
    
    nlohmann::json data;
    data["raw_data"] = rawData;
    data["processed_data"] = processedData;
    data["timestamp"] = std::time(nullptr);
    
    msg.data = data.dump();
    
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    for (auto& [sessionId, session] : sessions_) {
        if (session && session->isConnected()) {
            if (!session->sendCommand(msg)) {
                LOG_WARNING("Failed to broadcast to session: {}", sessionId);
            }
        }
    }
    
    LOG_DEBUG("Broadcast AIS data to {} clients", sessions_.size());
}

void IPCServerManager::setAISService(std::shared_ptr<AISCommunicationService> aisService)
{
    aisService_ = std::move(aisService);
}

bool IPCServerManager::initializeServerSocket(int port)
{
    // 创建服务器socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ == -1) {
        LOG_ERROR("Failed to create server socket");
        return false;
    }
    
    // 设置socket选项（地址重用）
    int opt = 1;
#ifdef _WIN32
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == -1) {
#else
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
#endif
        LOG_WARNING("Failed to set SO_REUSEADDR option, but continuing...");
    }
    
    // 绑定地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        LOG_ERROR("Failed to bind to port {}", port);
        cleanupServer();
        return false;
    }
    
    // 开始监听
    if (listen(serverSocket_, 10) == -1) {
        LOG_ERROR("Failed to listen on port {}", port);
        cleanupServer();
        return false;
    }
    
    LOG_INFO("Server socket initialized, listening on port {}", port);
    return true;
}

void IPCServerManager::cleanupServer()
{
    if (serverSocket_ != -1) {
#ifdef _WIN32
        closesocket(serverSocket_);
#else
        close(serverSocket_);
#endif
        serverSocket_ = -1;
    }
}

void IPCServerManager::acceptThread()
{
    LOG_INFO("Accept thread started");
    
    while (running_ && serverSocket_ != -1) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddr);
#else
        socklen_t clientAddrLen = sizeof(clientAddr);
#endif
        
        // 接受客户端连接
        int clientSocket = accept(serverSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            if (running_) {
                LOG_ERROR("Accept failed, error: {}", 
#ifdef _WIN32
                    WSAGetLastError()
#else
                    errno
#endif
                );
            }
            continue;
        }
        
        // 创建会话ID
        std::string sessionId = std::string(inet_ntoa(clientAddr.sin_addr)) + ":" + 
                              std::to_string(ntohs(clientAddr.sin_port));
        
        LOG_INFO("New client connected: {}", sessionId);
        
        // 创建TCP会话
        auto session = std::make_shared<TCPSession>(clientSocket, sessionId);
        
        // 设置消息处理器
        session->setMessageHandler([this, session](const protocol::CommandMessage& msg) {
            handleClientMessage(session, msg);
        });
        
        // 设置错误处理器（处理断开连接）
        session->setErrorHandler([this, sessionId](const std::string& error) {
            LOG_INFO("Client {} disconnected: {}", sessionId, error);
            handleClientDisconnect(sessionId);
        });
        
        // 启动会话
        if (session->start()) {
            std::lock_guard<std::mutex> lock(sessionsMutex_);
            sessions_[sessionId] = session;
            LOG_INFO("Session started for client: {}", sessionId);
        } else {
            LOG_ERROR("Failed to start session for client: {}", sessionId);
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
        }
    }
    
    LOG_INFO("Accept thread stopped");
}

void IPCServerManager::handleClientMessage(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    LOG_DEBUG("Received message from {}: type={}, sequence={}", 
              session->getSessionId(), static_cast<int>(message.type), message.sequence);
    
    try {
        // 根据命令类型分发处理
        switch (message.type) {
            case protocol::CommandType::GET_STATUS:
                handleGetStatus(session, message);
                break;
                
            case protocol::CommandType::START_SERVICE:
                handleStartService(session, message);
                break;
                
            case protocol::CommandType::STOP_SERVICE:
                handleStopService(session, message);
                break;
                
            case protocol::CommandType::GET_SHIP_COUNT:
                handleGetShipCount(session, message);
                break;
                
            case protocol::CommandType::CONFIG_UPDATE:
                handleConfigUpdate(session, message);
                break;
                
            case protocol::CommandType::GET_MESSAGE_STATS:
                handleGetMessageStats(session, message);
                break;
                
            case protocol::CommandType::HEARTBEAT:
                handleHeartbeat(session, message);
                break;
                
            case protocol::CommandType::CHANGE_SERVICE_LOGS:
                handleChangeServiceLogs(session, message);
                break;
                
            case protocol::CommandType::SEND_MESSAGE:
                // SEND_MESSAGE是服务器到客户端的命令，客户端不应发送
                sendResponse(session, protocol::ResponseStatus::INVALID_COMMAND, 
                           message.sequence, R"({"error": "Invalid command direction"})");
                break;
                
            default:
                sendResponse(session, protocol::ResponseStatus::INVALID_COMMAND, 
                           message.sequence, R"({"error": "Unknown command type"})");
                break;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling client message from {}: {}", session->getSessionId(), e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, 
                   message.sequence, R"({"error": "Internal server error"})");
    }
}

void IPCServerManager::handleClientDisconnect(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    if (sessions_.erase(sessionId) > 0) {
        LOG_INFO("Client session removed: {}", sessionId);
    }
}

void IPCServerManager::handleGetStatus(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    if (!statusCallback_) {
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Status callback not available"})");
        return;
    }
    
    try {
        auto status = statusCallback_();
        auto statusJson = status.toJson();
        sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, statusJson.dump());
        LOG_DEBUG("Sent service status to client: {}", session->getSessionId());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting service status: {}", e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Failed to get service status"})");
    }
}

void IPCServerManager::handleStartService(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    bool success = false;
    std::string resultMessage;
    
    if (serviceControlCallback_) {
        success = serviceControlCallback_(true); // true表示启动服务
        resultMessage = success ? "Service started successfully" : "Failed to start service";
    } else {
        // 如果没有控制回调，认为服务已经在运行
        success = true;
        resultMessage = "Service is already running (no control callback available)";
    }
    
    nlohmann::json result;
    result["success"] = success;
    result["message"] = resultMessage;
    result["timestamp"] = std::time(nullptr);
    
    auto status = success ? protocol::ResponseStatus::SUCCESS : protocol::ResponseStatus::ERR;
    sendResponse(session, status, message.sequence, result.dump());
    
    LOG_INFO("Start service command processed for client {}: {}", session->getSessionId(), resultMessage);
}

void IPCServerManager::handleStopService(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    bool success = false;
    std::string resultMessage;
    
    if (serviceControlCallback_) {
        success = serviceControlCallback_(false); // false表示停止服务
        resultMessage = success ? "Service stopped successfully" : "Failed to stop service";
    } else {
        // 如果没有控制回调，拒绝停止服务
        success = false;
        resultMessage = "Remote service stop is not allowed (no control callback available)";
    }
    
    nlohmann::json result;
    result["success"] = success;
    result["message"] = resultMessage;
    result["timestamp"] = std::time(nullptr);
    
    auto status = success ? protocol::ResponseStatus::SUCCESS : protocol::ResponseStatus::ERR;
    sendResponse(session, status, message.sequence, result.dump());
    
    LOG_INFO("Stop service command processed for client {}: {}", session->getSessionId(), resultMessage);
}

void IPCServerManager::handleGetShipCount(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    if (!aisService_) {
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "AIS service not available"})");
        return;
    }
    
    try {
        size_t shipCount = aisService_->getShipCount();
        nlohmann::json result;
        result["ship_count"] = shipCount;
        result["timestamp"] = std::time(nullptr);
        sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, result.dump());
        
        LOG_DEBUG("Sent ship count {} to client: {}", shipCount, session->getSessionId());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting ship count: {}", e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Failed to get ship count"})");
    }
}

void IPCServerManager::handleConfigUpdate(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    try {
        // 解析配置数据
        auto configJson = nlohmann::json::parse(message.data);
        
        // 这里应该实现具体的配置更新逻辑
        // 由于涉及具体业务逻辑，这里只记录并返回成功
        
        LOG_INFO("Configuration update requested by client: {}", session->getSessionId());
        LOG_DEBUG("Configuration data: {}", message.data);
        
        nlohmann::json result;
        result["success"] = true;
        result["message"] = "Configuration update received (implementation required)";
        result["timestamp"] = std::time(nullptr);
        
        sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, result.dump());
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error updating config for client {}: {}", session->getSessionId(), e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Invalid configuration format"})");
    }
}

void IPCServerManager::handleGetMessageStats(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    if (!statusCallback_) {
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Status callback not available"})");
        return;
    }
    
    try {
        auto status = statusCallback_();
        nlohmann::json result;
        result["messages_received"] = status.messagesReceived;
        result["messages_sent"] = status.messagesSent;
        result["messages_processed"] = status.messagesProcessed;
        result["last_message_time"] = status.lastMessageTime;
        result["timestamp"] = std::time(nullptr);
        
        sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, result.dump());
        
        LOG_DEBUG("Sent message stats to client: {}", session->getSessionId());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting message stats: {}", e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                   R"({"error": "Failed to get message statistics"})");
    }
}

void IPCServerManager::handleHeartbeat(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    // 心跳包直接返回成功，包含服务器时间戳
    nlohmann::json result;
    result["alive"] = true;
    result["timestamp"] = std::time(nullptr);
    result["server_time"] = std::time(nullptr);
    
    sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, result.dump());
    
    LOG_DEBUG("Heartbeat response sent to client: {}", session->getSessionId());
}

void IPCServerManager::handleChangeServiceLogs(TCPSessionPtr session, const protocol::CommandMessage &message)
{
    try
    {
        std::string resultMessage = "";
        bool success = true;
        // 有控制请求：变更数据广播状态
        if (logControlCallback_)
        {
            bool curStatus = logControlCallback_();
            resultMessage = curStatus ? "AIS数据广播已启用" : "AIS数据广播已禁用";
        }
        else
        {
            success = false;
            resultMessage = "数据广播控制不可用（无控制回调）";
        }

        // 获取当前服务状态信息
        std::vector<std::string> logs;
        if (logCallback_)
        {
            logs = logCallback_();
        }
        else
        {
            logs = {"日志回调不可用"};
        }

        // 构建响应
        nlohmann::json result;
        result["success"] = success;
        result["message"] = resultMessage;
        result["logs"] = logs;
        result["timestamp"] = std::time(nullptr);

        sendResponse(session, protocol::ResponseStatus::SUCCESS, message.sequence, result.dump());

        LOG_DEBUG("Service logs response sent to client {}: {} log entries",
                  session->getSessionId(), logs.size());
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error handling service logs request for client {}: {}", session->getSessionId(), e.what());
        sendResponse(session, protocol::ResponseStatus::ERR, message.sequence,
                     R"({"error": "处理服务日志请求失败"})");
    }
}

void IPCServerManager::sendResponse(TCPSessionPtr session, protocol::ResponseStatus status, 
                            uint32_t sequence, const std::string& data)
{
    if (!session || !session->isConnected()) {
        LOG_WARNING("Cannot send response to disconnected session");
        return;
    }
    
    protocol::ResponseMessage response;
    response.status = status;
    response.sequence = sequence;
    response.data = data;
    
    if (!session->sendResponse(response)) {
        LOG_ERROR("Failed to send response to client: {}", session->getSessionId());
    }
}

} // namespace ais