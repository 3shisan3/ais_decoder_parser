/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        nmea_parser.h
Version:     1.0
Author:      cjx
start date:
Description: NMEA协议解析
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1            2025-09-25       cjx         create

*****************************************************************/

#ifndef AIS_NMEA_PARSER_H
#define AIS_NMEA_PARSER_H

#include <string>
#include <vector>

namespace ais
{

/**
 * @brief NMEA解析工具类
 * 
 * 提供NMEA语句的解析和验证功能，专门用于处理AIS相关的NMEA 0183格式数据
 */
class NMEAParser
{
public:
    /**
     * @brief 验证NMEA语句的校验和
     * @param nmea NMEA语句，格式如：$AIVDM,1,1,,A,13aG?hP000OqW@SUwmTkQj0728L,0*7A
     * @return true 校验和有效，false 校验和无效或格式错误
     */
    static bool validateChecksum(const std::string &nmea);

    /**
     * @brief 从NMEA语句中提取负载部分（6-bit ASCII编码的数据）
     * @param nmea NMEA语句
     * @return 负载字符串，如："13aG?hP000OqW@SUwmTkQj0728L"
     */
    static std::string extractPayload(const std::string &nmea);

    /**
     * @brief 解码6-bit ASCII编码的负载为二进制位字符串
     * @param payload 6-bit ASCII编码的负载
     * @return 解码后的二进制字符串，每个字符为'0'或'1'
     */
    static std::string decode6bitASCII(const std::string &payload);

    /**
     * @brief 获取NMEA语句的分片总数
     * @param nmea NMEA语句
     * @return 分片总数，单条消息返回1
     */
    static int getFragmentCount(const std::string &nmea);

    /**
     * @brief 获取NMEA语句的当前分片号
     * @param nmea NMEA语句
     * @return 当前分片号，从1开始
     */
    static int getFragmentNumber(const std::string &nmea);

    /**
     * @brief 获取NMEA语句的消息ID（AIS消息类型）
     * @param nmea NMEA语句
     * @return 消息ID字符串，如："1"表示消息类型1
     */
    static std::string getMessageId(const std::string &nmea);

private:
    /**
     * @brief 分割字符串
     * @param str 要分割的字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串向量
     */
    static std::vector<std::string> split(const std::string &str, char delimiter);

    /**
     * @brief 将十六进制字符转换为整数值
     * @param c 十六进制字符（0-9, A-F, a-f）
     * @return 对应的整数值（0-15）
     */
    static int hexCharToInt(char c);
};

} // namespace ais

#endif // AIS_NMEA_PARSER_H