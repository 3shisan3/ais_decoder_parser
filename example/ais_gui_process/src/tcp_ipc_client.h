// ais_client.h
#ifndef AIS_CLIENT_H
#define AIS_CLIENT_H

#include "ipc_connection.h"
#include "protocol.h"

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace ais
{

/**
 * @brief AIS客户端主类 - 直接实现TCP客户端功能
 * 
 * 负责连接到AIS服务器，发送命令和接收响应
 * 采用与TCPSession类似的线程模型，保持架构一致性
 */
class AISClient : public IPCConnection
{
public:
    using MessageHandler = std::function<void(const protocol::CommandMessage&)>;
    using ResponseHandler = std::function<void(const protocol::ResponseMessage&)>;
    using ErrorHandler = std::function<void(const std::string&)>;
    
    AISClient();
    ~AISClient();
    
    // IPCConnection接口实现
    bool start() override;
    void stop() override;
    bool isConnected() const override;
    bool sendCommand(const protocol::CommandMessage& command, int timeoutMs = 5000) override;
    bool sendCommand(const std::string& jsonStr, int timeoutMs = 5000) override;
    bool sendCommandWithResponse(const protocol::CommandMessage& command, 
                                protocol::ResponseMessage& response, 
                                int timeoutMs = 5000) override;
    
    /**
     * @brief 连接到服务器
     * @param host 服务器地址
     * @param port 服务器端口
     * @return 成功返回true，失败返回false
     */
    bool connectToServer(const std::string& host, int port = 8080);
    
    /**
     * @brief 断开连接
     */
    void disconnect();
    
    // 设置处理器回调
    void setMessageHandler(MessageHandler handler);
    void setResponseHandler(ResponseHandler handler);
    void setErrorHandler(ErrorHandler handler);
    
    /**
     * @brief 获取远程服务器地址
     * @return 服务器地址字符串
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
    
    /**
     * @brief 执行实际的socket连接
     * @param host 服务器地址
     * @param port 服务器端口
     * @return 成功返回true，失败返回false
     */
    bool establishConnection(const std::string& host, int port);

    /**
     * @brief 关闭socket连接
     */
    void closeSocket();

private:
    int socket_;                        // Socket文件描述符
    std::string remoteAddress_;         // 远程服务器地址
    std::string host_;                  // 连接的主机地址
    int port_;                          // 连接的端口号

    std::atomic<bool> running_;         // 运行状态标志
    std::atomic<bool> connected_;       // 连接状态标志
    
    std::thread receiveThread_;         // 数据接收线程
    std::thread sendThread_;            // 数据发送线程

    std::queue<std::string> sendQueue_; // 发送消息队列
    std::mutex sendQueueMutex_;         // 发送队列互斥锁
    std::condition_variable sendQueueCV_; // 发送队列条件变量

    std::string receiveBuffer_;         // 接收数据缓冲区

    // 处理器回调
    MessageHandler messageHandler_;
    ResponseHandler responseHandler_;
    ErrorHandler errorHandler_;
    mutable std::mutex handlerMutex_;
};

} // namespace ais

#endif // AIS_CLIENT_H