#include "utils/sixbit_ascii_encoder.h"

#include <stdexcept>

namespace ais
{

std::string SixBitASCIIEncoder::encode(const std::string& binaryData)
{
    if (binaryData.empty()) {
        return "";
    }
    
    // 填充到6的倍数
    std::string padded = padToMultipleOfSix(binaryData);
    
    std::string result;
    size_t bitLength = padded.length();
    
    for (size_t i = 0; i < bitLength; i += 6) {
        // 提取6位
        int value = 0;
        for (int j = 0; j < 6; j++) {
            if (i + j < bitLength) {
                char bitChar = padded[i + j];
                if (bitChar == '1') {
                    value |= (1 << (5 - j));
                } else if (bitChar != '0') {
                    throw std::invalid_argument("Invalid binary character");
                }
            }
        }
        
        // 转换为ASCII字符
        result += valueToChar(value);
    }
    
    return result;
}

char SixBitASCIIEncoder::valueToChar(int value)
{
    value &= 0x3F; // 确保6位
    
    // AIS 6-bit ASCII编码规则
    if (value < 32) {
        return static_cast<char>(value + 64);
    } else {
        return static_cast<char>(value);
    }
}

std::string SixBitASCIIEncoder::padToMultipleOfSix(const std::string& binaryData)
{
    size_t remainder = binaryData.length() % 6;
    if (remainder == 0) {
        return binaryData;
    }
    
    // 填充0到6的倍数
    return binaryData + std::string(6 - remainder, '0');
}

} // namespace ais