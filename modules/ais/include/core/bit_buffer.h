/***************************************************************
Copyright (c) 2022-2030, shisan233@sszc.live.
SPDX-License-Identifier: MIT 
File:        bit_buffer.h
Version:     1.0
Author:      cjx
start date:
Description: 位操作核心类
Version history

[序号]    |   [修改日期]  |   [修改者]   |   [修改内容]
1            2025-09-25       cjx         create

*****************************************************************/

#ifndef AIS_BIT_BUFFER_H
#define AIS_BIT_BUFFER_H

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace ais
{

/**
 * @brief 位缓冲区类 - 用于从AIS二进制数据中提取各种类型的字段
 * @note 重新设计为更健壮和完整的实现
 */
class BitBuffer
{
public:
    /**
     * @brief 构造函数
     * @param binaryPayload 二进制负载数据
     */
    explicit BitBuffer(const std::string& binaryPayload);
    
    /**
     * @brief 获取当前位位置
     */
    size_t getPosition() const { return bitPosition_; }
    
    /**
     * @brief 设置位位置
     * @param pos 新的位位置
     */
    void setPosition(size_t pos);
    
    /**
     * @brief 获取剩余位数
     */
    size_t remaining() const { return totalBits_ - bitPosition_; }
    
    /************* 基本位操作 *************/
    /**
     * @brief 获取指定位范围的整数值
     * @param start 起始位位置
     * @param length 位数长度
     * @return 整数值
     */
    int getInt(size_t start, size_t length);
    
    /**
     * @brief 获取指定位范围的整数值（从当前位置）
     * @param length 位数长度
     * @return 整数值
     */
    int getInt(size_t length);

    uint32_t getUInt32(size_t start, size_t length);
    uint32_t getUInt32(size_t length);
    
    /**
     * @brief 获取指定位范围的字符串值
     * @param start 起始位位置
     * @param length 位数长度
     * @return 字符串值
     */
    std::string getString(size_t start, size_t length);
    
    /**
     * @brief 获取指定位范围的字符串值（从当前位置）
     * @param length 位数长度
     * @return 字符串值
     */
    std::string getString(size_t length);
    
    /**
     * @brief 获取指定位的布尔值
     * @param start 位位置
     * @return 布尔值
     */
    bool getBool(size_t start);
    
    /**
     * @brief 获取指定位的布尔值（从当前位置）
     * @return 布尔值
     */
    bool getBool();
    
    /************* 特殊数据类型 *************/
    /**
     * @brief 获取指定位范围的纬度值
     * @param start 起始位位置
     * @return 纬度值(度)
     */
    double getLatitude(size_t start, size_t length = 27);
    
    /**
     * @brief 获取指定位范围的纬度值（从当前位置）
     * @param length 位数长度（通常为27）
     * @return 纬度值(度)
     */
    double getLatitude(size_t length = 27);
    
    /**
     * @brief 获取指定位范围的经度值
     * @param start 起始位位置
     * @return 经度值(度)
     */
    double getLongitude(size_t start, size_t length = 28);
    
    /**
     * @brief 获取指定位范围的经度值（从当前位置）
     * @param length 位数长度（通常为28）
     * @return 经度值(度)
     */
    double getLongitude(size_t length = 28);
    
    /**
     * @brief 获取指定位范围的速度值
     * @param start 起始位位置
     * @return 速度值(节)
     */
    double getSpeed(size_t start, size_t length = 10);
    
    /**
     * @brief 获取指定位范围的速度值（从当前位置）
     * @param length 位数长度（通常为10）
     * @return 速度值(节)
     */
    double getSpeed(size_t length = 10);
    
    /**
     * @brief 获取指定位范围的航向值
     * @param start 起始位位置
     * @return 航向值(度)
     */
    double getCourse(size_t start, size_t length = 12);
    
    /**
     * @brief 获取指定位范围的航向值（从当前位置）
     * @param length 位数长度（通常为12）
     * @return 航向值(度)
     */
    double getCourse(size_t length = 12);

    /**
     * @brief 获取指定位范围的转向率值
     * @param start 起始位位置
     * @param length 位数长度（通常为8）
     * @return 转向率值(度/分钟)
     */
    double getRateOfTurn(size_t start, size_t length = 8);
    
    /**
     * @brief 获取指定位范围的转向率值（从当前位置）
     * @param length 位数长度（通常为8）
     * @return 转向率值(度/分钟)
     */
    double getRateOfTurn(size_t length = 8);
    
    /**
     * @brief 跳过指定数量的位
     * @param bits 要跳过的位数
     */
    void skip(size_t bits);

    
    /**
     * @brief 将6-bit ASCII字符转换为二进制值
     * @param c 6-bit ASCII字符
     * @return 二进制值(0-63)
     */
    static int charTo6Bit(char c);

    static char bit6ToChar(int value);

private:
    std::vector<bool> bits_;    // 存储位的向量
    size_t bitPosition_ = 0;    // 当前位位置
    size_t totalBits_ = 0;      // 总位数

    /**
     * @brief 检查位范围是否有效
     * @param start 起始位置
     * @param length 长度
     */
    void checkRange(size_t start, size_t length) const;

    /**
     * @brief 二进制补码转换
     * @param value 原始值
     * @param bits 位数
     * @return 有符号整数值
     */
    int32_t fromTwosComplement(uint32_t value, size_t bits) const;
};

} // namespace ais

#endif // AIS_BIT_BUFFER_H