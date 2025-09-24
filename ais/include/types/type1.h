/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type1.h
Version:     1.0
Author:      cjx
start date:
Description: 类型1-3: 位置报告
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE1_H
#define AIS_TYPE1_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型1: Class A设备位置报告
 * 
 * 包含船舶的实时位置、航速、航向等信息
 */
struct PositionReport : public AISMessage
{
    int navigationStatus = 0;      // 航行状态 (0-15)
    int rateOfTurn = 0;            // 转向率 (右转为正，左转为负)
    double speedOverGround = 0.0;  // 对地速度 (节)
    bool positionAccuracy = false; // 位置精度 (GNSS差分修正时为true)
    double longitude = 0.0;        // 经度 (度)
    double latitude = 0.0;         // 纬度 (度)
    double courseOverGround = 0.0; // 对地航向 (度)
    int trueHeading = 0;           // 真航向 (度)
    int timestampUTC = 0;          // UTC时间戳 (秒)
    int specialManeuver = 0;       // 特殊机动指示器
    bool raimFlag = false;         // RAIM标志
    int communicationState = 0;    // 通信状态

    PositionReport()
    {
        type = AISMessageType::POSITION_REPORT_CLASS_A;
    }

    std::string toJson() const override;
    std::string toCsv() const override;

    static std::unique_ptr<PositionReport> parse(BitBuffer &bits);
};

} // namespace ais

#endif // AIS_TYPE1_H