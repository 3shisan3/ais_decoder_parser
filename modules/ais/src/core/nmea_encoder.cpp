#include "core/nmea_encoder.h"

#include <sstream>
#include <iomanip>

namespace ais
{

std::string NMEAEncoder::encodeAIS(
    NMEAMessageType messageType,
    const std::string &payload,
    int fragmentCount,
    int fragmentNumber,
    const std::string &sequenceId,
    char channel,
    int fillBits)
{
    std::ostringstream nmea;

    // 构建数据部分
    std::ostringstream data;
    data << getMessageTypeString(messageType) << ","
         << fragmentCount << ","
         << fragmentNumber << ","
         << escapeField(sequenceId) << ","
         << channel << ","
         << payload << ","
         << fillBits;

    std::string dataStr = data.str();

    // 计算校验和
    std::string checksum = calculateChecksum(dataStr);

    // 组装完整NMEA语句
    nmea << "!" << dataStr << "*" << checksum;

    return nmea.str();
}

std::string NMEAEncoder::getMessageTypeString(NMEAMessageType type)
{
    switch (type)
    {
    case NMEAMessageType::AIVDM:
        return "AIVDM";
    case NMEAMessageType::AIVDO:
        return "AIVDO";
    default:
        return "AIVDM";
    }
}

std::string NMEAEncoder::calculateChecksum(const std::string &data)
{
    int checksum = 0;
    for (char c : data)
    {
        checksum ^= c;
    }

    std::ostringstream result;
    result << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << checksum;
    return result.str();
}

std::string NMEAEncoder::escapeField(const std::string &field)
{
    if (field.empty())
    {
        return "";
    }

    // 在实际实现中，这里应该处理NMEA转义规则
    // 简化实现：直接返回原字段
    return field;
}

} // namespace ais