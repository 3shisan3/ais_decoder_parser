#include "messages/message_encoder_factory.h"

#include <cmath>
#include <stdexcept>

namespace ais
{

std::string MessageEncoderFactory::encodeMessage(const AISMessage &message)
{
    BitBufferEncoder encoder;

    // 根据消息类型调用相应的编码方法
    switch (message.type)
    {
    case AISMessageType::POSITION_REPORT_CLASS_A:
        return encodeType1(static_cast<const PositionReport &>(message));
    case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED:
        return encodeType2(static_cast<const PositionReportAssigned &>(message));
    case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE:
        return encodeType3(static_cast<const PositionReportResponse &>(message));
    case AISMessageType::BASE_STATION_REPORT:
        return encodeType4(static_cast<const BaseStationReport &>(message));
    case AISMessageType::STATIC_VOYAGE_DATA:
        return encodeType5(static_cast<const StaticVoyageData &>(message));
    case AISMessageType::BINARY_ADDRESSED_MESSAGE:
        return encodeType6(static_cast<const BinaryAddressedMessage &>(message));
    case AISMessageType::BINARY_ACKNOWLEDGE:
        return encodeType7(static_cast<const BinaryAcknowledge &>(message));
    case AISMessageType::BINARY_BROADCAST_MESSAGE:
        return encodeType8(static_cast<const BinaryBroadcastMessage &>(message));
    case AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT:
        return encodeType9(static_cast<const StandardSARAircraftReport &>(message));
    case AISMessageType::UTC_DATE_INQUIRY:
        return encodeType10(static_cast<const UTCDateInquiry &>(message));
    case AISMessageType::UTC_DATE_RESPONSE:
        return encodeType11(static_cast<const UTCDateResponse &>(message));
    case AISMessageType::ADDRESSED_SAFETY_MESSAGE:
        return encodeType12(static_cast<const AddressedSafetyMessage &>(message));
    case AISMessageType::SAFETY_ACKNOWLEDGE:
        return encodeType13(static_cast<const SafetyAcknowledge &>(message));
    case AISMessageType::SAFETY_RELATED_BROADCAST:
        return encodeType14(static_cast<const SafetyRelatedBroadcast &>(message));
    case AISMessageType::INTERROGATION:
        return encodeType15(static_cast<const Interrogation &>(message));
    case AISMessageType::ASSIGNMENT_MODE_COMMAND:
        return encodeType16(static_cast<const AssignmentModeCommand &>(message));
    case AISMessageType::DGNSS_BINARY_BROADCAST:
        return encodeType17(static_cast<const DGNSSBinaryBroadcast &>(message));
    case AISMessageType::STANDARD_CLASS_B_CS_POSITION:
        return encodeType18(static_cast<const StandardClassBReport &>(message));
    case AISMessageType::EXTENDED_CLASS_B_CS_POSITION:
        return encodeType19(static_cast<const ExtendedClassBReport &>(message));
    case AISMessageType::DATA_LINK_MANAGEMENT:
        return encodeType20(static_cast<const DataLinkManagement &>(message));
    case AISMessageType::AID_TO_NAVIGATION_REPORT:
        return encodeType21(static_cast<const AidToNavigationReport &>(message));
    case AISMessageType::CHANNEL_MANAGEMENT:
        return encodeType22(static_cast<const ChannelManagement &>(message));
    case AISMessageType::GROUP_ASSIGNMENT_COMMAND:
        return encodeType23(static_cast<const GroupAssignmentCommand &>(message));
    case AISMessageType::STATIC_DATA_REPORT:
        return encodeType24(static_cast<const StaticDataReport &>(message));
    case AISMessageType::SINGLE_SLOT_BINARY_MESSAGE:
        return encodeType25(static_cast<const SingleSlotBinaryMessage &>(message));
    case AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE:
        return encodeType26(static_cast<const MultipleSlotBinaryMessage &>(message));
    case AISMessageType::POSITION_REPORT_LONG_RANGE:
        return encodeType27(static_cast<const LongRangePositionReport &>(message));
    default:
        throw std::invalid_argument("Unsupported AIS message type: " +
                                    std::to_string(static_cast<int>(message.type)));
    }
}

// ============ 类型1-3：A类位置报告 ============

std::string MessageEncoderFactory::encodeType1(const PositionReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(1, 6);                       // 消息类型
    encoder.putUInt32(msg.repeatIndicator, 2);     // 重复指示器
    encoder.putUInt32(msg.mmsi, 30);               // MMSI
    encoder.putUInt32(msg.navigationStatus, 4);    // 导航状态
    encoder.putRateOfTurn(msg.rateOfTurn, 8);      // 转向率
    encoder.putSpeed(msg.speedOverGround, 10);     // 对地速度
    encoder.putBool(msg.positionAccuracy);         // 位置精度
    encoder.putLongitude(msg.longitude, 28);       // 经度
    encoder.putLatitude(msg.latitude, 27);         // 纬度
    encoder.putCourse(msg.courseOverGround, 12);   // 对地航向
    encoder.putUInt32(msg.trueHeading, 9);         // 真航向
    encoder.putUInt32(msg.timestampUTC, 6);        // UTC时间戳
    encoder.putUInt32(msg.specialManeuver, 2);     // 特殊操纵指示
    encoder.putPadding(3);                         // spare
    encoder.putBool(msg.raimFlag);                 // RAIM标志
    encoder.putUInt32(msg.communicationState, 19); // 通信状态

    return encoder.getBinaryString();
}

std::string MessageEncoderFactory::encodeType2(const PositionReportAssigned &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(2, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.navigationStatus, 4);
    encoder.putRateOfTurn(msg.rateOfTurn, 8);
    encoder.putSpeed(msg.speedOverGround, 10);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putCourse(msg.courseOverGround, 12);
    encoder.putUInt32(msg.trueHeading, 9);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putUInt32(msg.specialManeuver, 2);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);

