#ifndef AIS_NMEA_PARSER_H
#define AIS_NMEA_PARSER_H

#include <string>
#include <vector>

namespace ais
{

/**
 * @brief NMEA解析工具类
 * 
 * 提供NMEA语句的解析和验证功能
 */
class NMEAParser
{
public:
    /**
     * @brief 验证NMEA语句的校验和
     * @param nmea NMEA语句
     * @return 校验和是否有效
     */
    static bool validateChecksum(const std::string &nmea);

    /**
     * @brief 从NMEA语句中提取负载部分
     * @param nmea NMEA语句
     * @return 负载字符串
     */
    static std::string extractPayload(const std::string &nmea);

    /**
     * @brief 获取NMEA语句的分片总数
     * @param nmea NMEA语句
     * @return 分片总数
     */
    static int getFragmentCount(const std::string &nmea);

    /**
     * @brief 获取NMEA语句的当前分片号
     * @param nmea NMEA语句
     * @return 当前分片号
     */
    static int getFragmentNumber(const std::string &nmea);

    /**
     * @brief 获取NMEA语句的消息ID
     * @param nmea NMEA语句
     * @return 消息ID字符串
     */
    static std::string getMessageId(const std::string &nmea);

    /**
     * @brief 解码6-bit ASCII编码的负载
     * @param payload 6-bit ASCII编码的负载
     * @return 解码后的二进制字符串
     */
    static std::string decode6bitASCII(const std::string &payload);

private:
    /**
     * @brief 分割字符串
     * @param s 要分割的字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串向量
     */
    static std::vector<std::string> split(const std::string &s, char delimiter);
};

} // namespace ais

#endif // AIS_NMEA_PARSER_H