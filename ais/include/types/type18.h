/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type18.h
Version:     1.0
Author:      cjx
start date:
Description: 标准Class B设备位置报告
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE18_H
#define AIS_TYPE18_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型18: 标准Class B设备位置报告
 * 
 * 来自Class B设备的简化位置报告
 */
struct StandardClassBReport : public AISMessage {
    int spare1 = 0;               // 保留位
    double speedOverGround = 0.0;  // 对地速度 (节)
    bool positionAccuracy = false; // 位置精度
    double longitude = 0.0;       // 经度 (度)
    double latitude = 0.0;         // 纬度 (度)
    double courseOverGround = 0.0; // 对地航向 (度)
    int trueHeading = 0;          // 真航向 (度)
    int timestampUTC = 0;         // UTC时间戳 (秒)
    int spare2 = 0;               // 保留位
    bool csUnit = false;          // Class B单元标志
    bool displayFlag = false;     // 显示标志
    bool dscFlag = false;         // DSC标志
    bool bandFlag = false;        // 频段标志
    bool m22Flag = false;         // 消息22标志
    bool assignedMode = false;    // 分配模式标志
    bool raimFlag = false;        // RAIM标志
    int communicationState = 0;    // 通信状态
    
    StandardClassBReport() {
        type = AISMessageType::STANDARD_CLASS_B_CS_POSITION;
    }
    
    std::string toJson() const override;
    std::string toCsv() const override;
    
    static std::unique_ptr<StandardClassBReport> parse(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_TYPE18_H