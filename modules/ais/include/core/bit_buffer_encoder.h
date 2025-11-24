#ifndef AIS_BIT_BUFFER_ENCODER_H
#define AIS_BIT_BUFFER_ENCODER_H

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace ais
{

/**
 * @brief 位缓冲区编码器类 - 用于将各种类型的数据编码为二进制位
 */
class BitBufferEncoder
{
public:
    BitBufferEncoder();
    
    /**
     * @brief 获取编码后的二进制字符串
     */
    std::string getBinaryString() const;
    
    /**
     * @brief 获取编码后的二进制数据
     */
    const std::vector<bool>& getBits() const { return bits_; }
    
    /**
     * @brief 获取当前位位置
     */
    size_t getPosition() const { return bitPosition_; }
    
    /**
     * @brief 设置位位置
     */
    void setPosition(size_t pos);
    
    /**
     * @brief 清空缓冲区
     */
    void clear();
    
    /************* 基本位操作 *************/
    
    /**
     * @brief 添加无符号整数值
     * @param value 值
     * @param length 位数
     */
    void putUInt32(uint32_t value, size_t length);
    
    /**
     * @brief 添加有符号整数值（二进制补码）
     * @param value 值
     * @param length 位数
     */
    void putInt(int32_t value, size_t length);
    
    /**
     * @brief 添加布尔值
     * @param value 布尔值
     */
    void putBool(bool value);
    
    /**
     * @brief 添加字符串值（6-bit ASCII编码）
     * @param str 字符串
     * @param length 总位数（必须是6的倍数）
     */
    void putString(const std::string& str, size_t length);
    
    /************* 特殊数据类型 *************/
    
    /**
     * @brief 添加纬度值
     * @param latitude 纬度（度）
     * @param length 位数（通常为27）
     */
    void putLatitude(double latitude, size_t length = 27);
    
    /**
     * @brief 添加经度值
     * @param longitude 经度（度）
     * @param length 位数（通常为28）
     */
    void putLongitude(double longitude, size_t length = 28);
    
    /**
     * @brief 添加速度值
     * @param speed 速度（节）
     * @param length 位数（通常为10）
     */
    void putSpeed(double speed, size_t length = 10);
    
    /**
     * @brief 添加航向值
     * @param course 航向（度）
     * @param length 位数（通常为12）
     */
    void putCourse(double course, size_t length = 12);
    
    /**
     * @brief 添加转向率值
     * @param rate 转向率（度/分钟）
     * @param length 位数（通常为8）
     */
    void putRateOfTurn(double rate, size_t length = 8);
    
    /**
     * @brief 填充指定位数
     * @param bits 填充位数
     * @param value 填充值（默认0）
     */
    void putPadding(size_t bits, bool value = false);

private:
    std::vector<bool> bits_;      // 存储位的向量
    size_t bitPosition_ = 0;      // 当前位位置
    
    /**
     * @brief 扩展到指定位数
     */
    void ensureCapacity(size_t requiredBits);
    
    /**
     * @brief 有符号整数转二进制补码
     */
    uint32_t toTwosComplement(int32_t value, size_t bits) const;
};

} // namespace ais

#endif // AIS_BIT_BUFFER_ENCODER_H