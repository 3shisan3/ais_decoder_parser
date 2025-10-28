/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        main_service.cpp
Version:     1.0
Author:      cjx
start date:
Description: 服务进程入口
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-28      cjx        create
2             2025-9-29      cjx        refactor with IPCServerManager

*****************************************************************/

#include "ais_communication_service.h"
#include "config_manager.h"
#include "ipc_server_manager.h"
#include "logger_define.h"

#include <atomic>
#include <iostream>
#include <csignal>
#include <thread>
#include <memory>
#include <vector>
#include <mutex>

#if _WIN32
#include <windows.h>
#endif

// 全局运行状态
std::atomic<bool> g_running{true};

// 全局服务实例
std::shared_ptr<ais::AISCommunicationService> g_aisService;
std::shared_ptr<ais::IPCServerManager> g_ipcManager;
std::shared_ptr<ais::AISParser> g_aisParser;

// 服务状态统计
std::atomic<uint64_t> g_messagesReceived{0};
std::atomic<uint64_t> g_messagesProcessed{0};
std::atomic<uint64_t> g_messagesSent{0};
std::atomic<uint64_t> g_lastMessageTime{0};
std::chrono::steady_clock::time_point g_startTime;

// 服务控制状态
std::atomic<bool> g_serviceRunning{true};        // AIS服务是否处理数据
std::atomic<bool> g_enableLogBroadcast{false};   // 是否启用日志广播

/**
 * @brief 信号处理函数
 * @param signal 接收到的信号
 */
void signalHandler(int signal)
{
    std::cout << "接收到信号: " << signal << ", 正在停止服务..." << std::endl;
    g_running = false;
}

/**
 * @brief 获取服务状态的回调函数
 * @return 服务状态信息
 */
ais::protocol::ServiceStatus getServiceStatus()
{
    ais::protocol::ServiceStatus status;
    
    auto currentTime = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - g_startTime).count();
    
    status.isRunning = g_serviceRunning.load();  // 使用服务运行状态控制
    status.shipCount = g_aisService ? g_aisService->getShipCount() : 0;
    status.uptime = static_cast<uint64_t>(uptime);
    status.messagesReceived = g_messagesReceived.load();
    status.messagesSent = g_messagesSent.load();
    status.messagesProcessed = g_messagesProcessed.load();
    status.lastMessageTime = g_lastMessageTime.load();
    
    return status;
}

/**
 * @brief 服务日志控制回调函数
 * @return 当前日志广播是否启用
 */
bool serviceLogControlCallback()
{
    bool oldValue = g_enableLogBroadcast.load();
    
    if (oldValue) {
        std::cout << "AIS数据广播已启用" << std::endl;
        LOG_INFO("AIS data broadcast enabled via IPC");
    } else{
        std::cout << "AIS数据广播已禁用" << std::endl;
        LOG_INFO("AIS data broadcast disabled via IPC");
    }

    return !g_enableLogBroadcast.exchange(!oldValue);
}

/**
 * @brief 服务控制回调函数
 * @param start true启动服务，false停止服务
 * @return 操作是否成功
 */
bool serviceControlCallback(bool start)
{
    bool oldValue = g_serviceRunning.exchange(start);
    
    if (start && !oldValue) {
        std::cout << "AIS数据处理已启动" << std::endl;
        LOG_INFO("AIS data processing started via IPC");
    } else if (!start && oldValue) {
        std::cout << "AIS数据处理已停止" << std::endl;
        LOG_INFO("AIS data processing stopped via IPC");
    }
    
    return true;
}

/**
 * @brief 获取当前服务日志广播状态
 * @return 日志广播状态信息
 */
std::vector<std::string> getServiceLogsStatus()
{
    std::vector<std::string> logs;
    
    // 返回服务状态信息
    logs.push_back("=== AIS通信服务状态 ===");
    logs.push_back("服务运行状态: " + std::string(g_serviceRunning ? "运行中" : "已停止"));
    logs.push_back("数据广播状态: " + std::string(g_enableLogBroadcast ? "启用" : "禁用"));
    logs.push_back("船舶跟踪数量: " + std::to_string(g_aisService ? g_aisService->getShipCount() : 0));
    logs.push_back("消息接收总数: " + std::to_string(g_messagesReceived.load()));
    logs.push_back("消息处理总数: " + std::to_string(g_messagesProcessed.load()));
    logs.push_back("消息发送总数: " + std::to_string(g_messagesSent.load()));
    
    auto currentTime = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - g_startTime).count();
    logs.push_back("服务运行时间: " + std::to_string(uptime) + "秒");
    
    if (g_ipcManager) {
        logs.push_back("IPC连接数量: " + std::to_string(g_ipcManager->getConnectionCount()));
    }
    
    // 添加时间戳
    std::time_t now = std::time(nullptr);
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "状态更新时间: %Y-%m-%d %H:%M:%S", std::localtime(&now));
    logs.push_back(timeStr);
    
    return logs;
}

