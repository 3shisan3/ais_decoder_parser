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

#include <atomic>
#include <iostream>

std::atomic<bool> g_running{true};

void signalHandler(int signal)
{
    std::cout << "接收到信号: " << signal << ", 正在停止服务..." << std::endl;
    g_running = false;
}

int main(int argc, char* argv[])
{
    // 读取启动时输入，确认是否开启IPC功能

    // 读取启动时输入，获取配置文件路径，否则读默认路径（./ais_config.yaml）

    // 初始化AIS通信服务类

    // 程序保活

    return 0;
}

