#include "ais_parser.h"

#include "core/bit_buffer.h"
#include "core/nmea_parser.h"
#include "utils/multipart_reassembler.h"

namespace ais {

AISParser::AISParser(const AISParseCfg& cfg) : config_(cfg) {}

std::unique_ptr<AISMessage> AISParser::parse(const std::string &nmea) const
{
    // 验证校验和
    if (config_.validateChecksum && !NMEAParser::validateChecksum(nmea))
    {
        return nullptr;
    }

    // 提取负载
    std::string payload = NMEAParser::extractPayload(nmea);
    if (payload.empty()) {
        return nullptr;
    }

    // 处理多部分消息
    int fragmentCount = NMEAParser::getFragmentCount(nmea);
    int fragmentNumber = NMEAParser::getFragmentNumber(nmea);
    
    if (config_.enableMultipartReassembly && fragmentCount > 1)
    {
        // 生成唯一消息ID（使用负载前几个字符作为标识）
        std::string messageId = payload.substr(0, std::min(10, (int)payload.length()));
        
        static MultipartReassembler reassembler(config_.maxMultipartAge);
        reassembler.addFragment(messageId, payload, fragmentNumber, fragmentCount);

        if (reassembler.isComplete(messageId, fragmentCount))
        {
            std::string completePayload = reassembler.reassemble(messageId, fragmentCount);
            return parseBinary(NMEAParser::decode6bitASCII(completePayload));
        }
        return nullptr; // 等待更多片段
    }

    // 单部分消息直接解析
    std::string binaryData = NMEAParser::decode6bitASCII(payload);
    return parseBinary(binaryData);
}

std::vector<std::unique_ptr<AISMessage>> AISParser::parseBatch(const std::vector<std::string> &nmeaSentences) const
{
    std::vector<std::unique_ptr<AISMessage>> messages;
    for (const auto &nmea : nmeaSentences)
    {
        auto msg = parse(nmea);
        if (msg)
        {
            messages.push_back(std::move(msg));
        }
    }
    return messages;
}

std::unique_ptr<AISMessage> AISParser::parseBinary(const std::string &binaryData) const
{
    if (binaryData.empty()) {
        return nullptr;
    }
    
    try
    {
        BitBuffer bits(binaryData);
        
        // 验证有足够的数据读取消息类型
        if (bits.remaining() < 6) {
            return nullptr;
        }
        
        auto message = AISMessage::parse(bits);
        return message;
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

void AISParser::setConfig(const AISParseCfg &newConfig)
{
    config_ = newConfig;
}

const AISParseCfg &AISParser::getConfig() const
{
    return config_;
}

} // namespace ais