#include "core/bit_buffer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ais
{

BitBuffer::BitBuffer(const std::string &binaryData)
{
    for (char c : binaryData)
    {
        int value = charTo6Bit(c);
        for (int i = 5; i >= 0; i--)
        {
            bits_.push_back((value & (1 << i)) != 0);
        }
    }
    totalBits_ = bits_.size();
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

int BitBuffer::charTo6Bit(char c)
{
    if (c >= 48 && c <= 87) // '0' to 'W'
    {
        int value = c - 48;
        if (value > 40)     // 处理特殊字符范围
            value -= 8;
        return value & 0x3F;
    }
    return 0;               // 无效字符
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

int BitBuffer::getInt(size_t start, size_t length)
{
    checkRange(start, length);
    int result = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (bits_[start + i])
        {
            result |= (1 << (length - 1 - i));
        }
    }
    return result;
}

int BitBuffer::getInt(size_t length)
{
    int result = getInt(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

uint32_t BitBuffer::getUInt32(size_t start, size_t length)
{
    return static_cast<uint32_t>(getInt(start, length));
}

uint32_t BitBuffer::getUInt32(size_t length)
{
    return static_cast<uint32_t>(getInt(length));
}

std::string BitBuffer::getString(size_t start, size_t length)
{
    checkRange(start, length);
    std::string result;
    size_t charCount = length / 6;

    for (size_t i = 0; i < charCount; i++)
    {
        int bitsValue = getInt(start + i * 6, 6);
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
    int value = getInt(start, length);
    if (value == 0x3412140)
        return 91.0; // 默认值
    return value / 600000.0;
}

double BitBuffer::getLatitude(size_t length)
{
    double result = getLatitude(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getLongitude(size_t start, size_t length)
{
    int value = getInt(start, length);
    if (value == 0x6791AC0)
        return 181.0; // 默认值
    return value / 600000.0;
}

double BitBuffer::getLongitude(size_t length)
{
    double result = getLongitude(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getSpeed(size_t start, size_t length)
{
    int sogRaw = getInt(start, length);
    return sogRaw == 1023 ? 0 : sogRaw / 10.0;
}

double BitBuffer::getSpeed(size_t length)
{
    double result = getSpeed(bitPosition_, length);
    bitPosition_ += length;
    return result;
}

double BitBuffer::getCourse(size_t start, size_t length)
{
    int cogRaw = getInt(start, length);
    return cogRaw == 3600 ? 0 : cogRaw / 10.0;
}

double BitBuffer::getCourse(size_t length)
{
    double result = getCourse(bitPosition_, length);
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

} // namespace ais