/**
 * @brief 增强的AIS通信服务类，支持IPC数据转发和服务控制
 */
class EnhancedAISCommunicationService : public ais::AISCommunicationService
{
public:
    EnhancedAISCommunicationService(std::shared_ptr<ais::AISParser> aisParser, 
                                   bool enableIPC)
        : ais::AISCommunicationService(aisParser), enableIPC_(enableIPC)
    {
    }
    
    /**
     * @brief 处理接收到的AIS消息
     * @param msg 接收到的消息
     * @return 错误码，0表示成功
     */
    virtual int handleMsg(std::shared_ptr<void> msg) override
    {
        // 统计接收到的消息数量
        g_messagesReceived++;
        g_lastMessageTime = std::time(nullptr);
        
        // 检查服务是否运行，如果不运行则直接返回成功但不处理
        if (!g_serviceRunning.load()) {
            LOG_DEBUG("AIS service is stopped, message received but not processed");
            return 0; // 返回成功但不实际处理
        }
        
        if (!enableIPC_) {
            // 不启用IPC，直接调用父类处理
            int result = ais::AISCommunicationService::handleMsg(msg);
            if (result == 0) {
                g_messagesProcessed++;
            }
            return result;
        }
        
        try {
            const char* aisData = static_cast<const char*>(msg.get());
            if (!aisData) {
                return -1;
            }
            
            std::string rawData = aisData;
            
            // 调用父类处理AIS消息
            int result = ais::AISCommunicationService::handleMsg(msg);
            
            if (result == 0) {
                g_messagesProcessed++;
                
                // 检查是否启用数据广播
                if (g_enableLogBroadcast.load()) {
                    // 准备转发处理过的数据
                    std::string processedData = "Processed: " + rawData + "\n";
                    
                    // 通过IPC管理器广播数据
                    if (g_ipcManager && g_ipcManager->isRunning()) {
                        g_ipcManager->broadcastAISData(rawData, processedData);
                        g_messagesSent++;
                        
                        LOG_DEBUG("AIS data broadcasted to IPC clients");
                    }
                }
            }
            
            return result;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in enhanced handleMsg: {}", e.what());
            return -1;
        }
    }
    
    /**
     * @brief 获取服务运行状态
     * @return true表示正在处理数据，false表示已停止
     */
    bool isProcessingData() const {
        return g_serviceRunning.load();
    }
    
    /**
     * @brief 获取数据广播状态
     * @return true表示启用广播，false表示禁用
     */
    bool isBroadcastingData() const {
        return g_enableLogBroadcast.load();
    }
    
private:
    bool enableIPC_;
};

/**
 * @brief 主函数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 程序退出码
 */
