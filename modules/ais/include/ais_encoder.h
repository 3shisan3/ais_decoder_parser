#ifndef AIS_ENCODER_H
#define AIS_ENCODER_H

#include "config.h"
#include "core/nmea_encoder.h"
#include "messages/message.h"

#include <memory>
#include <vector>
#include <string>

namespace ais
{

/**
 * @brief AIS主编码器类
 * 提供完整的AIS消息编码功能，将AIS消息对象编码为NMEA语句
 */
class AISEncoder
{
public:
    /**
     * @brief 构造函数
     * @param cfg 编码配置
     */
    explicit AISEncoder(const AISGenerateCfg &cfg = AISGenerateCfg());

    /**
     * @brief 编码单个AIS消息
     * @param message AIS消息对象
     * @param messageType NMEA消息类型 (AIVDM/AIVDO)
     * @return NMEA语句向量（可能分片为多条语句）
     */
    std::vector<std::string> encode(const AISMessage &message,
                                    NMEAMessageType messageType = NMEAMessageType::AIVDM);

    /**
     * @brief 批量编码AIS消息
     * @param messages AIS消息对象向量
     * @param messageType NMEA消息类型 (AIVDM/AIVDO)
     * @return NMEA语句向量
     */
    std::vector<std::string> encodeBatch(const std::vector<std::unique_ptr<AISMessage>> &messages,
                                         NMEAMessageType messageType = NMEAMessageType::AIVDM);

    /**
     * @brief 设置新配置
     * @param newConfig 新配置
     */
    void setConfig(const AISGenerateCfg &newConfig);

    /**
     * @brief 获取当前配置
     * @return 当前配置的引用
     */
    const AISGenerateCfg &getConfig() const;

private:
    AISGenerateCfg config_; // 编码器配置

    /**
     * @brief 分片处理
     * @param binaryData 二进制数据
     * @param messageType NMEA消息类型
     * @param aisMessageType AIS消息类型
     * @return 分片后的NMEA语句
     */
    std::vector<std::string> fragmentMessage(const std::string &binaryData,
                                             NMEAMessageType messageType,
                                             AISMessageType aisMessageType);

    /**
     * @brief 计算填充位数
     * @param binaryLength 二进制数据长度
     * @return 填充位数
     */
    int calculateFillBits(size_t binaryLength) const;
};

} // namespace ais

#endif // AIS_ENCODER_H