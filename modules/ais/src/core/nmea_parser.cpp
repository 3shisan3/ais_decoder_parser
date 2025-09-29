#include "core/nmea_parser.h"
#include "core/bit_buffer.h"

#include <algorithm>
#include <sstream>
#include <vector>
#include <stdexcept>

namespace ais
{

bool NMEAParser::validateChecksum(const std::string &nmea)
{
    // 检查字符串是否以'$'或'!'开头
    size_t start = nmea.find_first_of("$!");
    if (start == std::string::npos)
        return false;

    // 查找校验和分隔符'*'
    size_t end = nmea.find('*');
    if (end == std::string::npos || end <= start + 1)
        return false;

    // 提取'$'/'!'和'*'之间的数据部分
    std::string data = nmea.substr(start + 1, end - start - 1);
    
    // 计算数据的异或校验和
    int checksum = 0;
    for (char c : data)
    {
        checksum ^= c;
    }

    // 提取并转换校验和字符串
    std::string checksumStr = nmea.substr(end + 1, 2);
    int expectedChecksum;
    try
    {
        expectedChecksum = std::stoi(checksumStr, nullptr, 16);
    }
    catch (const std::exception&)
    {
        return false; // 校验和格式错误
    }

    return checksum == expectedChecksum;
}

std::string NMEAParser::extractPayload(const std::string &nmea)
{
    auto parts = split(nmea, ',');
    // AIVDM格式: !AIVDM,<片段总数>,<片段编号>,<序列号>,<信道>,<负载>,<填充位数>*<校验和>
    // 负载在第六个字段（索引5）
    if (parts.size() > 5 && !parts[5].empty())
    {
        std::string payload = parts[5];
        // 移除可能存在的校验和部分
        size_t asteriskPos = payload.find('*');
        if (asteriskPos != std::string::npos)
        {
            payload = payload.substr(0, asteriskPos);
        }
        return payload;
    }
    return "";
}

std::string NMEAParser::decode6bitASCII(const std::string &payload)
{
    std::string binaryData;
    
    // 遍历负载中的每个字符
    for (char c : payload)
    {
        // 将6-bit ASCII字符转换为0-63的整数值
        int value = BitBuffer::charTo6Bit(c);
        
        // 将6位值转换为6个二进制字符（'0'或'1'）
        for (int i = 5; i >= 0; i--)
        {
            binaryData += ((value & (1 << i)) != 0) ? '1' : '0';
        }
    }
    
    return binaryData;
}

int NMEAParser::getFragmentCount(const std::string &nmea)
{
    auto parts = split(nmea, ',');
    // 片段总数在第二个字段（索引1）
    if (parts.size() > 1 && !parts[1].empty())
    {
        try
        {
            return std::stoi(parts[1]);
        }
        catch (const std::exception&)
        {
            return 1; // 转换失败时返回默认值
        }
    }
    return 1; // 默认单片段消息
}

int NMEAParser::getFragmentNumber(const std::string &nmea)
{
    auto parts = split(nmea, ',');
    // 片段编号在第三个字段（索引2）
    if (parts.size() > 2 && !parts[2].empty())
    {
        try
        {
            return std::stoi(parts[2]);
        }
        catch (const std::exception&)
        {
            return 1; // 转换失败时返回默认值
        }
    }
    return 1; // 默认第一个片段
}

std::string NMEAParser::getMessageId(const std::string &nmea)
{
    auto parts = split(nmea, ',');
    // 消息类型在第四个字段（索引3），但实际AIS消息类型在负载的前6位
    // 这里返回空字符串，实际消息类型从二进制数据中解析
    return "";
}

std::vector<std::string> NMEAParser::split(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    // 使用stringstream按分隔符分割字符串
    while (std::getline(ss, item, delimiter))
    {
        result.push_back(item);
    }
    
    return result;
}

int NMEAParser::hexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0; // 无效字符返回0
}

} // namespace ais