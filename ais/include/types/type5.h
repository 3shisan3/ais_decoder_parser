/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        type5.h
Version:     1.0
Author:      cjx
start date:
Description: 静态和航程数据 
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_TYPE5_H
#define AIS_TYPE5_H

#include "message.h"
#include "core/bit_buffer.h"

namespace ais {

/**
 * @brief AIS消息类型5: 静态和航程相关数据
 * 
 * 包含船舶的静态信息和航程相关数据
 */
struct StaticVoyageData : public AISMessage {
    int aisVersion = 0;            // AIS版本指示器
    int imoNumber = 0;             // IMO编号
    std::string callSign;          // 呼号
    std::string vesselName;        // 船名
    int shipType = 0;              // 船舶类型
    int dimensionToBow = 0;        // 船首距离 (米)
    int dimensionToStern = 0;      // 船尾距离 (米)
    int dimensionToPort = 0;       // 左舷距离 (米)
    int dimensionToStarboard = 0;  // 右舷距离 (米)
    int epfdType = 0;             // 定位设备类型
    int etaMonth = 0;             // 预计到达时间-月
    int etaDay = 0;               // 预计到达时间-日
    int etaHour = 0;              // 预计到达时间-时
    int etaMinute = 0;            // 预计到达时间-分
    double draught = 0.0;         // 最大吃水深度 (米)
    std::string destination;      // 目的地
    bool dte = false;             // 数据终端就绪标志
    int spare = 0;                // 保留位
    
    StaticVoyageData() {
        type = AISMessageType::STATIC_VOYAGE_DATA;
    }
    
    std::string toJson() const override;
    std::string toCsv() const override;
    
    static std::unique_ptr<StaticVoyageData> parse(BitBuffer& bits);
};

} // namespace ais

#endif // AIS_TYPE5_H