#ifndef IPC_CONNECTION_H
#define IPC_CONNECTION_H

#include "protocol.h"

#include <atomic>
#include <functional>
#include <string>
#include <memory>
#include <mutex>

namespace ais
{

/**
 * @brief IPC连接基类
 * 
 * 提供进程间通信的基础接口，支持同步和异步消息处理
 * 支持连接状态管理和自动重连机制
 */
class IPCConnection
{
public:
    using MessageHandler = std::function<void(const protocol::CommandMessage&)>;
    using ErrorHandler = std::function<void(const std::string&)>;
    using ResponseHandler = std::function<void(const protocol::ResponseMessage&)>;
    
    /**
     * @brief 连接状态枚举
     */
    enum class ConnectionState {
        DISCONNECTED,   // 未连接
        CONNECTING,     // 连接中
        CONNECTED,      // 已连接
        RECONNECTING    // 重连中
    };
    
    /**
     * @brief 构造函数
     */
    IPCConnection();
    
    /**
     * @brief 虚析构函数
     */
    virtual ~IPCConnection();
    
    // 删除拷贝构造函数和赋值运算符
    IPCConnection(const IPCConnection&) = delete;
    IPCConnection& operator=(const IPCConnection&) = delete;
    
    // 纯虚接口
    /**
     * @brief 启动连接
     * @return 成功返回true，失败返回false
     */
    virtual bool start() = 0;
    
    /**
     * @brief 停止连接
     */
    virtual void stop() = 0;
    
    /**
     * @brief 检查连接状态
     * @return true已连接，false未连接
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief 发送命令消息
     * @param command 命令消息
     * @param timeoutMs 超时时间（毫秒）
     * @return 成功返回true，失败返回false
     */
    virtual bool sendCommand(const protocol::CommandMessage& command, int timeoutMs = 5000) = 0;
    
    /**
     * @brief 发送命令消息并等待响应
     * @param command 命令消息
     * @param response 响应消息输出参数
     * @param timeoutMs 超时时间（毫秒）
     * @return 成功返回true，失败返回false
     */
    virtual bool sendCommandWithResponse(const protocol::CommandMessage& command, 
                                        protocol::ResponseMessage& response, 
                                        int timeoutMs = 5000) = 0;
    
    // 辅助方法
    /**
     * @brief 发送命令消息（字符串版本）
     * @param jsonStr JSON格式的命令字符串
     * @param timeoutMs 超时时间（毫秒）
     * @return 成功返回true，失败返回false
     */
    virtual bool sendCommand(const std::string& jsonStr, int timeoutMs = 5000);
    
    /**
     * @brief 设置消息处理回调
     * @param handler 消息处理函数
     */
    void setMessageHandler(MessageHandler handler);
    
    /**
     * @brief 设置错误处理回调
     * @param handler 错误处理函数
     */
    void setErrorHandler(ErrorHandler handler);
    
    /**
     * @brief 设置响应处理回调
     * @param handler 响应处理函数
     */
    void setResponseHandler(ResponseHandler handler);
    
    /**
     * @brief 获取最后错误信息
     * @return 错误描述字符串
     */
    std::string getLastError() const;
    
    /**
     * @brief 获取连接状态
     * @return 当前连接状态
     */
    ConnectionState getConnectionState() const;
    
    /**
     * @brief 启用自动重连
     * @param enable 是否启用
     * @param intervalMs 重连间隔（毫秒）
     */
    void enableAutoReconnect(bool enable, uint32_t intervalMs = 3000);
    
protected:
    /**
     * @brief 处理接收到的消息
     * @param message 消息内容
     */
    void handleMessage(const std::string& message);
    
    /**
     * @brief 处理错误
     * @param error 错误信息
     */
    void handleError(const std::string& error);
    
    /**
     * @brief 设置错误信息
     * @param error 错误描述
     */
    void setLastError(const std::string& error);
    
    /**
     * @brief 更新连接状态
     * @param newState 新的连接状态
     */
    void setConnectionState(ConnectionState newState);
    
    // 处理器回调
    MessageHandler messageHandler_;
    ErrorHandler errorHandler_;
    ResponseHandler responseHandler_;
    
    // 线程安全保护
    mutable std::mutex handlerMutex_;
    mutable std::mutex stateMutex_;
    
    // 状态信息
    std::string lastError_;
    std::atomic<ConnectionState> connectionState_;
    
    // 自动重连配置
    std::atomic<bool> autoReconnect_;
    std::atomic<uint32_t> reconnectIntervalMs_;
};

} // namespace ais

#endif // IPC_CONNECTION_H