int main(int argc, char* argv[])
{
#if _WIN32
    // 设置命令行使用utf-8编码
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 记录启动时间
    g_startTime = std::chrono::steady_clock::now();

    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 解析命令行参数
    bool enableIPC = true;
    std::string configPath = "ais_config.yaml";
    int ipcPort = 2333; // 默认IPC端口
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--no-ipc") {
            enableIPC = false;
            std::cout << "IPC功能已禁用" << std::endl;
        } else if (arg == "--config" || arg == "-c") {
            if (i + 1 < argc) {
                configPath = argv[++i];
                std::cout << "使用配置文件: " << configPath << std::endl;
            }
        } else if (arg == "--ipc-port") {
            if (i + 1 < argc) {
                ipcPort = std::stoi(argv[++i]);
                std::cout << "IPC端口: " << ipcPort << std::endl;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --no-ipc          禁用IPC功能" << std::endl;
            std::cout << "  --config, -c      指定配置文件路径" << std::endl;
            std::cout << "  --ipc-port        指定IPC服务器端口" << std::endl;
            std::cout << "  --help, -h        显示帮助信息" << std::endl;
            return 0;
        }
    }
    
    // 初始化配置管理器
    ais::ConfigManager configManager(configPath);
    if (!configManager.loadConfig()) {
        std::cerr << "无法加载配置文件: " << configPath << std::endl;
        return 1;
    }
    
    // 获取配置
    auto loggerCfg = configManager.getLoggerConfig();
    auto parseCfg = configManager.getParserConfig();
    auto saveCfg = configManager.getSaveConfig();
    auto commCfgOpt = configManager.getCommunicateConfig();
    auto udptcpLibCfg = configManager.getUdpTcpCommunicateCfgPath();
    
    if (!commCfgOpt) {
        std::cerr << "配置文件中缺少通信配置" << std::endl;
        return 1;
    }
    
    ais::CommunicateCfg commCfg = commCfgOpt.value();
    
    // 初始化AIS解析器
    g_aisParser = std::make_shared<ais::AISParser>(parseCfg);
    
    // 初始化AIS通信服务
    if (enableIPC) {
        g_aisService = std::make_shared<EnhancedAISCommunicationService>(g_aisParser, true);
        std::cout << "增强版AIS通信服务已初始化（支持IPC）" << std::endl;
    } else {
        g_aisService = std::make_shared<ais::AISCommunicationService>(g_aisParser);
        std::cout << "标准版AIS通信服务已初始化" << std::endl;
    }
    
    // 初始化AIS通信服务
    int initResult = g_aisService->initialize(commCfg, udptcpLibCfg);
    if (initResult != 0) {
        std::cerr << "AIS通信服务初始化失败: " << initResult << std::endl;
        return 1;
    }
    
    // 如果启用IPC，启动IPC管理器
    if (enableIPC) {
        g_ipcManager = std::make_shared<ais::IPCServerManager>();
        g_ipcManager->setAISService(g_aisService);
        
        // 启动IPC管理器，传入相应的回调函数
        if (!g_ipcManager->start(ipcPort, getServiceStatus,
                getServiceLogsStatus, serviceControlCallback, serviceLogControlCallback)) {
            std::cerr << "IPC管理器启动失败" << std::endl;
            // 继续运行主服务，只是IPC功能不可用
        } else {
            std::cout << "IPC管理器已启动，监听端口: " << ipcPort << std::endl;
            std::cout << "支持的命令类型:" << std::endl;
            std::cout << "  GET_STATUS          - 获取服务状态" << std::endl;
            std::cout << "  START_SERVICE       - 启动AIS数据处理" << std::endl;
            std::cout << "  STOP_SERVICE        - 停止AIS数据处理" << std::endl;
            std::cout << "  GET_SHIP_COUNT      - 获取船舶数量" << std::endl;
            std::cout << "  CONFIG_UPDATE       - 更新配置" << std::endl;
            std::cout << "  GET_MESSAGE_STATS   - 获取消息统计" << std::endl;
            std::cout << "  HEARTBEAT           - 心跳检测" << std::endl;
            std::cout << "  CHANGE_SERVICE_LOGS - 修改服务状态和启用/禁用数据广播" << std::endl;
        }
    }
    
    std::cout << "AIS通信服务已成功启动" << std::endl;
    std::cout << "监听端口: " << commCfg.subPort << std::endl;
    std::cout << "目标地址: " << commCfg.sendIP << ":" << commCfg.sendPort << std::endl;
    std::cout << "初始状态: AIS数据处理=" << (g_serviceRunning ? "运行中" : "已停止") 
              << ", 数据广播=" << (g_enableLogBroadcast ? "启用" : "禁用") << std::endl;
    std::cout << "按Ctrl+C停止服务..." << std::endl;
    
    // 程序保活主循环
    while (g_running)
    {
        auto currentTime = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - g_startTime).count();
        
        // 获取当前状态
        size_t shipCount = g_aisService->getShipCount();
        size_t connectionCount = enableIPC && g_ipcManager ? g_ipcManager->getConnectionCount() : 0;

        // 只有数据变动时才打印
        static size_t lastShipCount = -1;
        static size_t lastConnectionCount = -1;
        static long long lastUptime = -1;
        static bool lastServiceRunning = true;
        static bool lastLogBroadcast = false;

        if (shipCount != lastShipCount || connectionCount != lastConnectionCount || 
            uptime != lastUptime || g_serviceRunning != lastServiceRunning || 
            g_enableLogBroadcast != lastLogBroadcast)
        {
            std::cout << "运行时间: " << uptime << "s, 船舶数量: " << shipCount
                      << ", IPC连接数: " << connectionCount
                      << ", 处理状态: " << (g_serviceRunning ? "运行" : "停止")
                      << ", 广播状态: " << (g_enableLogBroadcast ? "启用" : "禁用")
                      << ", 消息统计: R" << g_messagesReceived.load() 
                      << "/P" << g_messagesProcessed.load() 
                      << "/S" << g_messagesSent.load()
                      << std::endl;

            lastShipCount = shipCount;
            lastConnectionCount = connectionCount;
            lastUptime = uptime;
            lastServiceRunning = g_serviceRunning;
            lastLogBroadcast = g_enableLogBroadcast;
        }

        // 休眠1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 清理资源
    std::cout << std::endl << "正在停止服务..." << std::endl;
    
    if (g_ipcManager) {
        g_ipcManager->stop();
        std::cout << "IPC管理器已停止" << std::endl;
    }
    
    // 清空船舶信息
    if (g_aisService) {
        g_aisService->clearShipInfo();
        std::cout << "船舶信息已清空" << std::endl;
    }
    
    std::cout << "服务已停止" << std::endl;
    return 0;
}