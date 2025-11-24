#ifndef AIS_NMEA_ENCODER_H
#define AIS_NMEA_ENCODER_H

#include <string>

namespace ais
{

/**
 * @brief NMEA消息类型枚举
 */
enum class NMEAMessageType
{
    AIVDM, // 来自他船的数据
    AIVDO  // 来自本船的数据
};

/**
 * @brief NMEA编码器类
 * 用于生成完整的NMEA语句
 */
class NMEAEncoder
{
public:
    /**
     * @brief 生成AIS NMEA语句
     * @param messageType NMEA消息类型 (AIVDM/AIVDO)
     * @param payload 6-bit ASCII负载
     * @param fragmentCount 分片总数（默认为1）
     * @param fragmentNumber 当前分片号（默认为1）
     * @param sequenceId 序列ID（默认为空）
     * @param channel 信道（默认为'A'）
     * @param fillBits 填充位数（默认为0）
     * @return 完整的NMEA语句
     */
    static std::string encodeAIS(
        NMEAMessageType messageType,
        const std::string &payload,
        int fragmentCount = 1,
        int fragmentNumber = 1,
        const std::string &sequenceId = "",
        char channel = 'A',
        int fillBits = 0);

    /**
     * @brief 计算NMEA校验和
     * @param data 数据部分（不包含$和*）
     * @return 2字符的十六进制校验和
     */
    static std::string calculateChecksum(const std::string &data);

    /**
     * @brief 获取NMEA消息类型字符串
     */
    static std::string getMessageTypeString(NMEAMessageType type);

private:
    /**
     * @brief 转义NMEA字段中的特殊字符
     */
    static std::string escapeField(const std::string &field);
};

} // namespace ais

#endif // AIS_NMEA_ENCODER_H