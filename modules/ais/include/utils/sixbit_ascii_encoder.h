#ifndef AIS_SIXBIT_ASCII_ENCODER_H
#define AIS_SIXBIT_ASCII_ENCODER_H

#include <string>

namespace ais
{

/**
 * @brief 6-bit ASCII编码器类
 * 用于将二进制数据编码为6-bit ASCII字符串
 */
class SixBitASCIIEncoder
{
public:
    /**
     * @brief 将二进制字符串编码为6-bit ASCII
     * @param binaryData 二进制字符串（只包含'0'和'1'）
     * @return 6-bit ASCII编码的字符串
     */
    static std::string encode(const std::string& binaryData);
    
    /**
     * @brief 将6-bit值转换为ASCII字符
     * @param value 6-bit值 (0-63)
     * @return ASCII字符
     */
    static char valueToChar(int value);

private:
    /**
     * @brief 填充二进制字符串到6的倍数
     */
    static std::string padToMultipleOfSix(const std::string& binaryData);
};

} // namespace ais

#endif // AIS_SIXBIT_ASCII_ENCODER_H