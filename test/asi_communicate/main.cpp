#include "ais_communication_service.h"
#include "ais_parser.h"
#include "config.h"
#include "logger_define.h"
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> g_running{true};

void signalHandler(int signal)
{
    std::cout << "Received signal: " << signal << ", shutting down..." << std::endl;
    g_running = false;
}

int main(int argc, char* argv[])
{
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // 创建AIS解析器（外部构造）
        ais::AISParseCfg parseCfg;
        parseCfg.validateChecksum = true;
        parseCfg.enableMultipartReassembly = true;
        parseCfg.maxMultipartAge = 300;
        
        auto aisParser = std::make_shared<ais::AISParser>(parseCfg);
        
        // 创建通信配置
        ais::CommunicateCfg commCfg;
        commCfg.subPort = 10110;        // 本地监听端口
        commCfg.sendIP = "192.168.1.200"; // 目标发送IP
        commCfg.sendPort = 10111;       // 目标发送端口
        
        // 创建通信服务（依赖注入AISParser）
        ais::AISCommunicationService communicationService(aisParser);
        
        // 初始化通信服务
        int ret = communicationService.initialize(
            commCfg,                    // 通信配置
            1001                        // 任务ID（用于唯一标识发送任务）
        );
        
        if (ret != 0) {
            LOG_ERROR("Failed to initialize communication service: {}", ret);
            return 1;
        }
        
        // 启动服务
        ret = communicationService.start();
        if (ret != 0) {
            LOG_ERROR("Failed to start communication service: {}", ret);
            return 2;
        }
        
        LOG_INFO("AIS communication service started successfully");
        LOG_INFO("Task ID: {}, Current ship count: {}", 
                 communicationService.getTaskId(), communicationService.getShipCount());
        
        // 主循环
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            
            // 定期输出状态信息
            static int counter = 0;
            if (++counter % 6 == 0) { // 每30秒输出一次
                LOG_INFO("Service status - TaskID: {}, Ships tracked: {}", 
                         communicationService.getTaskId(), communicationService.getShipCount());
            }
        }
        
        // 停止服务
        communicationService.stop();
        LOG_INFO("AIS communication service stopped");
        
        // 销毁通信模块
        communicate::Destroy();
        
        return 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception in main: {}", e.what());
        return 3;
    }
}