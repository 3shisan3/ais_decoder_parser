#ifndef AIS_SERVER_H
#define AIS_SERVER_H

#include "tcp_session.h"

#include "protocol.h"
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_map>

namespace ais
{

/**
 * @brief AIS服务端主类
 * 
 * 负责监听客户端连接，处理客户端请求，维护会话状态
 */
class AISServer
{
public:
    AISServer();
    ~AISServer();
    
    /**
     * @brief 启动服务器
     * @param port 监听端口
     * @return 成功返回true，失败返回false
     */
    bool start(int port = 8080);
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 检查服务器是否正在运行
     * @return true运行中，false已停止
     */
    bool isRunning() const;
    
    /**
     * @brief 获取当前连接数
     * @return 连接数
     */
    size_t getConnectionCount() const;
    
    /**
     * @brief 广播消息给所有客户端
     * @param message 要广播的消息
     */
    void broadcast(const protocol::CommandMessage& message);
    
    /**
     * @brief 发送消息给指定客户端
     * @param sessionId 会话ID
     * @param message 要发送的消息
     * @return 成功返回true，失败返回false
     */
    bool sendToClient(const std::string& sessionId, const protocol::CommandMessage& message);

private:
    /**
     * @brief 接受连接线程函数
     */
    void acceptThread();
    
    /**
     * @brief 处理客户端消息
     * @param session TCP会话
     * @param message 接收到的消息
     */
    void handleClientMessage(TCPSessionPtr session, const protocol::CommandMessage& message);
    
    /**
     * @brief 处理客户端断开连接
     * @param sessionId 会话ID
     */
    void handleClientDisconnect(const std::string& sessionId);
    
    int serverSocket_;
    std::atomic<bool> running_;
    std::thread acceptThread_;
    
    std::unordered_map<std::string, TCPSessionPtr> sessions_;
    mutable std::mutex sessionsMutex_;
    
    std::function<void(const protocol::CommandMessage&)> messageHandler_;
    std::function<void(const std::string&)> disconnectHandler_;
};

} // namespace ais

#endif // AIS_SERVER_H