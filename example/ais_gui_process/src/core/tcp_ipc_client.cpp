#include "tcp_ipc_client.h"

#include <system_error>
#include <iostream>

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
#include <netdb.h>
#endif

#include <cstring>

namespace ais
{

AISClient::AISClient() 
    : socket_(-1)
    , port_(0)
    , running_(false)
    , connected_(false)
{
#ifdef _WIN32
    static bool winsockInitialized = false;
    if (!winsockInitialized) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "WSAStartup failed" << std::endl;
        }
        winsockInitialized = true;
    }
#endif
}

AISClient::~AISClient()
{
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool AISClient::start()
{
    // 对于客户端，start等同于connect
    if (host_.empty() || port_ == 0) {
        setLastError("Cannot start without host and port");
        return false;
    }
    return connectToServer(host_, port_);
}

void AISClient::stop()
{
    disconnect();
}

bool AISClient::isConnected() const
{
    return connected_ && socket_ != -1;
}

bool AISClient::connectToServer(const std::string& host, int port)
{
    if (running_) {
        disconnect();
    }
    
    host_ = host;
    port_ = port;
    
    if (!establishConnection(host, port)) {
        setLastError("Connection failed to " + host + ":" + std::to_string(port));
        return false;
    }
    
    running_ = true;
    connected_ = true;

    // 启动接收和发送线程
    receiveThread_ = std::thread(&AISClient::receiveThread, this);
    sendThread_ = std::thread(&AISClient::sendThread, this);
    
    setConnectionState(ConnectionState::CONNECTED);
    std::cout << "Connected to server: " << host << ":" << port << std::endl;
    return true;
}

void AISClient::disconnect()
{
    if (!running_) return;
    
    running_ = false;
    connected_ = false;
    
    sendQueueCV_.notify_all();
    closeSocket();
    
    if (receiveThread_.joinable()) receiveThread_.join();
    if (sendThread_.joinable()) sendThread_.join();
    
    std::lock_guard<std::mutex> lock(sendQueueMutex_);
    while (!sendQueue_.empty()) sendQueue_.pop();
    
    setConnectionState(ConnectionState::DISCONNECTED);
    std::cout << "Disconnected from server" << std::endl;
}

bool AISClient::establishConnection(const std::string& host, int port)
{
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1) {
        setLastError("Failed to create socket");
        return false;
    }
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        // 使用getaddrinfo替代gethostbyname（更现代的方式）
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || res == nullptr) {
            setLastError("Invalid address: " + host);
            closeSocket();
            return false;
        }
        
        serverAddr.sin_addr = ((sockaddr_in*)res->ai_addr)->sin_addr;
        freeaddrinfo(res);
    }
    
    // 使用::connect来调用系统的connect函数
    if (::connect(socket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        setLastError("Connect failed");
        closeSocket();
        return false;
    }
    
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &serverAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    remoteAddress_ = std::string(ipStr) + ":" + std::to_string(port);
    
    return true;
}

void AISClient::closeSocket()
{
    if (socket_ != -1) {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = -1;
    }
}

bool AISClient::sendCommand(const protocol::CommandMessage& command, int timeoutMs)
{
    std::string jsonStr = command.toJson() + "\n";
    return sendCommand(jsonStr, timeoutMs);
}

bool AISClient::sendCommand(const std::string& jsonStr, int timeoutMs)
{
    if (!isConnected()) {
        setLastError("Not connected");
        return false;
    }
    
    try {
        std::lock_guard<std::mutex> lock(sendQueueMutex_);
        sendQueue_.push(jsonStr);
        sendQueueCV_.notify_one();
        return true;
    } catch (const std::exception& e) {
        setLastError("Queue failed: " + std::string(e.what()));
        return false;
    }
}

bool AISClient::sendCommandWithResponse(const protocol::CommandMessage& command, 
                                       protocol::ResponseMessage& response, 
                                       int timeoutMs)
{
    if (!sendCommand(command, timeoutMs)) {
        return false;
    }
    
    setLastError("Response waiting not implemented");
    return false;
}

void AISClient::setMessageHandler(MessageHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    messageHandler_ = std::move(handler);
}

void AISClient::setResponseHandler(ResponseHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    responseHandler_ = std::move(handler);
}

void AISClient::setErrorHandler(ErrorHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    errorHandler_ = std::move(handler);
}

void AISClient::receiveThread()
{
    char buffer[4096];
    
    while (running_ && isConnected()) {
#ifdef _WIN32
        int bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#else
        ssize_t bytesRead = recv(socket_, buffer, sizeof(buffer), 0);
#endif
        
        if (bytesRead > 0) {
            processReceivedData(buffer, bytesRead);
        } else if (bytesRead == 0) {
            handleError("Connection closed by server");
            break;
        } else {
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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    if (running_) stop();
}

void AISClient::sendThread()
{
    while (running_) {
        std::unique_lock<std::mutex> lock(sendQueueMutex_);
        sendQueueCV_.wait(lock, [this]() { return !running_ || !sendQueue_.empty(); });
        
        if (!running_) break;
        
        if (!sendQueue_.empty()) {
            std::string message = sendQueue_.front();
            sendQueue_.pop();
            lock.unlock();
            
            if (isConnected()) {
#ifdef _WIN32
                int bytesSent = send(socket_, message.c_str(), message.size(), 0);
#else
                ssize_t bytesSent = send(socket_, message.c_str(), message.size(), 0);
#endif
                
                if (bytesSent <= 0) {
#ifdef _WIN32
                    handleError("Send failed: " + std::to_string(WSAGetLastError()));
#else
                    handleError("Send failed: " + std::string(strerror(errno)));
#endif
                    break;
                }
            }
        }
    }
}

void AISClient::processReceivedData(const char* data, size_t size)
{
    receiveBuffer_.append(data, size);
    
    size_t pos;
    while ((pos = receiveBuffer_.find('\n')) != std::string::npos) {
        std::string message = receiveBuffer_.substr(0, pos);
        receiveBuffer_.erase(0, pos + 1);
        
        try {
            // 尝试解析为CommandMessage
            try {
                auto cmd = protocol::CommandMessage::fromJson(message);
                std::lock_guard<std::mutex> lock(handlerMutex_);
                if (messageHandler_) messageHandler_(cmd);
                continue;
            } catch (...) {}
            
            // 尝试解析为ResponseMessage
            try {
                auto response = protocol::ResponseMessage::fromJson(message);
                std::lock_guard<std::mutex> lock(handlerMutex_);
                if (responseHandler_) responseHandler_(response);
                continue;
            } catch (...) {}
            
            std::cout << "Unrecognized message: " << message << std::endl;
            
        } catch (const std::exception& e) {
            handleError("Message processing failed: " + std::string(e.what()));
        }
    }
}

std::string AISClient::getRemoteAddress() const
{
    return remoteAddress_;
}

} // namespace ais