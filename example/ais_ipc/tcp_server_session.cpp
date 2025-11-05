#include "tcp_server_session.h"
#include "logger_define.h"

// 平台特定的头文件包含
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#endif

#include <system_error>

namespace ais
{

TCPServerSession::TCPServerSession(int socket, const std::string& sessionId)
    : socket_(socket), sessionId_(sessionId), running_(false)
{
    // 获取客户端地址信息
#ifdef _WIN32
    sockaddr_in addr;
    int addr_len = sizeof(addr);
    if (getpeername(socket, (sockaddr*)&addr, &addr_len) == 0) {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        clientAddress_ = std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
    } else {
        clientAddress_ = "unknown";
    }
#else
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getpeername(socket, (sockaddr*)&addr, &len) == 0) {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        clientAddress_ = std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
    } else {
        clientAddress_ = "unknown";
    }
#endif
    
    LOG_DEBUG("TCPServerSession created for client: {} [{}]", clientAddress_, sessionId);
}

TCPServerSession::~TCPServerSession()
{
    stop();
    if (socket_ != -1) {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = -1;
    }
    LOG_DEBUG("TCPServerSession destroyed: {}", sessionId_);
}

bool TCPServerSession::start()
{
    if (running_) {
        LOG_WARNING("TCPServerSession already started: {}", sessionId_);
        return true;
    }
    
    running_ = true;
    
    // 设置socket为非阻塞模式
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(socket_, FIONBIO, &mode);
#else
    int flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);
#endif

    // 启动接收线程
    receiveThread_ = std::thread(&TCPServerSession::receiveThread, this);
    
    // 启动发送线程
    sendThread_ = std::thread(&TCPServerSession::sendThread, this);
    
    setConnectionState(ConnectionState::CONNECTED);
    LOG_INFO("TCPServerSession started: {} [{}]", clientAddress_, sessionId_);
    return true;
}

void TCPServerSession::stop()
{
    if (!running_) return;
    
    LOG_DEBUG("TCPServerSession stopping: {}", sessionId_);
    
    running_ = false;
    sendQueueCV_.notify_all();
    
    // 关闭socket以中断阻塞调用
    if (socket_ != -1) {
#ifdef _WIN32
        shutdown(socket_, SD_BOTH);
#else
        shutdown(socket_, SHUT_RDWR);
#endif
    }
    
    if (receiveThread_.joinable()) receiveThread_.join();
    if (sendThread_.joinable()) sendThread_.join();
    
    setConnectionState(ConnectionState::DISCONNECTED);
    LOG_INFO("TCPServerSession stopped: {}", sessionId_);
}

bool TCPServerSession::isConnected() const
{
    return running_ && socket_ != -1;
}

bool TCPServerSession::sendCommand(const protocol::CommandMessage& command, int timeoutMs)
{
    std::string jsonStr = command.toJson();
    return sendCommand(jsonStr, timeoutMs);
}

bool TCPServerSession::sendCommand(const std::string& jsonStr, int timeoutMs)
{
    if (!isConnected()) {
        setLastError("Not connected to client");
        return false;
    }
    
    try {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueue_.push(jsonStr + '\n');
        sendQueueCV_.notify_one();
        LOG_DEBUG("Message queued for client {}: {}", clientAddress_, jsonStr);
        return true;
    } catch (const std::exception& e) {
        setLastError("Send queue failed: " + std::string(e.what()));
        return false;
    }
}

bool TCPServerSession::sendResponse(const protocol::ResponseMessage& response, int timeoutMs)
{
    std::string jsonStr = response.toJson();
    return sendCommand(jsonStr, timeoutMs);
}

