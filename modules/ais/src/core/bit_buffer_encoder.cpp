#include "core/bit_buffer_encoder.h"

#include <cmath>
#include <stdexcept>

namespace ais
{

BitBufferEncoder::BitBufferEncoder()
{
    bits_.reserve(1024); // 预分配空间
}

std::string BitBufferEncoder::getBinaryString() const
{
    std::string binary;
    binary.reserve(bits_.size());
    
    for (bool bit : bits_) {
        binary += bit ? '1' : '0';
    }
    
    return binary;
}

void BitBufferEncoder::setPosition(size_t pos)
{
    if (pos > bits_.size()) {
        throw std::out_of_range("Position exceeds buffer size");
    }
    bitPosition_ = pos;
}

void BitBufferEncoder::clear()
{
    bits_.clear();
    bitPosition_ = 0;
}

void BitBufferEncoder::ensureCapacity(size_t requiredBits)
{
    if (bitPosition_ + requiredBits > bits_.size()) {
        bits_.resize(bitPosition_ + requiredBits, false);
    }
}

uint32_t BitBufferEncoder::toTwosComplement(int32_t value, size_t bits) const
{
    if (bits == 0) return 0;
    
    uint32_t mask = (1U << bits) - 1;
    
    if (value < 0) {
        // 负数：计算补码
        return (static_cast<uint32_t>(-value) ^ mask) + 1;
    } else {
        // 正数：直接返回
        return static_cast<uint32_t>(value) & mask;
    }
}

void BitBufferEncoder::putUInt32(uint32_t value, size_t length)
{
    if (length > 32) {
        throw std::out_of_range("Length exceeds 32 bits for uint32");
    }
    
    ensureCapacity(length);
    
    // 从最高位到最低位写入
    for (size_t i = 0; i < length; i++) {
        bool bit = (value & (1U << (length - 1 - i))) != 0;
        bits_[bitPosition_ + i] = bit;
    }
    
    bitPosition_ += length;
}

void BitBufferEncoder::putInt(int32_t value, size_t length)
{
    uint32_t unsignedValue = toTwosComplement(value, length);
    putUInt32(unsignedValue, length);
}

void BitBufferEncoder::putBool(bool value)
{
    ensureCapacity(1);
    bits_[bitPosition_] = value;
    bitPosition_++;
}

void BitBufferEncoder::putString(const std::string& str, size_t length)
{
    if (length % 6 != 0) {
        throw std::invalid_argument("String length must be multiple of 6");
    }
    
    size_t charCount = length / 6;
    ensureCapacity(length);
    
    for (size_t i = 0; i < charCount; i++) {
        char c = ' ';
        if (i < str.length()) {
            c = str[i];
        }
        
        // 简单的6-bit ASCII编码（实际应使用查找表）
        int value;
        if (c >= 64 && c < 96) {
            value = c - 64;
        } else if (c >= 32 && c < 64) {
            value = c;
        } else {
            value = 0; // 空格或无效字符
        }
        
        value &= 0x3F; // 确保6位
        
        // 写入6位
        for (int j = 5; j >= 0; j--) {
            bool bit = (value & (1 << j)) != 0;
            bits_[bitPosition_] = bit;
            bitPosition_++;
        }
    }
}

void BitBufferEncoder::putLatitude(double latitude, size_t length)
{
    // AIS 特殊值处理
    if (latitude == 91.0) {
        putUInt32(0x3412140, length); // 91° 的原始值
    } else if (latitude == -91.0) {
        putUInt32(0x6791AC0 / 2, length); // 经度的一半
    } else {
        int32_t value = static_cast<int32_t>(std::round(latitude * 600000.0));
        putInt(value, length);
    }
}

void BitBufferEncoder::putLongitude(double longitude, size_t length)
{
    // AIS 特殊值处理
    if (longitude == 181.0) {
        putUInt32(0x6791AC0, length); // 181° 的原始值
    } else if (longitude == -181.0) {
        putUInt32(0x6791AC0 / 2, length); // 经度的一半
    } else {
        int32_t value = static_cast<int32_t>(std::round(longitude * 600000.0));
        putInt(value, length);
    }
}

void BitBufferEncoder::putSpeed(double speed, size_t length)
{
    // AIS 特殊值处理
    if (speed >= 102.2) {
        putUInt32(1022, length); // 102.2 节或以上
    } else if (speed < 0) {
        putUInt32(1023, length); // 不可用
    } else {
        uint32_t value = static_cast<uint32_t>(std::round(speed * 10.0));
        if (value > 1022) value = 1022;
        putUInt32(value, length);
    }
}

void BitBufferEncoder::putCourse(double course, size_t length)
{
    // AIS 特殊值处理
    if (course < 0 || course >= 360) {
        putUInt32(3600, length); // 不可用
    } else {
        uint32_t value = static_cast<uint32_t>(std::round(course * 10.0));
        if (value >= 3600) value = 3600;
        putUInt32(value, length);
    }
}

void BitBufferEncoder::putRateOfTurn(double rate, size_t length)
{
    // AIS 转向率特殊编码规则
    if (rate == -128.0) {
        putInt(-128, length); // 转向率不可用
    } else if (rate >= 127.0) {
        putInt(127, length);  // 向右转向且速率超过5°/30s
    } else if (rate <= -127.0) {
        putInt(-127, length); // 向左转向且速率超过5°/30s
    } else {
        // 正常转向率计算公式的反向：value = sqrt(|ROT|) * 4.733
        double absRate = std::abs(rate);
        int32_t value = static_cast<int32_t>(std::round(std::sqrt(absRate) * 4.733));
        if (rate < 0) value = -value;
        putInt(value, length);
    }
}

void BitBufferEncoder::putPadding(size_t bits, bool value)
{
    ensureCapacity(bits);
    for (size_t i = 0; i < bits; i++) {
        bits_[bitPosition_ + i] = value;
    }
    bitPosition_ += bits;
}

} // namespace ais