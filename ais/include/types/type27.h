/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type27.h
Version:     1.0
Author:      cjx
start date:
Description: 长距离应用位置报告
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE27_H
#define AIS_TYPE27_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型27: 长距离应用位置报告
 * 
 * 用于长距离AIS应用的位置报告
 */
struct LongRangePositionReport : public AISMessage {
    bool positionAccuracy = false; // 位置精度
    bool raimFlag = false;         // RAIM标志
    int navigationStatus = 0;      // 航行状态
    double longitude = 0.0;       // 经度 (度)
    double latitude = 0.0;         // 纬度 (度)
    double speedOverGround = 0.0;  // 对地速度 (节)
    double courseOverGround = 0.0; // 对地航向 (度)
    bool gnssPositionStatus = false; // GNSS位置状态
    int spare = 0;                // 保留位
    
    LongRangePositionReport() {
        type = AISMessageType::POSITION_REPORT_LONG_RANGE;
    }
    
    std::string toJson() const override;
    std::string toCsv() const override;
    
    static std::unique_ptr<LongRangePositionReport> parse(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_TYPE27_H