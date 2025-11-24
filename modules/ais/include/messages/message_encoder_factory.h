#ifndef AIS_MESSAGE_ENCODER_FACTORY_H
#define AIS_MESSAGE_ENCODER_FACTORY_H

#include "type_definitions.h"
#include "core/bit_buffer_encoder.h"

#include <memory>

namespace ais
{

/**
 * @brief 消息编码工厂类
 * 负责将AIS消息对象编码为二进制数据
 */
class MessageEncoderFactory
{
public:
    /**
     * @brief 编码AIS消息
     * @param message AIS消息对象
     * @return 二进制字符串
     */
    static std::string encodeMessage(const AISMessage& message);
    
private:
    // 27种消息类型的编码实现
    static std::string encodeType1(const PositionReport& msg);
    static std::string encodeType2(const PositionReportAssigned& msg);
    static std::string encodeType3(const PositionReportResponse& msg);
    static std::string encodeType4(const BaseStationReport& msg);
    static std::string encodeType5(const StaticVoyageData& msg);
    static std::string encodeType6(const BinaryAddressedMessage& msg);
    static std::string encodeType7(const BinaryAcknowledge& msg);
    static std::string encodeType8(const BinaryBroadcastMessage& msg);
    static std::string encodeType9(const StandardSARAircraftReport& msg);
    static std::string encodeType10(const UTCDateInquiry& msg);
    static std::string encodeType11(const UTCDateResponse& msg);
    static std::string encodeType12(const AddressedSafetyMessage& msg);
    static std::string encodeType13(const SafetyAcknowledge& msg);
    static std::string encodeType14(const SafetyRelatedBroadcast& msg);
    static std::string encodeType15(const Interrogation& msg);
    static std::string encodeType16(const AssignmentModeCommand& msg);
    static std::string encodeType17(const DGNSSBinaryBroadcast& msg);
    static std::string encodeType18(const StandardClassBReport& msg);
    static std::string encodeType19(const ExtendedClassBReport& msg);
    static std::string encodeType20(const DataLinkManagement& msg);
    static std::string encodeType21(const AidToNavigationReport& msg);
    static std::string encodeType22(const ChannelManagement& msg);
    static std::string encodeType23(const GroupAssignmentCommand& msg);
    static std::string encodeType24(const StaticDataReport& msg);
    static std::string encodeType25(const SingleSlotBinaryMessage& msg);
    static std::string encodeType26(const MultipleSlotBinaryMessage& msg);
    static std::string encodeType27(const LongRangePositionReport& msg);
    
    /**
     * @brief 编码二进制数据
     */
    static void encodeBinaryData(BitBufferEncoder& encoder, const std::vector<uint8_t>& data, size_t maxBits);
};

} // namespace ais

#endif // AIS_MESSAGE_ENCODER_FACTORY_H