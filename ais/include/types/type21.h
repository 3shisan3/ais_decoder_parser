/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type21.h
Version:     1.0
Author:      cjx
start date:
Description: 助航设备报告
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE21_H
#define AIS_TYPE21_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型21: 助航设备报告
 * 
 * 包含助航设备的位置和状态信息
 */
struct AidToNavigationReport : public AISMessage {
    int aidType = 0;              // 助航设备类型
    std::string name;             // 助航设备名称
    bool positionAccuracy = false; // 位置精度
    double longitude = 0.0;       // 经度 (度)
    double latitude = 0.0;         // 纬度 (度)
    int dimensionToBow = 0;        // 船首距离 (米)
    int dimensionToStern = 0;     // 船尾距离 (米)
    int dimensionToPort = 0;      // 左舷距离 (米)
    int dimensionToStarboard = 0; // 右舷距离 (米)
    int epfdType = 0;             // 定位设备类型
    int timestampUTC = 0;         // UTC时间戳 (秒)
    bool offPosition = false;     // 离位标志
    bool raimFlag = false;        // RAIM标志
    bool virtualAid = false;       // 虚拟助航设备标志
    bool assignedMode = false;     // 分配模式标志
    int spare = 0;               // 保留位
    
    AidToNavigationReport() {
        type = AISMessageType::AID_TO_NAVIGATION_REPORT;
    }
    
    std::string toJson() const override;
    std::string toCsv() const override;
    
    static std::unique_ptr<AidToNavigationReport> parse(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_TYPE21_H