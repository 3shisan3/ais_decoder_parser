#include "tcp_ipc_server.h"
#include "logger_define.h"
#include <system_error>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

namespace ais
{

AISServer::AISServer() : serverSocket_(-1), running_(false)
{
}

AISServer::~AISServer()
{
    stop();
}

bool AISServer::start(int port)
{
    if (running_) return true;
    
    // 创建服务器socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ == -1) {
        LOG_ERROR("Failed to create socket");
        return false;
    }
    
    // 设置socket选项
    int opt = 1;
    #ifdef _WIN32
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    #else
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
    
    // 绑定地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        LOG_ERROR("Failed to bind to port {}", port);
        #ifdef _WIN32
        closesocket(serverSocket_);
        #else
        close(serverSocket_);
        #endif
        return false;
    }
    
    // 开始监听
    if (listen(serverSocket_, 10) == -1) {
        LOG_ERROR("Failed to listen on port {}", port);
        #ifdef _WIN32
        closesocket(serverSocket_);
        #else
        close(serverSocket_);
        #endif
        return false;
    }
    
    running_ = true;
    acceptThread_ = std::thread(&AISServer::acceptThread, this);
    
    LOG_INFO("AIS server started on port {}", port);
    return true;
}

void AISServer::stop()
{
    if (!running_) return;
    
    running_ = false;
    
    // 关闭所有客户端连接
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        for (auto& [sessionId, session] : sessions_) {
            session->stop();
        }
        sessions_.clear();
    }
    
    // 关闭服务器socket
    if (serverSocket_ != -1) {
        #ifdef _WIN32
        closesocket(serverSocket_);
        #else
        close(serverSocket_);
        #endif
        serverSocket_ = -1;
    }
    
    if (acceptThread_.joinable()) {
        acceptThread_.join();
    }
    
    LOG_INFO("AIS server stopped");
}

bool AISServer::isRunning() const
{
    return running_;
}

size_t AISServer::getConnectionCount() const
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    return sessions_.size();
}

void AISServer::broadcast(const protocol::CommandMessage& message)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    for (auto& [sessionId, session] : sessions_) {
        session->sendCommand(message);
    }
}

bool AISServer::sendToClient(const std::string& sessionId, const protocol::CommandMessage& message)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = sessions_.find(sessionId);
    if (it != sessions_.end()) {
        return it->second->sendCommand(message);
    }
    return false;
}

void AISServer::acceptThread()
{
    while (running_) {
        sockaddr_in clientAddr{};
        #ifdef _WIN32
        int clientAddrLen = sizeof(clientAddr);
        #else
        socklen_t clientAddrLen = sizeof(clientAddr);
        #endif
        
        int clientSocket = accept(serverSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            if (running_) {
                LOG_ERROR("Accept failed");
            }
            continue;
        }
        
        // 创建会话ID
        std::string sessionId = std::string(inet_ntoa(clientAddr.sin_addr)) + ":" + 
                              std::to_string(ntohs(clientAddr.sin_port));
        
        // 创建TCP会话
        auto session = std::make_shared<TCPSession>(clientSocket, sessionId);
        
        // 设置消息处理器
        session->setMessageHandler([this, session](const protocol::CommandMessage& msg) {
            handleClientMessage(session, msg);
        });
        
        // 设置错误处理器（处理断开连接）
        session->setErrorHandler([this, sessionId](const std::string& error) {
            handleClientDisconnect(sessionId);
        });
        
        // 启动会话
        if (session->start()) {
            std::lock_guard<std::mutex> lock(sessionsMutex_);
            sessions_[sessionId] = session;
            LOG_INFO("Client connected: {}", sessionId);
        }
    }
}

void AISServer::handleClientMessage(TCPSessionPtr session, const protocol::CommandMessage& message)
{
    LOG_DEBUG("Received message from {}: type={}", session->getSessionId(), static_cast<int>(message.type));
    
    // 根据消息类型处理请求
    protocol::ResponseMessage response;
    response.sequence = message.sequence;
    
    switch (message.type) {
        case protocol::CommandType::GET_STATUS:
            response.status = protocol::ResponseStatus::SUCCESS;
            response.data = R"({"status": "running", "connections": )" + 
                          std::to_string(getConnectionCount()) + "}";
            break;
            
        case protocol::CommandType::START_SERVICE:
            response.status = protocol::ResponseStatus::SUCCESS;
            response.data = R"({"result": "service started"})";
            break;
            
        default:
            response.status = protocol::ResponseStatus::SUCCESS;
            response.data = R"({"result": "command processed"})";
            break;
    }
    
    session->sendResponse(response);
}

void AISServer::handleClientDisconnect(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessions_.erase(sessionId);
    LOG_INFO("Client disconnected: {}", sessionId);
}

} // namespace ais