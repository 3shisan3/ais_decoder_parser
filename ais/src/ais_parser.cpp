#include "ais_parser.h"

#include "core/bit_buffer.h"
#include "core/nmea_parser.h"
#include "core/multipart_reassembler.h"

namespace ais {

AISParser::AISParser(const Config& cfg) : config(cfg) {}

std::unique_ptr<AISMessage> AISParser::parse(const std::string& nmea) const {
    // 验证校验和
    if (config.validateChecksum && !NMEAParser::validateChecksum(nmea)) {
        return nullptr;
    }
    
    // 提取负载
    std::string payload = NMEAParser::extractPayload(nmea);
    if (payload.empty()) {
        return nullptr;
    }
    
    // 检查是否为多部分消息
    int fragmentCount = NMEAParser::getFragmentCount(nmea);
    int fragmentNumber = NMEAParser::getFragmentNumber(nmea);
    std::string messageId = NMEAParser::getMessageId(nmea);
    
    if (config.enableMultipartReassembly && fragmentCount > 1) {
        static MultipartReassembler reassembler(config.maxMultipartAge);
        reassembler.addFragment(messageId, payload, fragmentNumber, fragmentCount);
        
        if (reassembler.isComplete(messageId, fragmentCount)) {
            std::string completePayload = reassembler.reassemble(messageId, fragmentCount);
            auto msg = parseBinary(completePayload);
            if (msg) {
                msg->rawNMEA = nmea;
            }
            return msg;
        }
        return nullptr;
    }
    
    // 单部分消息处理
    auto msg = parseBinary(payload);
    if (msg) {
        msg->rawNMEA = nmea;
    }
    return msg;
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

std::unique_ptr<AISMessage> AISParser::parseBinary(const std::string& binary) const {
    BitBuffer bits(NMEAParser::decode6bitASCII(binary));
    int messageType = bits.getInt(0, 6);
    return AISMessage::create(static_cast<AISMessageType>(messageType));
}

void AISParser::setConfig(const Config& newConfig) {
    config = newConfig;
}

const Config& AISParser::getConfig() const {
    return config;
}

} // namespace ais