    return encoder.getBinaryString();
}

std::string MessageEncoderFactory::encodeType3(const PositionReportResponse &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(3, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.navigationStatus, 4);
    encoder.putRateOfTurn(msg.rateOfTurn, 8);
    encoder.putSpeed(msg.speedOverGround, 10);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putCourse(msg.courseOverGround, 12);
    encoder.putUInt32(msg.trueHeading, 9);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putUInt32(msg.specialManeuver, 2);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);

    return encoder.getBinaryString();
}

// ============ 类型4：基站报告 ============

std::string MessageEncoderFactory::encodeType4(const BaseStationReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(4, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.year, 14);
    encoder.putUInt32(msg.month, 4);
    encoder.putUInt32(msg.day, 5);
    encoder.putUInt32(msg.hour, 5);
    encoder.putUInt32(msg.minute, 6);
    encoder.putUInt32(msg.second, 6);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putUInt32(msg.epfdType, 4);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);

    return encoder.getBinaryString();
}

// ============ 类型5：静态和航程相关数据 ============

std::string MessageEncoderFactory::encodeType5(const StaticVoyageData &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(5, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.aisVersion, 2);
    encoder.putUInt32(msg.imoNumber, 30);
    encoder.putString(msg.callSign, 42);
    encoder.putString(msg.vesselName, 120);
    encoder.putUInt32(msg.shipType, 8);
    encoder.putUInt32(msg.dimensionToBow, 9);
    encoder.putUInt32(msg.dimensionToStern, 9);
    encoder.putUInt32(msg.dimensionToPort, 6);
    encoder.putUInt32(msg.dimensionToStarboard, 6);
    encoder.putUInt32(msg.epfdType, 4);
    encoder.putUInt32(msg.month, 4);
    encoder.putUInt32(msg.day, 5);
    encoder.putUInt32(msg.hour, 5);
    encoder.putUInt32(msg.minute, 6);
    encoder.putUInt32(static_cast<uint32_t>(msg.draught * 10.0), 8);
    encoder.putString(msg.destination, 120);
    encoder.putBool(msg.dte);
    encoder.putPadding(1); // spare

    return encoder.getBinaryString();
}

// ============ 类型6：二进制编址消息 ============

std::string MessageEncoderFactory::encodeType6(const BinaryAddressedMessage &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(6, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.sequenceNumber, 2);
    encoder.putUInt32(msg.destinationMmsi, 30);
    encoder.putBool(msg.retransmitFlag);
    encoder.putPadding(1); // spare
    encoder.putUInt32(msg.designatedAreaCode, 10);
    encoder.putUInt32(msg.functionalId, 6);

    // 编码二进制数据
    if (!msg.binaryData.empty())
    {
        encodeBinaryData(encoder, msg.binaryData, msg.binaryData.size() * 8);
    }

    return encoder.getBinaryString();
}

// ============ 类型7：二进制确认 ============

