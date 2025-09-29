#include "tcp_session.h"
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
#endif

#include <system_error>

namespace ais
{

TCPSession::TCPSession(int socket, const std::string& sessionId)
    : socket_(socket), sessionId_(sessionId), running_(false)
{
    // 获取远程地址信息
#ifdef _WIN32
    // Windows平台获取远程地址
    sockaddr_in addr;
    int addr_len = sizeof(addr);
    if (getpeername(socket, (sockaddr*)&addr, &addr_len) == 0) {
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        remoteAddress_ = std::string(ip_str) + ":" + std::to_string(ntohs(addr.sin_port));
    } else {
        remoteAddress_ = "unknown";
    }
#else
    // Linux/Unix平台获取远程地址
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getpeername(socket, (sockaddr*)&addr, &len) == 0) {
        remoteAddress_ = std::string(inet_ntoa(addr.sin_addr)) + ":" + 
                        std::to_string(ntohs(addr.sin_port));
    } else {
        remoteAddress_ = "unknown";
    }
#endif
}

TCPSession::~TCPSession()
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
}

bool TCPSession::start()
{
    if (running_) return true;
    
    running_ = true;
    
    // 启动接收线程
    receiveThread_ = std::thread(&TCPSession::receiveThread, this);
    
    // 启动发送线程
    sendThread_ = std::thread(&TCPSession::sendThread, this);
    
    setConnectionState(ConnectionState::CONNECTED);
    return true;
}

void TCPSession::stop()
{
    if (!running_) return;
    
    running_ = false;
    sendQueueCV_.notify_all();
    
    if (receiveThread_.joinable()) receiveThread_.join();
    if (sendThread_.joinable()) sendThread_.join();
    
    setConnectionState(ConnectionState::DISCONNECTED);
}

bool TCPSession::isConnected() const
{
    return running_ && socket_ != -1;
}

bool TCPSession::sendCommand(const protocol::CommandMessage& command, int timeoutMs)
{
    std::string jsonStr = command.toJson();
    return sendCommand(jsonStr, timeoutMs);
}

bool TCPSession::sendCommand(const std::string& jsonStr, int timeoutMs)
{
    if (!isConnected()) {
        setLastError("Not connected");
        return false;
    }
    
    try {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueue_.push(jsonStr + '\n');    // client里用'\n'来识别结束
        sendQueueCV_.notify_one();
        return true;
    } catch (const std::exception& e) {
        setLastError("Send failed: " + std::string(e.what()));
        return false;
    }
}

bool TCPSession::sendResponse(const protocol::ResponseMessage& response, int timeoutMs)
{
    std::string jsonStr = response.toJson();
    return sendCommand(jsonStr, timeoutMs);
}

bool TCPSession::sendCommandWithResponse(const protocol::CommandMessage& command, 
                                       protocol::ResponseMessage& response, 
                                       int timeoutMs)
{
    if (!sendCommand(command, timeoutMs)) {
        return false;
    }
    
    setLastError("Not implemented");
    return false;
}

void TCPSession::receiveThread()
{
    char buffer[4096];
    
    while (running_ && socket_ != -1) {
#ifdef _WIN32
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#else
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#endif
        
        if (bytesRead > 0) {
            processReceivedData(buffer, bytesRead);
        } else if (bytesRead == 0) {
            // 连接关闭
            handleError("Connection closed by peer");
            break;
        } else {
            // 读取错误
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                handleError("Receive error: " + std::to_string(error));
                break;
            }
#else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                handleError("Receive error: " + std::string(strerror(errno)));
                break;
            }
#endif
        }
    }
}

void TCPSession::sendThread()
{
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
                    handleError("Send failed: " + std::to_string(error));
#else
                    handleError("Send failed: " + std::string(strerror(errno)));
#endif
                }
            }
        }
    }
}

void TCPSession::processReceivedData(const char* data, size_t size)
{
    receivePartialData_.append(data, size);
    
    // 处理完整消息（假设以换行符分隔）
    size_t pos;
    while ((pos = receivePartialData_.find('\n')) != std::string::npos) {
        std::string message = receivePartialData_.substr(0, pos);
        receivePartialData_.erase(0, pos + 1);
        
        try {
            auto cmd = protocol::CommandMessage::fromJson(message);
            handleMessage(message);
        } catch (const std::exception& e) {
            handleError("Invalid message: " + std::string(e.what()));
        }
    }
}

std::string TCPSession::getSessionId() const { return sessionId_; }
std::string TCPSession::getRemoteAddress() const { return remoteAddress_; }

} // namespace ais