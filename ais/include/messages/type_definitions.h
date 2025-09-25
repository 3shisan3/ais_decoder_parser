/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT
File:        type_definitions.h
Version:     1.0
Author:      cjx
start date:  
Description: 27种消息类型实现
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE_DEFINITIONS_H
#define AIS_TYPE_DEFINITIONS_H

#include "message.h"

#include <string>
#include <vector>
#include <cstdint>

namespace ais
{

// 类型1：A类位置报告
struct PositionReport : public AISMessage
{
    int navigationStatus = 0;           // 导航状态 (0-15)
    int rateOfTurn = 0;                 // 转向率 (-128 to 127)
    double speedOverGround = 0.0;       // 对地速度 (节)
    bool positionAccuracy = false;      // 位置精度 (0=低, 1=高)
    double longitude = 0.0;             // 经度 (度)
    double latitude = 0.0;              // 纬度 (度)
    double courseOverGround = 0.0;      // 对地航向 (度)
    int trueHeading = 0;                // 真航向 (0-359)
    int timestampUTC = 0;               // UTC时间戳 (0-59)
    int specialManeuver = 0;            // 特殊操纵指示 (0-2)
    bool raimFlag = false;              // RAIM标志
    int communicationState = 0;         // 通信状态

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型2：A类位置报告（分配时隙）
struct PositionReportAssigned : public AISMessage
{
    int navigationStatus = 0;
    int rateOfTurn = 0;
    double speedOverGround = 0.0;
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    double courseOverGround = 0.0;
    int trueHeading = 0;
    int timestampUTC = 0;
    int specialManeuver = 0;
    bool raimFlag = false;
    int communicationState = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型3：A类位置报告（响应询问）
struct PositionReportResponse : public AISMessage
{
    int navigationStatus = 0;
    int rateOfTurn = 0;
    double speedOverGround = 0.0;
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    double courseOverGround = 0.0;
    int trueHeading = 0;
    int timestampUTC = 0;
    int specialManeuver = 0;
    bool raimFlag = false;
    int communicationState = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型4：基站报告
struct BaseStationReport : public AISMessage
{
    int year = 0;                       // 年
    int month = 0;                      // 月 (1-12)
    int day = 0;                        // 日 (1-31)
    int hour = 0;                       // 时 (0-23)
    int minute = 0;                     // 分 (0-59)
    int second = 0;                     // 秒 (0-59)
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    int epfdType = 0;                   // 定位设备类型 (1-15)
    bool raimFlag = false;
    int communicationState = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型5：静态和航程相关数据
struct StaticVoyageData : public AISMessage
{
    int aisVersion = 0;                 // AIS版本 (0-3)
    int imoNumber = 0;                  // IMO编号
    std::string callSign;               // 呼号 (7个字符)
    std::string vesselName;             // 船名 (最多20字符)
    int shipType = 0;                   // 船舶类型 (0-255)
    int dimensionToBow = 0;             // 到船首距离 (米)
    int dimensionToStern = 0;           // 到船尾距离 (米)
    int dimensionToPort = 0;            // 到左舷距离 (米)
    int dimensionToStarboard = 0;       // 到右舷距离 (米)
    int epfdType = 0;                   // 定位设备类型
    int month = 0;                      // 预计到达时间-月
    int day = 0;                        // 预计到达时间-日
    int hour = 0;                       // 预计到达时间-时
    int minute = 0;                     // 预计到达时间-分
    double draught = 0.0;               // 吃水深度 (米)
    std::string destination;            // 目的地 (最多20字符)
    bool dte = false;                   // 数据终端就绪

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型6：二进制编址消息
struct BinaryAddressedMessage : public AISMessage
{
    int sequenceNumber = 0;             // 序列号 (0-3)
    uint32_t destinationMmsi = 0;       // 目标MMSI
    bool retransmitFlag = false;        // 重传标志
    int designatedAreaCode = 0;         // 指定区域码
    int functionalId = 0;               // 功能ID
    std::vector<uint8_t> binaryData;    // 二进制数据

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型7：二进制确认
struct BinaryAcknowledge : public AISMessage
{
    int sequenceNumber = 0;
    uint32_t destinationMmsi1 = 0;
    uint32_t destinationMmsi2 = 0;
    uint32_t destinationMmsi3 = 0;
    uint32_t destinationMmsi4 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型8：二进制广播消息
struct BinaryBroadcastMessage : public AISMessage
{
    int spare = 0;
    int designatedAreaCode = 0;
    int functionalId = 0;
    std::vector<uint8_t> binaryData;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型9：标准搜救飞机位置报告
struct StandardSARAircraftReport : public AISMessage
{
    int altitude = 0;                   // 海拔高度 (米)
    double speedOverGround = 0.0;
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    double courseOverGround = 0.0;
    int timestampUTC = 0;
    int spare = 0;
    bool assignedModeFlag = false;      // 分配模式标志
    bool raimFlag = false;
    int communicationState = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型10：UTC和日期询问
struct UTCDateInquiry : public AISMessage
{
    int spare1 = 0;
    uint32_t destinationMmsi = 0;
    int spare2 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型11：UTC和日期响应
struct UTCDateResponse : public AISMessage
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    int epfdType = 0;
    int spare = 0;
    bool raimFlag = false;
    int communicationState = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型12：安全相关编址消息
struct AddressedSafetyMessage : public AISMessage
{
    int sequenceNumber = 0;             // 序列号
    uint32_t destinationMmsi = 0;       // 目标MMSI
    bool retransmitFlag = false;        // 重传标志 (添加缺失字段)
    int spare = 0;                      // 保留位
    std::string safetyText;             // 安全文本

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型13：安全相关确认
struct SafetyAcknowledge : public AISMessage
{
    int sequenceNumber = 0;
    uint32_t destinationMmsi1 = 0;
    uint32_t destinationMmsi2 = 0;
    uint32_t destinationMmsi3 = 0;
    uint32_t destinationMmsi4 = 0;
    int spare = 0;                      // 保留位

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型14：安全相关广播消息
struct SafetyRelatedBroadcast : public AISMessage
{
    int spare = 0;
    std::string safetyText;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型15：询问
struct Interrogation : public AISMessage
{
    int spare1 = 0;
    uint32_t destinationMmsi1 = 0;
    int messageType1_1 = 0;
    int slotOffset1_1 = 0;
    int spare2 = 0;
    int messageType1_2 = 0;
    int slotOffset1_2 = 0;
    int spare3 = 0;
    uint32_t destinationMmsi2 = 0;
    int messageType2 = 0;
    int slotOffset2 = 0;
    int spare4 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型16：分配模式命令
struct AssignmentModeCommand : public AISMessage
{
    int spare1 = 0;
    uint32_t destinationMmsiA = 0;
    int offsetA = 0;
    int incrementA = 0;
    int spare2 = 0;
    uint32_t destinationMmsiB = 0;
    int offsetB = 0;
    int incrementB = 0;
    int spare3 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型17：DGNSS二进制广播消息
struct DGNSSBinaryBroadcast : public AISMessage
{
    int spare1 = 0;
    double longitude = 0.0;
    double latitude = 0.0;
    int spare2 = 0;
    std::vector<uint8_t> dgnssData;     // DGNSS修正数据

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型18：标准B类设备位置报告 - 修正positionAccuracy类型
struct StandardClassBReport : public AISMessage
{
    int spare1 = 0;
    double speedOverGround = 0.0;
    bool positionAccuracy = false;      // 修正为bool类型
    double longitude = 0.0;
    double latitude = 0.0;
    double courseOverGround = 0.0;
    int trueHeading = 0;
    int timestampUTC = 0;
    int spare2 = 0;
    int csUnit = 0;                     // CS单元标志
    bool displayFlag = false;           // 显示标志
    bool dscFlag = false;               // DSC标志
    bool bandFlag = false;              // 频带标志
    bool message22Flag = false;         // 消息22标志
    bool assignedModeFlag = false;      // 分配模式标志
    bool raimFlag = false;
    int communicationState = 0;
    int spare3 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型19：扩展B类设备位置报告 - 补充缺失字段
struct ExtendedClassBReport : public AISMessage
{
    int spare1 = 0;
    double speedOverGround = 0.0;
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    double courseOverGround = 0.0;
    int trueHeading = 0;
    int timestampUTC = 0;
    int spare2 = 0;
    std::string vesselName;             // 船名
    int shipType = 0;                   // 船舶类型
    int dimensionToBow = 0;             // 到船首距离
    int dimensionToStern = 0;           // 到船尾距离
    int dimensionToPort = 0;            // 到左舷距离
    int dimensionToStarboard = 0;       // 到右舷距离
    int epfdType = 0;                   // 定位设备类型
    int spare3 = 0;
    bool raimFlag = false;
    bool dte = false;                   // 数据终端就绪
    bool assignedModeFlag = false;      // 分配模式标志
    int spare4 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型20：数据链路管理消息
struct DataLinkManagement : public AISMessage
{
    int spare1 = 0;
    int offsetNumber1 = 0;
    int reservedSlots1 = 0;
    int timeout1 = 0;
    int increment1 = 0;
    int offsetNumber2 = 0;
    int reservedSlots2 = 0;
    int timeout2 = 0;
    int increment2 = 0;
    int offsetNumber3 = 0;
    int reservedSlots3 = 0;
    int timeout3 = 0;
    int increment3 = 0;
    int offsetNumber4 = 0;
    int reservedSlots4 = 0;
    int timeout4 = 0;
    int increment4 = 0;
    int spare2 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型21：助航设备报告 - 调整字段顺序和定义
struct AidToNavigationReport : public AISMessage
{
    int aidType = 0;                    // 助航设备类型 (0-31)
    std::string name;                   // 名称 (最多20字符)
    bool positionAccuracy = false;
    double longitude = 0.0;
    double latitude = 0.0;
    int dimensionToBow = 0;             // 到前端距离
    int dimensionToStern = 0;           // 到后端距离
    int dimensionToPort = 0;            // 到左侧距离
    int dimensionToStarboard = 0;       // 到右侧距离
    int epfdType = 0;                   // 定位设备类型
    int timestampUTC = 0;               // 时间戳
    bool offPositionIndicator = false;  // 偏离位置指示器
    int regional = 0;                   // 区域保留
    bool raimFlag = false;
    bool virtualAidFlag = false;        // 虚拟助航设备标志
    bool assignedModeFlag = false;      // 分配模式标志
    std::string nameExtension;          // 名称扩展 (最多14字符)
    int spare = 0;                      // 保留位

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型22：信道管理
struct ChannelManagement : public AISMessage
{
    int spare1 = 0;
    int channelA = 0;                   // 信道A
    int channelB = 0;                   // 信道B
    int txRxMode = 0;                   // 收发模式
    int power = 0;                      // 功率等级
    double longitude1 = 0.0;            // 区域经度1
    double latitude1 = 0.0;             // 区域纬度1
    double longitude2 = 0.0;            // 区域经度2
    double latitude2 = 0.0;             // 区域纬度2
    int addressedOrBroadcast = 0;       // 编址或广播
    int bandwidthA = 0;                 // 带宽A
    int bandwidthB = 0;                 // 带宽B
    int zoneSize = 0;                   // 区域大小
    int spare2 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型23：组分配命令
struct GroupAssignmentCommand : public AISMessage
{
    int spare1 = 0;
    double longitude1 = 0.0;            // 西北角经度
    double latitude1 = 0.0;             // 西北角纬度
    double longitude2 = 0.0;            // 东南角经度
    double latitude2 = 0.0;             // 东南角纬度
    int stationType = 0;                // 台站类型
    int shipType = 0;                   // 船舶类型
    int txRxMode = 0;                   // 收发模式
    int reportingInterval = 0;          // 报告间隔
    int quietTime = 0;                  // 静默时间
    int spare2 = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型24：静态数据报告
struct StaticDataReport : public AISMessage
{
    int partNumber = 0;                 // 部分编号 (0或1)
    std::string vesselName;             // 船名 (部分A)
    int shipType = 0;                   // 船舶类型 (部分B)
    std::string vendorId;               // 供应商ID (部分B)
    std::string callSign;               // 呼号 (部分A)
    int dimensionToBow = 0;             // 到船首距离 (部分B)
    int dimensionToStern = 0;           // 到船尾距离 (部分B)
    int dimensionToPort = 0;            // 到左舷距离 (部分B)
    int dimensionToStarboard = 0;       // 到右舷距离 (部分B)
    uint32_t mothershipMmsi = 0;        // 母船MMSI (部分B)
    int spare = 0;                      // 保留位

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型25：单时隙二进制消息
struct SingleSlotBinaryMessage : public AISMessage
{
    bool addressed = false;             // 编址标志
    bool structured = false;            // 结构化标志
    uint32_t destinationMmsi = 0;       // 目标MMSI
    int designatedAreaCode = 0;         // 指定区域码
    int functionalId = 0;               // 功能ID
    std::vector<uint8_t> binaryData;    // 二进制数据
    int spare = 0;                      // 保留位

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型26：多时隙二进制消息
struct MultipleSlotBinaryMessage : public AISMessage
{
    bool addressed = false;
    bool structured = false;
    uint32_t destinationMmsi = 0;
    int designatedAreaCode = 0;
    int functionalId = 0;
    std::vector<uint8_t> binaryData;
    int commStateFlag = 0;              // 通信状态标志
    int spare = 0;                      // 保留位

    std::string toJson() const override;
    std::string toCsv() const override;
};

// 类型27：长距离位置报告
struct LongRangePositionReport : public AISMessage
{
    bool positionAccuracy = false;
    bool raimFlag = false;
    int navigationStatus = 0;
    double longitude = 0.0;             // 经度 (1/10分精度)
    double latitude = 0.0;              // 纬度 (1/10分精度)
    double speedOverGround = 0.0;       // 对地速度
    double courseOverGround = 0.0;      // 对地航向
    bool gnssPositionStatus = false;    // GNSS位置状态
    bool assignedModeFlag = false;      // 分配模式标志
    int spare = 0;

    std::string toJson() const override;
    std::string toCsv() const override;
};

} // namespace ais

#endif // AIS_TYPE_DEFINITIONS_H