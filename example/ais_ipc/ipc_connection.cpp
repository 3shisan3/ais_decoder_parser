#include "ipc_connection.h"

#include "logger_define.h"

#include <chrono>
#include <thread>

namespace ais
{

IPCConnection::IPCConnection()
    : connectionState_(ConnectionState::DISCONNECTED)
    , autoReconnect_(false)
    , reconnectIntervalMs_(3000)
{
    // 设置默认错误处理器
    errorHandler_ = [](const std::string& error) {
        LOG_ERROR("IPC Connection Error: {}", error);
    };
    
    // 设置默认响应处理器
    responseHandler_ = [](const protocol::ResponseMessage& response) {
        LOG_DEBUG("Received response: sequence={}, status={}", 
                     response.sequence, static_cast<int>(response.status));
    };
}

IPCConnection::~IPCConnection()
{
    // 确保连接已停止
    // stop();
}

bool IPCConnection::sendCommand(const std::string& jsonStr, int timeoutMs)
{
    try {
        // 将JSON字符串解析为CommandMessage
        auto cmd = protocol::CommandMessage::fromJson(jsonStr);
        return sendCommand(cmd, timeoutMs);
    } catch (const std::exception& e) {
        setLastError("Invalid command JSON: " + std::string(e.what()));
        return false;
    }
}

void IPCConnection::setMessageHandler(MessageHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    messageHandler_ = std::move(handler);
}

void IPCConnection::setErrorHandler(ErrorHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    errorHandler_ = std::move(handler);
}

void IPCConnection::setResponseHandler(ResponseHandler handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    responseHandler_ = std::move(handler);
}

std::string IPCConnection::getLastError() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return lastError_;
}

IPCConnection::ConnectionState IPCConnection::getConnectionState() const
{
    return connectionState_.load();
}

void IPCConnection::enableAutoReconnect(bool enable, uint32_t intervalMs)
{
    autoReconnect_ = enable;
    reconnectIntervalMs_ = intervalMs;
    
    if (enable) {
        LOG_INFO("Auto reconnect enabled, interval: {}ms", intervalMs);
    } else {
        LOG_INFO("Auto reconnect disabled");
    }
}

void IPCConnection::handleMessage(const std::string& message)
{
    try {
        // 尝试解析为CommandMessage（服务端到客户端的消息）
        try {
            auto cmd = protocol::CommandMessage::fromJson(message);
            std::lock_guard<std::mutex> lock(handlerMutex_);
            if (messageHandler_) {
                messageHandler_(cmd);
            }
            return;
        } catch (...) {
            // 不是CommandMessage，继续尝试解析为ResponseMessage
        }
        
        // 尝试解析为ResponseMessage（服务端响应）
        try {
            auto response = protocol::ResponseMessage::fromJson(message);
            std::lock_guard<std::mutex> lock(handlerMutex_);
            if (responseHandler_) {
                responseHandler_(response);
            }
            return;
        } catch (...) {
            // 不是ResponseMessage，继续处理
        }
        
        // 无法识别的消息格式
        LOG_WARNING("Received unrecognized message format: {}", message);
        
    } catch (const std::exception& e) {
        handleError("Message handling failed: " + std::string(e.what()));
    }
}

void IPCConnection::handleError(const std::string& error)
{
    setLastError(error);
    
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (errorHandler_) {
        errorHandler_(error);
    }
    
    // 如果启用了自动重连，尝试重新连接
    if (autoReconnect_ && connectionState_ != ConnectionState::RECONNECTING) {
        LOG_INFO("Attempting to reconnect due to error: {}", error);
        setConnectionState(ConnectionState::RECONNECTING);
        
        // 在实际实现中，这里应该启动重连线程
        // 这里简化处理，只是记录日志
        LOG_INFO("Auto reconnect would start in {}ms", reconnectIntervalMs_.load());
    }
}

void IPCConnection::setLastError(const std::string& error)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    lastError_ = error;
    LOG_ERROR("IPC Connection Error: {}", error);
}

void IPCConnection::setConnectionState(ConnectionState newState)
{
    ConnectionState oldState = connectionState_.exchange(newState);
    
    // 记录状态变化
    if (oldState != newState) {
        const char* stateNames[] = {
            "DISCONNECTED", "CONNECTING", "CONNECTED", "RECONNECTING"
        };
        
        LOG_INFO("Connection state changed: {} -> {}", 
                    stateNames[static_cast<int>(oldState)],
                    stateNames[static_cast<int>(newState)]);
    }
}

} // namespace ais