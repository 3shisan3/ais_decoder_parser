#include "ais_encoder.h"

#include "messages/message_encoder_factory.h"
#include "utils/sixbit_ascii_encoder.h"

namespace ais
{

AISEncoder::AISEncoder(const AISGenerateCfg &cfg) : config_(cfg) {}

std::vector<std::string> AISEncoder::encode(const AISMessage &message,
                                            NMEAMessageType messageType)
{
    std::vector<std::string> nmeaSentences;

    try
    {
        // 1. 将消息编码为二进制数据
        std::string binaryData = MessageEncoderFactory::encodeMessage(message);

        // 2. 分片处理
        if (config_.enableFragmentation && binaryData.length() > config_.defaultFragmentSize * 6)
        {
            nmeaSentences = fragmentMessage(binaryData, messageType, message.type);
        }
        else
        {
            // 3. 编码为6-bit ASCII
            std::string payload = SixBitASCIIEncoder::encode(binaryData);

            // 4. 计算填充位数
            int fillBits = calculateFillBits(binaryData.length());

            // 5. 生成NMEA语句
            std::string nmea = NMEAEncoder::encodeAIS(
                messageType, payload, 1, 1, config_.defaultSequenceId,
                config_.defaultChannel, fillBits);

            nmeaSentences.push_back(nmea);
        }
    }
    catch (const std::exception &e)
    {
        // 编码失败，返回空向量
    }

    return nmeaSentences;
}

std::vector<std::string> AISEncoder::encodeBatch(const std::vector<std::unique_ptr<AISMessage>> &messages,
                                                 NMEAMessageType messageType)
{
    std::vector<std::string> allNmeaSentences;

    for (const auto &message : messages)
    {
        if (message)
        {
            auto nmeaSentences = encode(*message, messageType);
            allNmeaSentences.insert(allNmeaSentences.end(),
                                    nmeaSentences.begin(), nmeaSentences.end());
        }
    }

    return allNmeaSentences;
}

std::vector<std::string> AISEncoder::fragmentMessage(const std::string &binaryData,
                                                     NMEAMessageType messageType,
                                                     AISMessageType aisMessageType)
{
    std::vector<std::string> fragments;

    size_t fragmentSize = config_.defaultFragmentSize * 6; // 转换为位数
    size_t totalBits = binaryData.length();
    size_t totalFragments = (totalBits + fragmentSize - 1) / fragmentSize;

    for (size_t i = 0; i < totalFragments; i++)
    {
        size_t startBit = i * fragmentSize;
        size_t endBit = std::min(startBit + fragmentSize, totalBits);
        size_t fragmentBits = endBit - startBit;

        // 提取分片数据
        std::string fragmentBinary = binaryData.substr(startBit, fragmentBits);

        // 编码为6-bit ASCII
        std::string payload = SixBitASCIIEncoder::encode(fragmentBinary);

        // 计算填充位数（仅最后一个分片需要）
        int fillBits = (i == totalFragments - 1) ? calculateFillBits(fragmentBits) : 0;

        // 生成NMEA语句
        std::string nmea = NMEAEncoder::encodeAIS(
            messageType, payload, totalFragments, i + 1, config_.defaultSequenceId,
            config_.defaultChannel, fillBits);

        fragments.push_back(nmea);
    }

    return fragments;
}

int AISEncoder::calculateFillBits(size_t binaryLength) const
{
    int remainder = binaryLength % 6;
    return (remainder == 0) ? 0 : (6 - remainder);
}

void AISEncoder::setConfig(const AISGenerateCfg &newConfig)
{
    config_ = newConfig;
}

const AISGenerateCfg &AISEncoder::getConfig() const
{
    return config_;
}

} // namespace ais