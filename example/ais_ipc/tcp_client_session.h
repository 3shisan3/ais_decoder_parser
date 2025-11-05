#ifndef TCP_CLIENT_SESSION_H
#define TCP_CLIENT_SESSION_H

#include "ipc_connection.h"

#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace ais
{

/**
 * @brief TCP客户端连接类
 * 
 * 负责主动连接到TCP服务器，管理连接状态，发送命令和接收响应
 * 支持自动重连机制和异步消息处理
 */
class TCPClientSession : public IPCConnection
{
public:
    /**
     * @brief 构造函数
     * @param serverIp 服务器IP地址
     * @param serverPort 服务器端口
     */
    TCPClientSession(const std::string& serverIp, uint16_t serverPort);
    
    /**
     * @brief 析构函数
     */
    ~TCPClientSession() override;
    
    // IPCConnection接口实现
    bool start() override;
    void stop() override;
    bool isConnected() const override;
    bool sendCommand(const protocol::CommandMessage& command, int timeoutMs = 5000) override;
    bool sendCommandWithResponse(const protocol::CommandMessage& command, 
                               protocol::ResponseMessage& response, 
                               int timeoutMs = 5000) override;
    
    /**
     * @brief 发送命令消息（字符串版本）
     * @param jsonStr JSON格式的命令字符串
     * @param timeoutMs 超时时间（毫秒）
     * @return 成功返回true，失败返回false
     */
    bool sendCommand(const std::string& jsonStr, int timeoutMs = 5000);
    
    /**
     * @brief 获取服务器地址信息
     * @return 服务器地址字符串 "IP:Port"
     */
    std::string getServerAddress() const;
    
    /**
     * @brief 获取远程地址信息（兼容接口）
     * @return 远程地址字符串
     */
    std::string getRemoteAddress() const { return getServerAddress(); }
    
    /**
     * @brief 获取会话ID（兼容接口）
     * @return 会话ID
     */
    std::string getSessionId() const { return "client_" + getServerAddress(); }
    
    /**
     * @brief 设置连接超时时间
     * @param timeoutMs 超时时间（毫秒）
     */
    void setConnectTimeout(int timeoutMs);

private:
    /**
     * @brief 连接线程函数
     * 负责建立TCP连接和自动重连
     */
    void connectThread();
    
    /**
     * @brief 接收线程函数
     * 负责接收服务器发送的数据
     */
    void receiveThread();
    
    /**
     * @brief 发送线程函数
     * 负责发送数据到服务器
     */
    void sendThread();
    
    /**
     * @brief 处理接收到的数据
     * @param data 接收到的数据
     * @param size 数据大小
     */
    void processReceivedData(const char* data, size_t size);
    
    /**
     * @brief 建立TCP连接
     * @return 成功返回true，失败返回false
     */
    bool establishConnection();
    
    /**
     * @brief 关闭TCP连接
     */
    void closeConnection();
    
    // 连接配置
    std::string serverIp_;
    uint16_t serverPort_;
    int connectTimeoutMs_ = 5000;
    
    // Socket和连接状态
    int socket_ = -1;
    std::atomic<bool> running_;
    std::atomic<bool> connected_;
    
    // 线程管理
    std::thread connectThread_;
    std::thread receiveThread_;
    std::thread sendThread_;
    
    // 发送队列
    std::queue<std::string> sendQueue_;
    std::mutex sendQueueMutex_;
    std::condition_variable sendQueueCV_;
    
    // 接收缓冲区
    std::string receivePartialData_;
    mutable std::mutex connectionMutex_;
};

using TCPClientSessionPtr = std::shared_ptr<TCPClientSession>;

} // namespace ais

#endif // TCP_CLIENT_SESSION_H