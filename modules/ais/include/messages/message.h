/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        message.h
Version:     1.0
Author:      cjx
start date:
Description: 基类
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_MESSAGE_H
#define AIS_MESSAGE_H

#include <cstdint>
#include <memory>
#include <string>

namespace ais
{

/**
 * @brief AIS消息类型枚举
 * 
 * 完整定义了27种AIS消息类型
 */
enum class AISMessageType
{
    // Class A 位置报告 (来自Class A设备)
    POSITION_REPORT_CLASS_A = 1,            // SOTDMA位置报告
    POSITION_REPORT_CLASS_A_ASSIGNED = 2,   // SOTDMA分配时隙位置报告
    POSITION_REPORT_CLASS_A_RESPONSE = 3,   // SOTDMA轮询响应位置报告

    // 基站报告
    BASE_STATION_REPORT = 4,                // 基站UTC和位置报告

    // 静态和航程相关数据
    STATIC_VOYAGE_DATA = 5,                 // 船舶静态和航程相关数据

    // 二进制消息
    BINARY_ADDRESSED_MESSAGE = 6,           // 寻址二进制消息
    BINARY_ACKNOWLEDGE = 7,                 // 二进制确认
    BINARY_BROADCAST_MESSAGE = 8,           // 广播二进制消息

    // 标准SAR飞机报告
    STANDARD_SAR_AIRCRAFT_REPORT = 9,       // SAR飞机位置报告

    // UTC和日期查询
    UTC_DATE_INQUIRY = 10,                  // UTC/日期查询
    UTC_DATE_RESPONSE = 11,                 // UTC/日期响应

    // 安全相关消息
    ADDRESSED_SAFETY_MESSAGE = 12,          // 寻址安全消息
    SAFETY_ACKNOWLEDGE = 13,                // 安全确认
    SAFETY_RELATED_BROADCAST = 14,          // 广播安全消息

    // 询问
    INTERROGATION = 15,                     // 询问

    // 分配模式命令
    ASSIGNMENT_MODE_COMMAND = 16,           // 分配模式命令

    // DGNSS广播
    DGNSS_BINARY_BROADCAST = 17,            // DGNSS二进制广播

    // Class B 位置报告
    STANDARD_CLASS_B_CS_POSITION = 18,      // 标准Class B设备位置报告
    EXTENDED_CLASS_B_CS_POSITION = 19,      // 扩展Class B设备位置报告

    // 数据链路管理
    DATA_LINK_MANAGEMENT = 20,              // 数据链路管理消息

    // 助航设备报告
    AID_TO_NAVIGATION_REPORT = 21,          // 助航设备报告

    // 信道管理
    CHANNEL_MANAGEMENT = 22,                // 信道管理

    // 组分配命令
    GROUP_ASSIGNMENT_COMMAND = 23,          // 组分配命令

    // 静态数据报告
    STATIC_DATA_REPORT = 24,                // 静态数据报告

    // 单时隙二进制消息
    SINGLE_SLOT_BINARY_MESSAGE = 25,        // 单时隙二进制消息

    // 多时隙二进制消息
    MULTIPLE_SLOT_BINARY_MESSAGE = 26,      // 多时隙二进制消息

    // 长距离应用位置报告
    POSITION_REPORT_LONG_RANGE = 27,        // 长距离应用位置报告

    UNKNOWN = 0 // 未知消息类型
};

/**
 * @brief AIS消息基类
 * 
 * 所有具体AIS消息类型的基类，定义了公共接口和属性
 */
class AISMessage
{
public:
    AISMessageType type = AISMessageType::UNKNOWN; // 消息类型
    int repeatIndicator = 0;                       // 重复指示器
    uint32_t mmsi = 0;                             // 水上移动服务标识
    std::string rawNMEA;                           // 原始NMEA语句
    std::string timestamp;                         // 接收时间戳

    virtual ~AISMessage() = default;

    /**
     * @brief 将消息转换为JSON格式字符串
     * @return JSON格式的消息字符串
     */
    virtual std::string toJson() const;

    /**
     * @brief 将消息转换为CSV格式字符串
     * @return CSV格式的消息字符串
     */
    virtual std::string toCsv() const;

    
    static std::unique_ptr<AISMessage> parse(class BitBuffer& bits);
};

} // namespace ais

#endif // AIS_MESSAGE_H