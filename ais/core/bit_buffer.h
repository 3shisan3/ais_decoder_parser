#ifndef AIS_BIT_BUFFER_H
#define AIS_BIT_BUFFER_H

#include <bitset>
#include <string>

namespace ais
{

/**
 * @brief 位缓冲区类
 * 
 * 用于从AIS二进制数据中提取各种类型的字段
 */
class BitBuffer
{
public:
    /**
     * @brief 构造函数
     * @param payload 二进制负载数据
     */
    explicit BitBuffer(const std::string &payload);

    /**
     * @brief 获取指定位范围的整数值
     * @param start 起始位位置
     * @param length 位数长度
     * @return 整数值
     */
    int getInt(int start, int length);

    /**
     * @brief 获取指定位范围的字符串值
     * @param start 起始位位置
     * @param length 位数长度
     * @return 字符串值
     */
    std::string getString(int start, int length);

    /**
     * @brief 获取指定位的布尔值
     * @param start 位位置
     * @return 布尔值
     */
    bool getBool(int start);

    /**
     * @brief 获取指定位范围的纬度值
     * @param start 起始位位置
     * @return 纬度值(度)
     */
    double getLatitude(int start);

    /**
     * @brief 获取指定位范围的经度值
     * @param start 起始位位置
     * @return 经度值(度)
     */
    double getLongitude(int start);

    /**
     * @brief 获取指定位范围的速度值
     * @param start 起始位位置
     * @return 速度值(节)
     */
    double getSpeed(int start);

    /**
     * @brief 获取指定位范围的航向值
     * @param start 起始位位置
     * @return 航向值(度)
     */
    double getCourse(int start);

    /**
     * @brief 获取指定位范围的航行状态
     * @param start 起始位位置
     * @return 航行状态值
     */
    int getNavStatus(int start);

    /**
     * @brief 获取指定位范围的机动指示器
     * @param start 起始位位置
     * @return 机动指示器值
     */
    int getManeuverIndicator(int start);

    /**
     * @brief 获取指定位范围的RAIM标志
     * @param start 位位置
     * @return RAIM标志值
     */
    int getRAIMFlag(int start);

    /**
     * @brief 获取指定位范围的通信状态
     * @param start 起始位位置
     * @return 通信状态值
     */
    int getCommunicationState(int start);

private:
    std::string payload;    // 二进制负载数据
    size_t bitPosition = 0; // 当前位位置
};

} // namespace ais

#endif // AIS_BIT_BUFFER_H