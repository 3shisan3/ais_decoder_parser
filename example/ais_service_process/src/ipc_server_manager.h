#ifndef IPC_SERVER_MANAGER_H
#define IPC_SERVER_MANAGER_H

#include "tcp_server_session.h" 
#include "protocol.h"
#include "ais_communication_service.h"

#include <memory>
#include <functional>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>

namespace ais
{

/**
 * @brief IPC服务器管理器类
 * 
 * 统一管理TCP IPC通信，处理客户端连接、命令和响应
 * 替代原有的AISServer类
 */
class IPCServerManager
{
public:
    using StatusCallback = std::function<protocol::ServiceStatus()>;
    using LogCallback = std::function<std::vector<std::string>()>;
    using ServiceControlCallback = std::function<bool(bool start)>;
    using LogControlCallback = std::function<bool()>;
    
    IPCServerManager();
    ~IPCServerManager();
    
    /**
     * @brief 启动IPC服务
     * @param port 监听端口
     * @param statusCallback 状态查询回调
     * @param logCallback 日志查询回调
     * @param serviceControlCallback 服务控制回调
     * @param logControlCallback 日志广播控制回调
     * @return 成功返回true
     */
    bool start(int port, 
               StatusCallback statusCallback, 
               LogCallback logCallback = nullptr,
               ServiceControlCallback serviceControlCallback = nullptr,
               LogControlCallback logControlCallback = nullptr);
    
    /**
     * @brief 停止IPC服务
     */
    void stop();
    
    /**
     * @brief 检查IPC服务是否运行
     * @return true运行中
     */
    bool isRunning() const;
    
    /**
     * @brief 获取连接数
     * @return 当前连接数
     */
    size_t getConnectionCount() const;
    
    /**
     * @brief 广播AIS数据到所有客户端
     * @param rawData 原始AIS数据
     * @param processedData 处理后的数据
     */
    void broadcastAISData(const std::string& rawData, const std::string& processedData);
    
    /**
     * @brief 设置AIS服务引用
     * @param aisService AIS服务指针
     */
    void setAISService(std::shared_ptr<AISCommunicationService> aisService);

private:
    /**
     * @brief 接受连接线程函数
     */
    void acceptThread();
    
    /**
     * @brief 初始化服务器socket
     * @param port 监听端口
     * @return 成功返回true
     */
    bool initializeServerSocket(int port);
    
    /**
     * @brief 清理服务器资源
     */
    void cleanupServer();
    
    /**
     * @brief 处理客户端消息
     * @param session TCP会话
     * @param message 接收到的消息
     */
    void handleClientMessage(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    
    /**
     * @brief 处理客户端断开连接
     * @param sessionId 会话ID
     */
    void handleClientDisconnect(const std::string& sessionId);
    
    // 命令处理函数
    void handleGetStatus(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleStartService(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleStopService(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleGetShipCount(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleConfigUpdate(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleGetMessageStats(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleHeartbeat(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    void handleChangeServiceLogs(TCPServerSessionPtr session, const protocol::CommandMessage& message);
    
    /**
     * @brief 发送响应消息
     * @param session TCP会话
     * @param status 响应状态
     * @param sequence 序列号
     * @param data 响应数据
     */
    void sendResponse(TCPServerSessionPtr session, protocol::ResponseStatus status, 
                 uint32_t sequence, const std::string& data = "{}");

private:
    int serverSocket_;                                      // 服务器socket
    std::atomic<bool> running_;                             // 运行状态
    std::thread acceptThread_;                              // 接受连接线程
    
    std::unordered_map<std::string, TCPServerSessionPtr> sessions_; // 客户端会话映射
    mutable std::mutex sessionsMutex_;                      // 会话映射互斥锁
    
    std::shared_ptr<AISCommunicationService> aisService_;   // AIS服务引用
    StatusCallback statusCallback_;                         // 状态查询回调
    LogCallback logCallback_;                               // 日志查询回调
    ServiceControlCallback serviceControlCallback_;         // 服务控制回调
    LogControlCallback logControlCallback_;                 // 日志广播控制回调
};

} // namespace ais

#endif // IPC_SERVER_MANAGER_H