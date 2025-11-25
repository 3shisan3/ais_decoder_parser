#ifndef AIS_MESSAGE_GENERATOR_H
#define AIS_MESSAGE_GENERATOR_H

#include <string>
#include <vector>
#include <cstdint>
#include <QGeoCoordinate>
#include <QString>

/**
 * @brief AIS消息类型枚举
 */
enum class AISMessageType
{
    TYPE_1_POSITION_REPORT = 1,      // A类位置报告
    TYPE_5_STATIC_VOYAGE = 5,        // 静态和航程数据
    TYPE_18_CLASS_B_POSITION = 18,   // B类位置报告
    TYPE_19_EXTENDED_CLASS_B = 19,   // 扩展B类位置报告
    TYPE_24_STATIC_DATA = 24         // 静态数据报告
};

/**
 * @brief AIS船舶基础信息
 */
struct AISVesselData
{
    int mmsi = 0;                       // 海上移动业务标识
    int imo = 0;                        // IMO编号
    QString callSign;                   // 呼号
    QString vesselName;                 // 船名
    int shipType = 0;                   // 船舶类型
    int length = 0;                     // 总长度（米）
    int width = 0;                      // 宽度（米）
    double draft = 0.0;                 // 吃水深度（米）
    
    // 动态信息
    QGeoCoordinate position;            // 位置
    double speed = 0.0;                 // 对地速度（节）
    double heading = 0.0;               // 真航向（度）
    double courseOverGround = 0.0;      // 对地航向（度）
    int navigationStatus = 0;           // 导航状态
    int rateOfTurn = 0;                 // 转向率
    
    // 航程信息
    QString destination;                // 目的地
    int etaMonth = 0;                   // 预计到达时间-月
    int etaDay = 0;                     // 预计到达时间-日
    int etaHour = 0;                    // 预计到达时间-时
    int etaMinute = 0;                  // 预计到达时间-分
};

/**
 * @brief AIS消息生成器类
 * 负责生成符合ITU-R M.1371标准的AIS NMEA消息
 */
class AISMessageGenerator
{
public:
    AISMessageGenerator();
    ~AISMessageGenerator() = default;

    /**
     * @brief 生成Type 1位置报告（A类设备）
     * @param vesselData 船舶数据
     * @return NMEA格式的AIS消息
     */
    std::string generateType1Message(const AISVesselData& vesselData);

    /**
     * @brief 生成Type 5静态和航程数据
     * @param vesselData 船舶数据
     * @return NMEA格式的AIS消息
     */
    std::string generateType5Message(const AISVesselData& vesselData);

    /**
     * @brief 生成Type 18标准B类位置报告
     * @param vesselData 船舶数据
     * @return NMEA格式的AIS消息
     */
    std::string generateType18Message(const AISVesselData& vesselData);

    /**
     * @brief 生成Type 19扩展B类位置报告
     * @param vesselData 船舶数据
     * @return NMEA格式的AIS消息
     */
    std::string generateType19Message(const AISVesselData& vesselData);

    /**
     * @brief 生成Type 24静态数据报告
     * @param vesselData 船舶数据
     * @param partNumber 部分号（0=A部分，1=B部分）
     * @return NMEA格式的AIS消息
     */
    std::string generateType24Message(const AISVesselData& vesselData, int partNumber);

private:
    /**
     * @brief 添加指定位数的二进制数据
     * @param bits 二进制向量
     * @param value 数值
     * @param bitCount 位数
     */
    void appendBits(std::vector<bool>& bits, uint32_t value, size_t bitCount);

    /**
     * @brief 添加有符号整数（使用补码）
     * @param bits 二进制向量
     * @param value 有符号数值
     * @param bitCount 位数
     */
    void appendSignedBits(std::vector<bool>& bits, int32_t value, size_t bitCount);

    /**
     * @brief 添加布尔值
     * @param bits 二进制向量
     * @param value 布尔值
     */
    void appendBool(std::vector<bool>& bits, bool value);

    /**
     * @brief 添加字符串（6-bit ASCII编码）
     * @param bits 二进制向量
     * @param text 文本
     * @param maxBits 最大位数
     */
    void appendString(std::vector<bool>& bits, const QString& text, size_t maxBits);

    /**
     * @brief 将二进制数据编码为6-bit ASCII（符合AIS标准）
     * @param bits 二进制向量
     * @return 编码后的字符串
     */
    std::string encodeTo6bitAscii(const std::vector<bool>& bits);

    /**
     * @brief 计算NMEA校验和
     * @param data NMEA数据（不包含$和*）
     * @return 校验和（十六进制字符串）
     */
    QString calculateNmeaChecksum(const QString& data);

    /**
     * @brief 构建完整NMEA语句
     * @param payload 6-bit ASCII编码的载荷
     * @param fillBits 填充位数
     * @return 完整的NMEA语句
     */
    std::string buildNmeaSentence(const std::string& payload, int fillBits);
};

#endif // AIS_MESSAGE_GENERATOR_H