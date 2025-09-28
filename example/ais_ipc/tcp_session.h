#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include "ipc_connection.h"

#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace ais
{

class TCPSession : public IPCConnection
{
public:
    /**
     * @brief 构造函数
     * @param socket 已连接的socket
     * @param sessionId 会话ID
     */
    TCPSession(int socket, const std::string& sessionId = "");
    
    /**
     * @brief 析构函数
     */
    ~TCPSession() override;
    
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
     * @brief 发送响应消息
     * @param response 响应消息
     * @param timeoutMs 超时时间（毫秒）
     * @return 成功返回true，失败返回false
     */
    bool sendResponse(const protocol::ResponseMessage& response, int timeoutMs = 5000);
    
    /**
     * @brief 获取会话ID
     * @return 会话ID
     */
    std::string getSessionId() const;
    
    /**
     * @brief 获取远程地址信息
     * @return 远程地址字符串
     */
    std::string getRemoteAddress() const;
    
private:
    /**
     * @brief 接收线程函数
     */
    void receiveThread();
    
    /**
     * @brief 发送线程函数
     */
    void sendThread();
    
    /**
     * @brief 处理接收到的数据
     * @param data 接收到的数据
     * @param size 数据大小
     */
    void processReceivedData(const char* data, size_t size);
    
    int socket_;                    // Socket文件描述符
    std::string sessionId_;         // 会话ID
    std::string remoteAddress_;     // 远程地址
    
    std::atomic<bool> running_;
    std::thread receiveThread_;
    std::thread sendThread_;
    
    std::queue<std::string> sendQueue_;
    std::mutex sendQueueMutex_;
    std::condition_variable sendQueueCV_;
    
    std::string receivePartialData_;
};

using TCPSessionPtr = std::shared_ptr<TCPSession>;

} // namespace ais

#endif // TCP_SESSION_H