std::string MessageEncoderFactory::encodeType7(const BinaryAcknowledge &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(7, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.sequenceNumber, 2);

    // 编码目标MMSI（最多4个）
    if (msg.destinationMmsi1 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi1, 30);
    }
    if (msg.destinationMmsi2 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi2, 30);
    }
    if (msg.destinationMmsi3 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi3, 30);
    }
    if (msg.destinationMmsi4 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi4, 30);
    }

    return encoder.getBinaryString();
}

// ============ 类型8：二进制广播消息 ============

std::string MessageEncoderFactory::encodeType8(const BinaryBroadcastMessage &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(8, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare, 2);
    encoder.putUInt32(msg.designatedAreaCode, 10);
    encoder.putUInt32(msg.functionalId, 6);

    // 编码二进制数据
    if (!msg.binaryData.empty())
    {
        encodeBinaryData(encoder, msg.binaryData, msg.binaryData.size() * 8);
    }

    return encoder.getBinaryString();
}

// ============ 类型9：标准搜救飞机位置报告 ============

std::string MessageEncoderFactory::encodeType9(const StandardSARAircraftReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(9, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.altitude, 12);
    encoder.putSpeed(msg.speedOverGround, 10);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putCourse(msg.courseOverGround, 12);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putUInt32(msg.spare, 2);
    encoder.putPadding(8); // spare区域
    encoder.putBool(msg.assignedModeFlag);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);

    return encoder.getBinaryString();
}

// ============ 类型10：UTC和日期询问 ============

std::string MessageEncoderFactory::encodeType10(const UTCDateInquiry &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(10, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putUInt32(msg.destinationMmsi, 30);
    encoder.putUInt32(msg.spare2, 2);

    return encoder.getBinaryString();
}

// ============ 类型11：UTC和日期响应 ============

std::string MessageEncoderFactory::encodeType11(const UTCDateResponse &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(11, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.year, 14);
    encoder.putUInt32(msg.month, 4);
    encoder.putUInt32(msg.day, 5);
    encoder.putUInt32(msg.hour, 5);
    encoder.putUInt32(msg.minute, 6);
    encoder.putUInt32(msg.second, 6);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putUInt32(msg.epfdType, 4);
    encoder.putUInt32(msg.spare, 10);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);

    return encoder.getBinaryString();
}

// ============ 类型12：安全相关编址消息 ============

std::string MessageEncoderFactory::encodeType12(const AddressedSafetyMessage &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(12, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.sequenceNumber, 2);
    encoder.putUInt32(msg.destinationMmsi, 30);
    encoder.putBool(msg.retransmitFlag);
    encoder.putUInt32(msg.spare, 1);

    // 编码安全文本
    if (!msg.safetyText.empty())
    {
        size_t textBits = msg.safetyText.length() * 6;
        encoder.putString(msg.safetyText, textBits);
    }

    return encoder.getBinaryString();
}

// ============ 类型13：安全相关确认 ============

std::string MessageEncoderFactory::encodeType13(const SafetyAcknowledge &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(13, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.sequenceNumber, 2);

    // 编码目标MMSI（最多4个）
    if (msg.destinationMmsi1 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi1, 30);
    }
    if (msg.destinationMmsi2 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi2, 30);
    }
    if (msg.destinationMmsi3 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi3, 30);
    }
    if (msg.destinationMmsi4 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi4, 30);
    }

    encoder.putUInt32(msg.spare, 2);

    return encoder.getBinaryString();
}

// ============ 类型14：安全相关广播消息 ============

std::string MessageEncoderFactory::encodeType14(const SafetyRelatedBroadcast &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(14, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare, 2);

    // 编码安全文本
    if (!msg.safetyText.empty())
    {
        size_t textBits = msg.safetyText.length() * 6;
        encoder.putString(msg.safetyText, textBits);
    }

    return encoder.getBinaryString();
}

// ============ 类型15：询问 ============

