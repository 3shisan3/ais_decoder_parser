/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type4.h
Version:     1.0
Author:      cjx
start date:
Description: 基站报告
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE4_H
#define AIS_TYPE4_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型4: 基站报告
 * 
 * 包含基站的UTC时间和位置信息
 */
struct BaseStationReport : public AISMessage {
    int year = 0;                  // UTC年份
    int month = 0;                 // UTC月份 (1-12)
    int day = 0;                   // UTC日 (1-31)
    int hour = 0;                  // UTC小时 (0-23)
    int minute = 0;               // UTC分钟 (0-59)
    int second = 0;               // UTC秒 (0-59)
    bool positionAccuracy = false; // 位置精度
    double longitude = 0.0;       // 经度 (度)
    double latitude = 0.0;         // 纬度 (度)
    int epfdType = 0;             // 定位设备类型
    bool raimFlag = false;        // RAIM标志
    int spare = 0;                // 保留位
    
    BaseStationReport() {
        type = AISMessageType::BASE_STATION_REPORT;
    }
    
    std::string toJson() const override;
    std::string toCsv() const override;
    
    static std::unique_ptr<BaseStationReport> parse(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_TYPE4_H