bool TCPServerSession::sendCommandWithResponse(const protocol::CommandMessage& command, 
                                             protocol::ResponseMessage& response, 
                                             int timeoutMs)
{
    if (!sendCommand(command, timeoutMs)) {
        return false;
    }
    
    // 简化实现：在实际项目中应该实现完整的请求-响应匹配机制
    response.status = protocol::ResponseStatus::SUCCESS;
    response.sequence = command.sequence;
    response.data = R"({"result": "command_sent", "note": "async_response"})";
    
    LOG_DEBUG("Command sent to client {} with async response, sequence: {}", 
              clientAddress_, command.sequence);
    return true;
}

void TCPServerSession::receiveThread()
{
    LOG_DEBUG("Receive thread started for client: {}", clientAddress_);
    
    char buffer[4096];
    
    while (running_ && socket_ != -1) {
#ifdef _WIN32
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#else
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#endif
        
        if (bytesRead > 0) {
            LOG_DEBUG("Received {} bytes from client {}", bytesRead, clientAddress_);
            processReceivedData(buffer, bytesRead);
        } else if (bytesRead == 0) {
            // 连接关闭
            LOG_INFO("Client {} closed the connection", clientAddress_);
            handleError("Connection closed by client");
            break;
        } else {
            // 读取错误
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                handleError("Receive error from client " + clientAddress_ + ": " + std::to_string(error));
                break;
            }
            // 非阻塞模式下没有数据可读，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
#else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                handleError("Receive error from client " + clientAddress_ + ": " + std::string(strerror(errno)));
                break;
            }
            // 非阻塞模式下没有数据可读，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
        }
    }
    
    // 清理资源
    if (socket_ != -1) {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = -1;
    }
    
    running_ = false;
    LOG_DEBUG("Receive thread ended for client: {}", clientAddress_);
}

void TCPServerSession::sendThread()
{
    LOG_DEBUG("Send thread started for client: {}", clientAddress_);
    
    while (running_) {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueueCV_.wait(lock, [this]() { 
            return !running_ || !sendQueue_.empty(); 
        });
        
        if (!running_) break;
        
        if (!sendQueue_.empty()) {
            std::string message = sendQueue_.front();
            sendQueue_.pop();
            lock.unlock();
            
            if (socket_ != -1) {
#ifdef _WIN32
                ssize_t bytesSent = send(socket_, message.c_str(), message.size(), 0);
#else
                ssize_t bytesSent = send(socket_, message.c_str(), message.size(), 0);
#endif
                if (bytesSent <= 0) {
#ifdef _WIN32
                    int error = WSAGetLastError();
                    if (error != WSAEWOULDBLOCK) {
                        handleError("Send failed to client " + clientAddress_ + ": " + std::to_string(error));
                        break;
                    }
                    // 发送缓冲区满，将消息重新放回队列
                    std::lock_guard<std::mutex> lock(sendQueueMutex_);
                    sendQueue_.push(message);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#else
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        handleError("Send failed to client " + clientAddress_ + ": " + std::string(strerror(errno)));
                        break;
                    }
                    // 发送缓冲区满，将消息重新放回队列
                    std::lock_guard<std::mutex> lock(sendQueueMutex_);
                    sendQueue_.push(message);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
                } else {
                    LOG_DEBUG("Sent {} bytes to client {}", bytesSent, clientAddress_);
                }
            }
        }
    }
    
    LOG_DEBUG("Send thread ended for client: {}", clientAddress_);
}

void TCPServerSession::processReceivedData(const char* data, size_t size)
{
    receivePartialData_.append(data, size);
    
    // 处理完整消息（以换行符分隔）
    size_t pos;
    while ((pos = receivePartialData_.find('\n')) != std::string::npos) {
        std::string message = receivePartialData_.substr(0, pos);
        receivePartialData_.erase(0, pos + 1);
        
        if (message.empty()) {
            continue; // 跳过空消息
        }
        
        LOG_DEBUG("Processing message from client {}: {}", clientAddress_, message);
        handleMessage(message);
    }
}

std::string TCPServerSession::getSessionId() const 
{ 
    return sessionId_; 
}

std::string TCPServerSession::getClientAddress() const 
{ 
    return clientAddress_; 
}

} // namespace ais