std::string MessageEncoderFactory::encodeType15(const Interrogation &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(15, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putUInt32(msg.destinationMmsi1, 30);
    encoder.putUInt32(msg.messageType1_1, 6);
    encoder.putUInt32(msg.slotOffset1_1, 12);
    encoder.putUInt32(msg.spare2, 2);

    // 第二个询问（如果存在）
    if (msg.messageType1_2 != 0)
    {
        encoder.putUInt32(msg.messageType1_2, 6);
        encoder.putUInt32(msg.slotOffset1_2, 12);
        encoder.putUInt32(msg.spare3, 2);
    }

    // 第二个目标（如果存在）
    if (msg.destinationMmsi2 != 0)
    {
        encoder.putUInt32(msg.destinationMmsi2, 30);
        encoder.putUInt32(msg.messageType2, 6);
        encoder.putUInt32(msg.slotOffset2, 12);
        encoder.putUInt32(msg.spare4, 2);
    }

    return encoder.getBinaryString();
}

// ============ 类型16：分配模式命令 ============

std::string MessageEncoderFactory::encodeType16(const AssignmentModeCommand &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(16, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putUInt32(msg.destinationMmsiA, 30);
    encoder.putUInt32(msg.offsetA, 12);
    encoder.putUInt32(msg.incrementA, 10);
    encoder.putUInt32(msg.spare2, 4);

    // 第二个分配（如果存在）
    if (msg.destinationMmsiB != 0)
    {
        encoder.putUInt32(msg.destinationMmsiB, 30);
        encoder.putUInt32(msg.offsetB, 12);
        encoder.putUInt32(msg.incrementB, 10);
        encoder.putUInt32(msg.spare3, 4);
    }

    return encoder.getBinaryString();
}

// ============ 类型17：DGNSS二进制广播消息 ============

std::string MessageEncoderFactory::encodeType17(const DGNSSBinaryBroadcast &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(17, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putLongitude(msg.longitude, 18);
    encoder.putLatitude(msg.latitude, 17);
    encoder.putUInt32(msg.spare2, 5);

    // 编码DGNSS数据
    if (!msg.dgnssData.empty())
    {
        encodeBinaryData(encoder, msg.dgnssData, msg.dgnssData.size() * 8);
    }

    return encoder.getBinaryString();
}

// ============ 类型18：标准B类设备位置报告 ============

std::string MessageEncoderFactory::encodeType18(const StandardClassBReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(18, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 8);
    encoder.putSpeed(msg.speedOverGround, 10);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putCourse(msg.courseOverGround, 12);
    encoder.putUInt32(msg.trueHeading, 9);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putUInt32(msg.spare2, 2);
    encoder.putUInt32(msg.csUnit, 2);
    encoder.putBool(msg.displayFlag);
    encoder.putBool(msg.dscFlag);
    encoder.putBool(msg.bandFlag);
    encoder.putBool(msg.message22Flag);
    encoder.putBool(msg.assignedModeFlag);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.communicationState, 19);
    encoder.putUInt32(msg.spare3, 1);

    return encoder.getBinaryString();
}

// ============ 类型19：扩展B类设备位置报告 ============

std::string MessageEncoderFactory::encodeType19(const ExtendedClassBReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(19, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 8);
    encoder.putSpeed(msg.speedOverGround, 10);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putCourse(msg.courseOverGround, 12);
    encoder.putUInt32(msg.trueHeading, 9);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putUInt32(msg.spare2, 4);
    encoder.putString(msg.vesselName, 120);
    encoder.putUInt32(msg.shipType, 8);
    encoder.putUInt32(msg.dimensionToBow, 9);
    encoder.putUInt32(msg.dimensionToStern, 9);
    encoder.putUInt32(msg.dimensionToPort, 6);
    encoder.putUInt32(msg.dimensionToStarboard, 6);
    encoder.putUInt32(msg.epfdType, 4);
    encoder.putUInt32(msg.spare3, 1);
    encoder.putBool(msg.raimFlag);
    encoder.putBool(msg.dte);
    encoder.putBool(msg.assignedModeFlag);
    encoder.putUInt32(msg.spare4, 4);

    return encoder.getBinaryString();
}

// ============ 类型20：数据链路管理消息 ============

std::string MessageEncoderFactory::encodeType20(const DataLinkManagement &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(20, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);

    // 编码偏移配置（最多4个）
    if (msg.offsetNumber1 != 0)
    {
        encoder.putUInt32(msg.offsetNumber1, 12);
        encoder.putUInt32(msg.reservedSlots1, 4);
        encoder.putUInt32(msg.timeout1, 3);
        encoder.putUInt32(msg.increment1, 11);
    }

    if (msg.offsetNumber2 != 0)
    {
        encoder.putUInt32(msg.offsetNumber2, 12);
        encoder.putUInt32(msg.reservedSlots2, 4);
        encoder.putUInt32(msg.timeout2, 3);
        encoder.putUInt32(msg.increment2, 11);
    }

    if (msg.offsetNumber3 != 0)
    {
        encoder.putUInt32(msg.offsetNumber3, 12);
        encoder.putUInt32(msg.reservedSlots3, 4);
        encoder.putUInt32(msg.timeout3, 3);
        encoder.putUInt32(msg.increment3, 11);
    }

    if (msg.offsetNumber4 != 0)
    {
        encoder.putUInt32(msg.offsetNumber4, 12);
        encoder.putUInt32(msg.reservedSlots4, 4);
        encoder.putUInt32(msg.timeout4, 3);
        encoder.putUInt32(msg.increment4, 11);
    }

    encoder.putUInt32(msg.spare2, 6);

    return encoder.getBinaryString();
}

// ============ 类型21：助航设备报告 ============

std::string MessageEncoderFactory::encodeType21(const AidToNavigationReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(21, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.aidType, 5);
    encoder.putString(msg.name, 120);
    encoder.putBool(msg.positionAccuracy);
    encoder.putLongitude(msg.longitude, 28);
    encoder.putLatitude(msg.latitude, 27);
    encoder.putUInt32(msg.dimensionToBow, 9);
    encoder.putUInt32(msg.dimensionToStern, 9);
    encoder.putUInt32(msg.dimensionToPort, 6);
    encoder.putUInt32(msg.dimensionToStarboard, 6);
    encoder.putUInt32(msg.epfdType, 4);
    encoder.putUInt32(msg.timestampUTC, 6);
    encoder.putBool(msg.offPositionIndicator);
    encoder.putUInt32(msg.regional, 8);
    encoder.putBool(msg.raimFlag);
    encoder.putBool(msg.virtualAidFlag);
    encoder.putBool(msg.assignedModeFlag);

    // 名称扩展
    if (!msg.nameExtension.empty())
    {
        size_t extensionBits = msg.nameExtension.length() * 6;
        encoder.putString(msg.nameExtension, extensionBits);
    }

    encoder.putUInt32(msg.spare, 2);

    return encoder.getBinaryString();
}

// ============ 类型22：信道管理 ============

std::string MessageEncoderFactory::encodeType22(const ChannelManagement &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(22, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putUInt32(msg.channelA, 12);
    encoder.putUInt32(msg.channelB, 12);
    encoder.putUInt32(msg.txRxMode, 4);
    encoder.putUInt32(msg.power, 1);

    // 检查是否有地理区域定义
    bool hasGeoArea = (msg.longitude1 != 0.0 || msg.latitude1 != 0.0 ||
                       msg.longitude2 != 0.0 || msg.latitude2 != 0.0);

    encoder.putBool(hasGeoArea);

    if (hasGeoArea)
    {
        encoder.putLongitude(msg.longitude1, 18);
        encoder.putLatitude(msg.latitude1, 17);
        encoder.putLongitude(msg.longitude2, 18);
        encoder.putLatitude(msg.latitude2, 17);
        encoder.putUInt32(msg.addressedOrBroadcast, 1);
        encoder.putUInt32(msg.bandwidthA, 2);
        encoder.putUInt32(msg.bandwidthB, 2);
        encoder.putUInt32(msg.zoneSize, 3);
    }
    else
    {
        encoder.putUInt32(msg.addressedOrBroadcast, 1);
        encoder.putUInt32(msg.bandwidthA, 2);
        encoder.putUInt32(msg.bandwidthB, 2);
        encoder.putUInt32(msg.zoneSize, 3);
        encoder.putPadding(70); // 填充无地理区域时的剩余位
    }

    encoder.putUInt32(msg.spare2, 2);

    return encoder.getBinaryString();
}

// ============ 类型23：组分配命令 ============

std::string MessageEncoderFactory::encodeType23(const GroupAssignmentCommand &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(23, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.spare1, 2);
    encoder.putLongitude(msg.longitude1, 18);
    encoder.putLatitude(msg.latitude1, 17);
    encoder.putLongitude(msg.longitude2, 18);
    encoder.putLatitude(msg.latitude2, 17);
    encoder.putUInt32(msg.stationType, 4);
    encoder.putUInt32(msg.shipType, 8);
    encoder.putUInt32(msg.txRxMode, 2);
    encoder.putUInt32(msg.reportingInterval, 4);
    encoder.putUInt32(msg.quietTime, 4);
    encoder.putUInt32(msg.spare2, 6);

    return encoder.getBinaryString();
}

// ============ 类型24：静态数据报告 ============

std::string MessageEncoderFactory::encodeType24(const StaticDataReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(24, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putUInt32(msg.partNumber, 2);

    if (msg.partNumber == 0)
    {
        // 部分A：船名
        encoder.putString(msg.vesselName, 120);
        encoder.putUInt32(msg.spare, 8);
    }
    else
    {
        // 部分B：其他静态数据
        encoder.putUInt32(msg.shipType, 8);
        encoder.putString(msg.vendorId, 42);
        encoder.putString(msg.callSign, 42);
        encoder.putUInt32(msg.dimensionToBow, 9);
        encoder.putUInt32(msg.dimensionToStern, 9);
        encoder.putUInt32(msg.dimensionToPort, 6);
        encoder.putUInt32(msg.dimensionToStarboard, 6);
        encoder.putUInt32(msg.mothershipMmsi, 30);
        encoder.putUInt32(msg.spare, 6);
    }

    return encoder.getBinaryString();
}

// ============ 类型25：单时隙二进制消息 ============

std::string MessageEncoderFactory::encodeType25(const SingleSlotBinaryMessage &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(25, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putBool(msg.addressed);
    encoder.putBool(msg.structured);

    if (msg.addressed)
    {
        encoder.putUInt32(msg.destinationMmsi, 30);
    }

    if (msg.structured)
    {
        encoder.putUInt32(msg.designatedAreaCode, 10);
        encoder.putUInt32(msg.functionalId, 6);
    }

    // 编码二进制数据
    if (!msg.binaryData.empty())
    {
        encodeBinaryData(encoder, msg.binaryData, msg.binaryData.size() * 8);
    }

    encoder.putUInt32(msg.spare, 2);

    return encoder.getBinaryString();
}

// ============ 类型26：多时隙二进制消息 ============

std::string MessageEncoderFactory::encodeType26(const MultipleSlotBinaryMessage &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(26, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putBool(msg.addressed);
    encoder.putBool(msg.structured);

    if (msg.addressed)
    {
        encoder.putUInt32(msg.destinationMmsi, 30);
    }

    if (msg.structured)
    {
        encoder.putUInt32(msg.designatedAreaCode, 10);
        encoder.putUInt32(msg.functionalId, 6);
    }

    // 编码二进制数据（保留最后16位用于通信状态）
    size_t dataBits = encoder.getBinaryString().length() + (msg.binaryData.size() * 8);
    size_t remainingBits = 256 - dataBits - 16; // 假设最大256位

    if (!msg.binaryData.empty() && remainingBits > 0)
    {
        encodeBinaryData(encoder, msg.binaryData, remainingBits);
    }

    // 通信状态标志
    encoder.putUInt32(msg.commStateFlag, 16);
    encoder.putUInt32(msg.spare, 2);

    return encoder.getBinaryString();
}

// ============ 类型27：长距离位置报告 ============

std::string MessageEncoderFactory::encodeType27(const LongRangePositionReport &msg)
{
    BitBufferEncoder encoder;

    encoder.putUInt32(27, 6);
    encoder.putUInt32(msg.repeatIndicator, 2);
    encoder.putUInt32(msg.mmsi, 30);
    encoder.putBool(msg.positionAccuracy);
    encoder.putBool(msg.raimFlag);
    encoder.putUInt32(msg.navigationStatus, 4);
    encoder.putLongitude(msg.longitude, 18);
    encoder.putLatitude(msg.latitude, 17);
    encoder.putSpeed(msg.speedOverGround, 6);
    encoder.putCourse(msg.courseOverGround, 9);
    encoder.putBool(msg.gnssPositionStatus);
    encoder.putBool(msg.assignedModeFlag);
    encoder.putUInt32(msg.spare, 4);

    return encoder.getBinaryString();
}

// ============ 通用二进制数据编码 ============

void MessageEncoderFactory::encodeBinaryData(BitBufferEncoder &encoder,
                                             const std::vector<uint8_t> &data,
                                             size_t maxBits)
{
    size_t bitsToEncode = std::min(maxBits, data.size() * 8);

    for (size_t i = 0; i < bitsToEncode; i++)
    {
        size_t byteIndex = i / 8;
        size_t bitIndex = 7 - (i % 8); // 从最高位开始
        bool bit = (data[byteIndex] & (1 << bitIndex)) != 0;
        encoder.putBool(bit);
    }

    // 填充剩余位
    if (bitsToEncode < maxBits)
    {
        encoder.putPadding(maxBits - bitsToEncode, false);
    }
}

} // namespace ais