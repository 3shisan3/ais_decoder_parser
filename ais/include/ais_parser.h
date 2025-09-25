/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT
File:        ais_parser.h
Version:     1.0
Author:      cjx
start date:
Description: 主解析器
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1             2025-9-24      cjx        create

*****************************************************************/

#ifndef AIS_PARSER_H
#define AIS_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include "config.h"

#include "messages/message.h"

namespace ais {

/**
 * @brief AIS主解析器类
 * 
 * 提供完整的AIS消息解析功能，仅负责解析不涉及存储和日志
 */
class AISParser {
public:
    /**
     * @brief 构造函数
     * @param cfg 配置
     */
    explicit AISParser(const Config& cfg = Config());
    
    /**
     * @brief 解析单个NMEA语句
     * @param nmea NMEA语句
     * @return 解析后的AIS消息
     */
    std::unique_ptr<AISMessage> parse(const std::string& nmea) const;
    
    /**
     * @brief 批量解析NMEA语句
     * @param nmeaSentences NMEA语句向量
     * @return 解析后的AIS消息向量
     */
    std::vector<std::unique_ptr<AISMessage>> parseBatch(const std::vector<std::string>& nmeaSentences) const;
    
    /**
     * @brief 设置新配置
     * @param newConfig 新配置
     */
    void setConfig(const Config& newConfig);
    
    /**
     * @brief 获取当前配置
     * @return 当前配置的引用
     */
    const Config& getConfig() const;

private:
    Config config_; // 解析器配置
    
    /**
     * @brief 解析二进制负载
     * @param binary 二进制负载
     * @return 解析后的AIS消息
     */
    std::unique_ptr<AISMessage> parseBinary(const std::string& binary) const;
};

} // namespace ais

#endif // AIS_PARSER_H