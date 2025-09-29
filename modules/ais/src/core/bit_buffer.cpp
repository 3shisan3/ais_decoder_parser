#include "core/bit_buffer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace ais
{

constexpr std::array<uint8_t, 256> initToSixbit() {
    std::array<uint8_t, 256> table{};
    for (int chr = 0; chr < 256; chr++) {
        if (chr < 48 || chr > 119 || (chr > 87 && chr < 96)) {
            table[chr] = 0xFF; // -1 (invalid)
        } else if (chr < 0x60) { // chr < 96
            table[chr] = (chr - 48) & 0x3F; // 0-63
        } else {
            table[chr] = (chr - 56) & 0x3F; // 0-63
        }
    }
    return table;
}

constexpr auto toSixbit = initToSixbit();

BitBuffer::BitBuffer(const std::string &binaryData)
{
    for (char c : binaryData)
    {
        if (c == '1') {
            bits_.push_back(true);
        } else if (c == '0') {
            bits_.push_back(false);
        }
        // 忽略其他字符
    }
    totalBits_ = bits_.size();
    bitPosition_ = 0;
}

void BitBuffer::setPosition(size_t pos)
{
    if (pos > totalBits_)
    {
        throw std::out_of_range("Position exceeds buffer size");
    }
    bitPosition_ = pos;
}

void BitBuffer::checkRange(size_t start, size_t length) const
{
    if (start + length > totalBits_)
    {
        throw std::out_of_range("Bit range exceeds buffer size");
    }
}

int32_t BitBuffer::fromTwosComplement(uint32_t value, size_t bits) const
{
    if (bits == 0) return 0;
    
    // 检查符号位（最高位）
    uint32_t signBit = 1U << (bits - 1);
    
    if (value & signBit) {
        // 如果是负数，计算补码
        // 先取反码，然后加1，最后取负数
        uint32_t mask = (1U << bits) - 1;
        value = (~value + 1) & mask;
        return -static_cast<int32_t>(value);
    }
    
    // 如果是正数，直接返回
    return static_cast<int32_t>(value);
}

uint32_t BitBuffer::getUInt32(size_t start, size_t length)
{
    checkRange(start, length);
    
    if (length > 32) {
        throw std::out_of_range("Length exceeds 32 bits for uint32");
    }
    
    uint32_t result = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (bits_[start + i])
        {
            result |= (1U << (length - 1 - i));
        }
    }
    return result;
}

uint32_t BitBuffer::getUInt32(size_t length)
{
    uint32_t result = getUInt32(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

int32_t BitBuffer::getInt(size_t start, size_t length)
{
    uint32_t unsignedValue = getUInt32(start, length);
    return fromTwosComplement(unsignedValue, length);
}

int32_t BitBuffer::getInt(size_t length)
{
    int32_t result = getInt(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

std::string BitBuffer::getString(size_t start, size_t length)
{
    checkRange(start, length);
    std::string result;
    size_t charCount = length / 6;

    for (size_t i = 0; i < charCount; i++)
    {
        int bitsValue = getUInt32(start + i * 6, 6);
        if (bitsValue == 0)     // 字符串终止
            break;
        result += bit6ToChar(bitsValue);
    }

    // 去除尾部空格
    while (!result.empty() && result.back() == ' ')
    {
        result.pop_back();
    }

    return result;
}

std::string BitBuffer::getString(size_t length)
{
    std::string result = getString(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

bool BitBuffer::getBool(size_t start)
{
    checkRange(start, 1);
    return bits_[start];
}

bool BitBuffer::getBool()
{
    bool result = getBool(bitPosition_);
    bitPosition_++;
    return result;
}

double BitBuffer::getLatitude(size_t start, size_t length)
{
    int32_t value = getInt(start, length);
    
    // AIS 特殊值处理
    if (value == 0x3412140)      // 91° 的原始值（默认值）
        return 91.0;
    if (value == 0x6791AC0 / 2) // 经度的一半，用于某些特殊情况
        return -91.0;
    
    return value / 600000.0;    // 转换为度
}

double BitBuffer::getLatitude(size_t length)
{
    double result = getLatitude(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getLongitude(size_t start, size_t length)
{
    int32_t value = getInt(start, length);
    
    // AIS 特殊值处理
    if (value == 0x6791AC0)      // 181° 的原始值（默认值）
        return 181.0;
    if (value == 0x6791AC0 / 2) // 经度的一半，用于某些特殊情况
        return -181.0;
    
    return value / 600000.0;    // 转换为度
}

double BitBuffer::getLongitude(size_t length)
{
    double result = getLongitude(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getSpeed(size_t start, size_t length)
{
    uint32_t sogRaw = getUInt32(start, length);
    
    // AIS 特殊值：1023 表示速度不可用
    if (sogRaw == 1023)
        return 0;
    
    // AIS 特殊值：1022 表示速度 >= 102.2 节
    if (sogRaw == 1022)
        return 102.2;
    
    return sogRaw / 10.0;       // 转换为节
}

double BitBuffer::getSpeed(size_t length)
{
    double result = getSpeed(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getCourse(size_t start, size_t length)
{
    uint32_t cogRaw = getUInt32(start, length);
    
    // AIS 特殊值：3600 表示航向不可用
    if (cogRaw == 3600)
        return 0;
    
    return cogRaw / 10.0;       // 转换为度
}

double BitBuffer::getCourse(size_t length)
{
    double result = getCourse(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getRateOfTurn(size_t start, size_t length)
{
    int32_t rawValue = getInt(start, length);
    
    // AIS 转向率特殊编码规则
    if (rawValue == -128) {
        return -128.0; // 表示转向率不可用
    } else if (rawValue == 127) {
        return 127.0;  // 表示向右转向且速率超过5°/30s
    } else if (rawValue == -127) {
        return -127.0; // 表示向左转向且速率超过5°/30s
    } else {
        // 正常转向率计算公式：ROT = (value/4.733)^2
        double rot = std::pow(rawValue / 4.733, 2);
        return (rawValue >= 0) ? rot : -rot;
    }
}

double BitBuffer::getRateOfTurn(size_t length)
{
    double result = getRateOfTurn(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

void BitBuffer::skip(size_t bits)
{
    if (bitPosition_ + bits > totalBits_)
    {
        throw std::out_of_range("Skip exceeds buffer size");
    }
    bitPosition_ += bits;
}

int BitBuffer::charTo6Bit(char c)
{
    return toSixbit[c];
}

char BitBuffer::bit6ToChar(int value)
{
    value &= 0x3F;
    if (value < 32)
    {
        return static_cast<char>(value + 64);
    }
    return static_cast<char>(value);
}

} // namespace ais