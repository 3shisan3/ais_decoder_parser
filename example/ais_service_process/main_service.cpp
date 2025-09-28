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

*****************************************************************/

#include "ais_communication_service.h"
#include "config_manager.h"
#include "tcp_ipc_server.h"
#include "logger_define.h"

#include <atomic>
#include <iostream>
#include <csignal>
#include <thread>
#include <memory>

std::atomic<bool> g_running{true};

// 全局变量用于存储服务实例
std::shared_ptr<ais::AISCommunicationService> g_aisService;
std::shared_ptr<ais::AISServer> g_ipcServer;
std::shared_ptr<ais::AISParser> g_aisParser;

void signalHandler(int signal)
{
    std::cout << "接收到信号: " << signal << ", 正在停止服务..." << std::endl;
    g_running = false;
}

// IPC消息处理器 - 将AIS数据转发给连接的进程
void handleIPCDataForwarding(const std::string& rawData, const std::string& processedData)
{
    if (g_ipcServer && g_ipcServer->isRunning()) {
        // 创建包含原始数据和处理后数据的消息
        ais::protocol::CommandMessage msg;
        msg.type = ais::protocol::CommandType::SEND_MESSAGE;
        msg.sequence = 0;
        msg.data = R"({"raw_data":")" + rawData + R"(", "processed_data":")" + processedData + R"("})";
        
        // 广播给所有连接的客户端
        g_ipcServer->broadcast(msg);
    }
}

// 增强的AIS通信服务类，支持IPC数据转发
class EnhancedAISCommunicationService : public ais::AISCommunicationService
{
public:
    EnhancedAISCommunicationService(std::shared_ptr<ais::AISParser> aisParser, 
                                   bool enableIPC)
        : ais::AISCommunicationService(aisParser), enableIPC_(enableIPC)
    {
    }
    
    virtual int handleMsg(std::shared_ptr<void> msg) override
    {
        if (!enableIPC_) {
            return ais::AISCommunicationService::handleMsg(msg);
        }
        
        try {
            // 保存原始数据用于IPC转发
            std::string* aisData = static_cast<std::string*>(msg.get());
            if (!aisData || aisData->empty()) {
                return 0;
            }
            
            std::string rawData = *aisData;
            
            // 调用父类处理
            int result = ais::AISCommunicationService::handleMsg(msg);
            
            // 如果处理成功，准备转发处理后的数据
            if (result == 0) {
                // 这里简化处理，实际应该从shipInfoCache_中获取最新处理的数据
                std::string processedData = "Processed: " + rawData.substr(0, 50) + "...";
                handleIPCDataForwarding(rawData, processedData);
            }
            
            return result;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Exception in enhanced handleMsg: {}", e.what());
            return -1;
        }
    }
    
private:
    bool enableIPC_;
};

int main(int argc, char* argv[])
{
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 解析命令行参数
    bool enableIPC = true;
    std::string configPath = "ais_config.yaml";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--ipc" || arg == "-i") {
            enableIPC = true;
            std::cout << "IPC功能已启用" << std::endl;
        } else if (arg == "--config" || arg == "-c") {
            if (i + 1 < argc) {
                configPath = argv[++i];
                std::cout << "使用配置文件: " << configPath << std::endl;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --ipc, -i       启用IPC功能" << std::endl;
            std::cout << "  --config, -c    指定配置文件路径" << std::endl;
            std::cout << "  --help, -h      显示帮助信息" << std::endl;
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
    
    // 初始化AIS解析器（需要根据实际情况实现）
    g_aisParser = std::make_shared<ais::AISParser>(parseCfg);
    
    // 初始化AIS通信服务
    if (enableIPC) {
        g_aisService = std::make_shared<EnhancedAISCommunicationService>(g_aisParser, true);
        std::cout << "增强版AIS通信服务已初始化（支持IPC）" << std::endl;
    } else {
        g_aisService = std::make_shared<ais::AISCommunicationService>(g_aisParser);
        std::cout << "标准版AIS通信服务已初始化" << std::endl;
    }
    
    // 初始化服务
    int initResult = g_aisService->initialize(commCfg, udptcpLibCfg);
    if (initResult != 0) {
        std::cerr << "AIS通信服务初始化失败: " << initResult << std::endl;
        return 1;
    }
    
    // 如果启用IPC，启动IPC服务器
    if (enableIPC) {
        g_ipcServer = std::make_shared<ais::AISServer>();
        int ipcPort = 2333; // 默认IPC端口，可以从配置中读取
        
        if (!g_ipcServer->start(ipcPort)) {
            std::cerr << "IPC服务器启动失败" << std::endl;
            // 继续运行主服务，只是IPC功能不可用
        } else {
            std::cout << "IPC服务器已启动，监听端口: " << ipcPort << std::endl;
        }
    }
    
    std::cout << "AIS通信服务已成功启动" << std::endl;
    std::cout << "监听端口: " << commCfg.subPort << std::endl;
    std::cout << "目标地址: " << commCfg.sendIP << ":" << commCfg.sendPort << std::endl;
    std::cout << "按Ctrl+C停止服务..." << std::endl;
    
    // 程序保活主循环
    while (g_running)
    {
        // 获取当前状态
        size_t shipCount = g_aisService->getShipCount();
        size_t connectionCount = enableIPC && g_ipcServer ? g_ipcServer->getConnectionCount() : 0;

        // 只有数据变动时才打印
        static size_t lastShipCount = -1;
        static size_t lastConnectionCount = -1;

        if (shipCount != lastShipCount || connectionCount != lastConnectionCount)
        {
            std::cout << "当前状态: 船舶数量=" << shipCount
                      << ", IPC连接数=" << connectionCount
                      << std::endl;

            lastShipCount = shipCount;
            lastConnectionCount = connectionCount;
        }

        // 休眠1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 清理资源
    std::cout << std::endl << "正在停止服务..." << std::endl;
    
    if (g_ipcServer) {
        g_ipcServer->stop();
    }
    
    // 清空船舶信息
    g_aisService->clearShipInfo();
    
    std::cout << "服务已停止" << std::endl;
    return 0;
}