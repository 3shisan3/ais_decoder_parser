#include "tcp_client_session.h"
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

TCPClientSession::TCPClientSession(const std::string& serverIp, uint16_t serverPort)
    : serverIp_(serverIp)
    , serverPort_(serverPort)
    , running_(false)
    , connected_(false)
{
    LOG_DEBUG("TCPClientSession created: {}:{}", serverIp, serverPort);
}

TCPClientSession::~TCPClientSession()
{
    stop();
}

bool TCPClientSession::start()
{
    if (running_) {
        LOG_WARNING("TCPClientSession already started");
        return true;
    }
    
    std::lock_guard<std::mutex> lock(connectionMutex_);
    
    running_ = true;
    connected_ = false;
    
    setConnectionState(ConnectionState::CONNECTING);
    
    // 启动连接线程
    connectThread_ = std::thread(&TCPClientSession::connectThread, this);
    
    LOG_INFO("TCPClientSession starting, connecting to {}:{}", serverIp_, serverPort_);
    return true;
}

void TCPClientSession::stop()
{
    if (!running_) return;
    
    LOG_DEBUG("TCPClientSession stopping");
    
    running_ = false;
    connected_ = false;
    
    // 通知所有等待的线程
    sendQueueCV_.notify_all();
    
    // 关闭socket以中断阻塞的调用
    closeConnection();
    
    // 等待线程结束
    if (connectThread_.joinable()) {
        connectThread_.join();
    }
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    if (sendThread_.joinable()) {
        sendThread_.join();
    }
    
    setConnectionState(ConnectionState::DISCONNECTED);
    LOG_INFO("TCPClientSession stopped");
}

bool TCPClientSession::isConnected() const
{
    return connected_.load() && socket_ != -1;
}

bool TCPClientSession::sendCommand(const protocol::CommandMessage& command, int timeoutMs)
{
    std::string jsonStr = command.toJson();
    return sendCommand(jsonStr, timeoutMs);
}

bool TCPClientSession::sendCommand(const std::string& jsonStr, int timeoutMs)
{
    if (!isConnected()) {
        setLastError("Not connected to server");
        return false;
    }
    
    try {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueue_.push(jsonStr + '\n');  // 添加换行符作为消息分隔符
        sendQueueCV_.notify_one();
        LOG_DEBUG("Command queued for sending: {}", jsonStr);
        return true;
    } catch (const std::exception& e) {
        setLastError("Send queue failed: " + std::string(e.what()));
        return false;
    }
}

bool TCPClientSession::sendCommandWithResponse(const protocol::CommandMessage& command, 
                                                protocol::ResponseMessage& response, 
                                                int timeoutMs)
{
    // 简化实现：在实际项目中应该实现完整的请求-响应匹配机制
    if (!sendCommand(command, timeoutMs)) {
        return false;
    }
    
    // 设置一个默认的成功响应
    response.status = protocol::ResponseStatus::SUCCESS;
    response.sequence = command.sequence;
    response.data = R"({"result": "command_sent", "note": "async_response"})";
    
    LOG_DEBUG("Command sent with async response, sequence: {}", command.sequence);
    return true;
}

void TCPClientSession::connectThread()
{
    LOG_DEBUG("Connect thread started");
    
    while (running_) {
        if (establishConnection()) {
            // 连接成功，启动收发线程
            connected_ = true;
            setConnectionState(ConnectionState::CONNECTED);
            
            receiveThread_ = std::thread(&TCPClientSession::receiveThread, this);
            sendThread_ = std::thread(&TCPClientSession::sendThread, this);
            
            LOG_INFO("Successfully connected to server {}:{}", serverIp_, serverPort_);
            
            // 等待连接断开
            if (receiveThread_.joinable()) {
                receiveThread_.join();
            }
            if (sendThread_.joinable()) {
                sendThread_.join();
            }
            
            connected_ = false;
            setConnectionState(ConnectionState::DISCONNECTED);
            LOG_INFO("Disconnected from server");
        }
        
        // 检查是否需要重连
        if (running_ && autoReconnect_) {
            LOG_INFO("Auto reconnect in {}ms", reconnectIntervalMs_.load());
            std::this_thread::sleep_for(std::chrono::milliseconds(reconnectIntervalMs_.load()));
            setConnectionState(ConnectionState::RECONNECTING);
        } else {
            break;
        }
    }
    
    LOG_DEBUG("Connect thread ended");
}

