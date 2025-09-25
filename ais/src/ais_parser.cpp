#include "ais_parser.h"

#include "core/bit_buffer.h"
#include "core/nmea_parser.h"
#include "utils/multipart_reassembler.h"

namespace ais {

AISParser::AISParser(const Config& cfg) : config_(cfg) {}

std::unique_ptr<AISMessage> AISParser::parse(const std::string &nmea) const
{
    // 验证校验和
    if (config_.validateChecksum && !NMEAParser::validateChecksum(nmea))
    {
        return nullptr;
    }

    // 处理多部分消息
    int fragmentCount = NMEAParser::getFragmentCount(nmea);
    int fragmentNumber = NMEAParser::getFragmentNumber(nmea);
    std::string messageId = NMEAParser::getMessageId(nmea);

    if (config_.enableMultipartReassembly && fragmentCount > 1)
    {
        static MultipartReassembler reassembler(config_.maxMultipartAge);
        std::string payload = NMEAParser::extractPayload(nmea);

        reassembler.addFragment(messageId, payload, fragmentNumber, fragmentCount);

        if (reassembler.isComplete(messageId, fragmentCount))
        {
            std::string completePayload = reassembler.reassemble(messageId, fragmentCount);
            return parseBinary(NMEAParser::decode6bitASCII(completePayload));
        }
        return nullptr;
    }

    // 单部分消息
    std::string payload = NMEAParser::extractPayload(nmea);
    return parseBinary(NMEAParser::decode6bitASCII(payload));
}

std::vector<std::unique_ptr<AISMessage>> AISParser::parseBatch(const std::vector<std::string>& nmeaSentences) const {
    std::vector<std::unique_ptr<AISMessage>> messages;
    for (const auto& nmea : nmeaSentences) {
        auto msg = parse(nmea);
        if (msg) {
            messages.push_back(std::move(msg));
        }
    }
    return messages;
}

std::unique_ptr<AISMessage> AISParser::parseBinary(const std::string &binaryData) const
{
    try
    {
        BitBuffer bits(binaryData);
        auto message = AISMessage::parse(bits);
        return message;
    }
    catch (const std::exception &e)
    {
        return nullptr;
    }
}

void AISParser::setConfig(const Config& newConfig) {
    config_ = newConfig;
}

const Config& AISParser::getConfig() const {
    return config_;
}

} // namespace ais