bool TCPClientSession::establishConnection()
{
    closeConnection();
    
#ifdef _WIN32
    // Windows平台初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        handleError("WSAStartup failed");
        return false;
    }
#endif

    // 创建socket
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1) {
        handleError("Socket creation failed");
        return false;
    }
    
    // 设置服务器地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort_);
    
    if (inet_pton(AF_INET, serverIp_.c_str(), &serverAddr.sin_addr) <= 0) {
        handleError("Invalid server IP address: " + serverIp_);
        closeConnection();
        return false;
    }
    
    // 设置连接超时
#ifdef _WIN32
    DWORD timeout = connectTimeoutMs_;
    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = connectTimeoutMs_ / 1000;
    tv.tv_usec = (connectTimeoutMs_ % 1000) * 1000;
    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    
    // 连接服务器
    LOG_DEBUG("Connecting to {}:{}", serverIp_, serverPort_);
    
    if (connect(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        handleError("Connection to " + serverIp_ + ":" + std::to_string(serverPort_) + " failed");
        closeConnection();
        return false;
    }
    
    // 设置socket为非阻塞模式（可选）
#ifdef _WIN32
    u_long mode = 1;  // 1表示非阻塞
    if (ioctlsocket(socket_, FIONBIO, &mode) != 0) {
        LOG_WARNING("Failed to set non-blocking mode");
    }
#else
    int flags = fcntl(socket_, F_GETFL, 0);
    if (fcntl(socket_, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_WARNING("Failed to set non-blocking mode");
    }
#endif

    return true;
}

void TCPClientSession::closeConnection()
{
    if (socket_ != -1) {
#ifdef _WIN32
        closesocket(socket_);
        WSACleanup();
#else
        close(socket_);
#endif
        socket_ = -1;
    }
}

void TCPClientSession::receiveThread()
{
    LOG_DEBUG("Receive thread started");
    
    char buffer[4096];
    
    while (running_ && connected_ && socket_ != -1) {
#ifdef _WIN32
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#else
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#endif
        
        if (bytesRead > 0) {
            LOG_DEBUG("Received {} bytes from server", bytesRead);
            processReceivedData(buffer, bytesRead);
        } else if (bytesRead == 0) {
            // 连接关闭
            LOG_INFO("Server closed the connection");
            break;
        } else {
            // 读取错误
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                handleError("Receive error: " + std::to_string(error));
                break;
            }
            // 非阻塞模式下没有数据可读，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
#else
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                handleError("Receive error: " + std::string(strerror(errno)));
                break;
            }
            // 非阻塞模式下没有数据可读，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
#endif
        }
    }
    
    connected_ = false;
    LOG_DEBUG("Receive thread ended");
}

void TCPClientSession::sendThread()
{
    LOG_DEBUG("Send thread started");
    
    while (running_ && connected_) {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueueCV_.wait(lock, [this]() { 
            return !running_ || !connected_ || !sendQueue_.empty(); 
        });
        
        if (!running_ || !connected_) break;
        
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
                        handleError("Send failed: " + std::to_string(error));
                        break;
                    }
#else
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        handleError("Send failed: " + std::string(strerror(errno)));
                        break;
                    }
#endif
                    // 发送缓冲区满，将消息重新放回队列
                    std::lock_guard<std::mutex> lock(sendQueueMutex_);
                    sendQueue_.push(message);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } else {
                    LOG_DEBUG("Sent {} bytes to server", bytesSent);
                }
            }
        }
    }
    
    LOG_DEBUG("Send thread ended");
}

void TCPClientSession::processReceivedData(const char* data, size_t size)
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
        
        LOG_DEBUG("Processing message: {}", message);
        handleMessage(message);
    }
}

std::string TCPClientSession::getServerAddress() const
{
    return serverIp_ + ":" + std::to_string(serverPort_);
}

void TCPClientSession::setConnectTimeout(int timeoutMs)
{
    connectTimeoutMs_ = timeoutMs;
}

} // namespace ais