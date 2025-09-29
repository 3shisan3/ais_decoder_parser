#include "messages/message_factory.h"

#include "core/bit_buffer.h"
#include "messages/type_definitions.h"

namespace ais
{

std::unique_ptr<AISMessage> MessageFactory::createMessage(BitBuffer &bits)
{
    if (bits.remaining() < 6)
        return nullptr;

    int messageType = bits.getUInt32(0, 6);
    bits.setPosition(0);

    switch (static_cast<AISMessageType>(messageType))
    {
    case AISMessageType::POSITION_REPORT_CLASS_A:
        return parseType1(bits);
    case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED:
        return parseType2(bits);
    case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE:
        return parseType3(bits);
    case AISMessageType::BASE_STATION_REPORT:
        return parseType4(bits);
    case AISMessageType::STATIC_VOYAGE_DATA:
        return parseType5(bits);
    case AISMessageType::BINARY_ADDRESSED_MESSAGE:
        return parseType6(bits);
    case AISMessageType::BINARY_ACKNOWLEDGE:
        return parseType7(bits);
    case AISMessageType::BINARY_BROADCAST_MESSAGE:
        return parseType8(bits);
    case AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT:
        return parseType9(bits);
    case AISMessageType::UTC_DATE_INQUIRY:
        return parseType10(bits);
    case AISMessageType::UTC_DATE_RESPONSE:
        return parseType11(bits);
    case AISMessageType::ADDRESSED_SAFETY_MESSAGE:
        return parseType12(bits);
    case AISMessageType::SAFETY_ACKNOWLEDGE:
        return parseType13(bits);
    case AISMessageType::SAFETY_RELATED_BROADCAST:
        return parseType14(bits);
    case AISMessageType::INTERROGATION:
        return parseType15(bits);
    case AISMessageType::ASSIGNMENT_MODE_COMMAND:
        return parseType16(bits);
    case AISMessageType::DGNSS_BINARY_BROADCAST:
        return parseType17(bits);
    case AISMessageType::STANDARD_CLASS_B_CS_POSITION:
        return parseType18(bits);
    case AISMessageType::EXTENDED_CLASS_B_CS_POSITION:
        return parseType19(bits);
    case AISMessageType::DATA_LINK_MANAGEMENT:
        return parseType20(bits);
    case AISMessageType::AID_TO_NAVIGATION_REPORT:
        return parseType21(bits);
    case AISMessageType::CHANNEL_MANAGEMENT:
        return parseType22(bits);
    case AISMessageType::GROUP_ASSIGNMENT_COMMAND:
        return parseType23(bits);
    case AISMessageType::STATIC_DATA_REPORT:
        return parseType24(bits);
    case AISMessageType::SINGLE_SLOT_BINARY_MESSAGE:
        return parseType25(bits);
    case AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE:
        return parseType26(bits);
    case AISMessageType::POSITION_REPORT_LONG_RANGE:
        return parseType27(bits);
    default:
        return nullptr;
    }
}

/*************** 类型1-27的解析实现 ***************/

// 类型1：A类位置报告
std::unique_ptr<AISMessage> MessageFactory::parseType1(BitBuffer &bits)
{
    auto msg = std::make_unique<PositionReport>();

    msg->type = AISMessageType::POSITION_REPORT_CLASS_A;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->navigationStatus = bits.getUInt32(38, 4);
    msg->rateOfTurn = bits.getRateOfTurn(42, 8);
    msg->speedOverGround = bits.getSpeed(50, 10);
    msg->positionAccuracy = bits.getBool(60);
    msg->longitude = bits.getLongitude(61, 28);
    msg->latitude = bits.getLatitude(89, 27);
    msg->courseOverGround = bits.getCourse(116, 12);
    msg->trueHeading = bits.getUInt32(128, 9);
    msg->timestampUTC = bits.getUInt32(137, 6);
    msg->specialManeuver = bits.getUInt32(143, 2);
    
    // 跳过spare位 (145-147)
    bits.setPosition(148);

    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);

    return msg;
}

// 类型2：A类位置报告（分配时隙）
std::unique_ptr<AISMessage> MessageFactory::parseType2(BitBuffer &bits)
{
    auto msg = std::make_unique<PositionReportAssigned>();

    msg->type = AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->navigationStatus = bits.getUInt32(38, 4);
    msg->rateOfTurn = bits.getRateOfTurn(42, 8);
    msg->speedOverGround = bits.getSpeed(50, 10);
    msg->positionAccuracy = bits.getBool(60);
    msg->longitude = bits.getLongitude(61, 28);
    msg->latitude = bits.getLatitude(89, 27);
    msg->courseOverGround = bits.getCourse(116, 12);
    msg->trueHeading = bits.getUInt32(128, 9);
    msg->timestampUTC = bits.getUInt32(137, 6);
    msg->specialManeuver = bits.getUInt32(143, 2);
    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);

    return msg;
}

// 类型3：A类位置报告（响应询问）
std::unique_ptr<AISMessage> MessageFactory::parseType3(BitBuffer &bits)
{
    auto msg = std::make_unique<PositionReportResponse>();

    msg->type = AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->navigationStatus = bits.getUInt32(38, 4);
    msg->rateOfTurn = bits.getRateOfTurn(42, 8);
    msg->speedOverGround = bits.getSpeed(50, 10);
    msg->positionAccuracy = bits.getBool(60);
    msg->longitude = bits.getLongitude(61, 28);
    msg->latitude = bits.getLatitude(89, 27);
    msg->courseOverGround = bits.getCourse(116, 12);
    msg->trueHeading = bits.getUInt32(128, 9);
    msg->timestampUTC = bits.getUInt32(137, 6);
    msg->specialManeuver = bits.getUInt32(143, 2);
    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);

    return msg;
}

// 类型4：基站报告
std::unique_ptr<AISMessage> MessageFactory::parseType4(BitBuffer &bits)
{
    auto msg = std::make_unique<BaseStationReport>();

    msg->type = AISMessageType::BASE_STATION_REPORT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->year = bits.getUInt32(38, 14);
    msg->month = bits.getUInt32(52, 4);
    msg->day = bits.getUInt32(56, 5);
    msg->hour = bits.getUInt32(61, 5);
    msg->minute = bits.getUInt32(66, 6);
    msg->second = bits.getUInt32(72, 6);
    msg->positionAccuracy = bits.getBool(78);
    msg->longitude = bits.getLongitude(79, 28);
    msg->latitude = bits.getLatitude(107, 27);
    msg->epfdType = bits.getUInt32(134, 4);
    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);

    return msg;
}

// 类型5：静态和航程相关数据
std::unique_ptr<AISMessage> MessageFactory::parseType5(BitBuffer &bits)
{
    auto msg = std::make_unique<StaticVoyageData>();

    msg->type = AISMessageType::STATIC_VOYAGE_DATA;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->aisVersion = bits.getUInt32(38, 2);
    msg->imoNumber = bits.getUInt32(40, 30);
    msg->callSign = bits.getString(70, 42);
    msg->vesselName = bits.getString(112, 120);
    msg->shipType = bits.getUInt32(232, 8);
    msg->dimensionToBow = bits.getUInt32(240, 9);
    msg->dimensionToStern = bits.getUInt32(249, 9);
    msg->dimensionToPort = bits.getUInt32(258, 6);
    msg->dimensionToStarboard = bits.getUInt32(264, 6);
    msg->epfdType = bits.getUInt32(270, 4);
    msg->month = bits.getUInt32(274, 4);
    msg->day = bits.getUInt32(278, 5);
    msg->hour = bits.getUInt32(283, 5);
    msg->minute = bits.getUInt32(288, 6);
    msg->draught = bits.getUInt32(294, 8) / 10.0;
    msg->destination = bits.getString(302, 120);
    msg->dte = bits.getBool(422);

    return msg;
}

// 类型6：二进制编址消息
std::unique_ptr<AISMessage> MessageFactory::parseType6(BitBuffer &bits)
{
    auto msg = std::make_unique<BinaryAddressedMessage>();

    msg->type = AISMessageType::BINARY_ADDRESSED_MESSAGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->sequenceNumber = bits.getUInt32(38, 2);
    msg->destinationMmsi = bits.getUInt32(40, 30);
    msg->retransmitFlag = bits.getBool(70);

    // 跳过spare位
    bits.skip(1);

    msg->designatedAreaCode = bits.getUInt32(72, 10);
    msg->functionalId = bits.getUInt32(82, 6);

    // 使用流式读取剩余数据
    int remainingBits = bits.remaining();
    if (remainingBits > 0)
    {
        for (int i = 0; i < remainingBits; i += 8)
        {
            int bitsToRead = std::min(8, remainingBits - i);
            uint8_t byte = static_cast<uint8_t>(bits.getUInt32(bitsToRead));
            msg->binaryData.push_back(byte);
        }
    }

    return msg;
}

// 类型7：二进制确认
std::unique_ptr<AISMessage> MessageFactory::parseType7(BitBuffer &bits)
{
    auto msg = std::make_unique<BinaryAcknowledge>();

    msg->type = AISMessageType::BINARY_ACKNOWLEDGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->sequenceNumber = bits.getUInt32(38, 2);
    
    // 读取目标MMSI，最多4个
    int currentPos = 40;
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi1 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi2 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi3 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi4 = bits.getUInt32(currentPos, 30);
    }

    return msg;
}

// 类型8：二进制广播消息 - 修正数据提取
std::unique_ptr<AISMessage> MessageFactory::parseType8(BitBuffer &bits)
{
    auto msg = std::make_unique<BinaryBroadcastMessage>();

    msg->type = AISMessageType::BINARY_BROADCAST_MESSAGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare = bits.getUInt32(38, 2);

    msg->designatedAreaCode = bits.getUInt32(40, 10);
    msg->functionalId = bits.getUInt32(50, 6);

    // 使用流式读取剩余数据
    int remainingBits = bits.remaining();
    if (remainingBits > 0)
    {
        for (int i = 0; i < remainingBits; i += 8)
        {
            int bitsToRead = std::min(8, remainingBits - i);
            uint8_t byte = static_cast<uint8_t>(bits.getUInt32(bitsToRead));
            msg->binaryData.push_back(byte);
        }
    }

    return msg;
}

// 类型9：标准搜救飞机位置报告 - 修正位偏移错误
std::unique_ptr<AISMessage> MessageFactory::parseType9(BitBuffer &bits)
{
    auto msg = std::make_unique<StandardSARAircraftReport>();

    msg->type = AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->altitude = bits.getUInt32(38, 12);
    msg->speedOverGround = bits.getSpeed(50, 10);
    msg->positionAccuracy = bits.getBool(60);
    msg->longitude = bits.getLongitude(61, 28);
    msg->latitude = bits.getLatitude(89, 27);
    msg->courseOverGround = bits.getCourse(116, 12);
    msg->timestampUTC = bits.getUInt32(128, 6);
    msg->spare = bits.getUInt32(134, 2);
    
    // 跳过136-145位的spare区域（10位）
    bits.skip(10); // 跳过134-143位（已经读取了134-135）
    
    msg->assignedModeFlag = bits.getBool(144);
    msg->raimFlag = bits.getBool(145);
    msg->communicationState = bits.getUInt32(146, 19); // 从146位开始

    return msg;
}

// 类型10：UTC和日期询问
std::unique_ptr<AISMessage> MessageFactory::parseType10(BitBuffer &bits)
{
    auto msg = std::make_unique<UTCDateInquiry>();

    msg->type = AISMessageType::UTC_DATE_INQUIRY;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->destinationMmsi = bits.getUInt32(40, 30);
    msg->spare2 = bits.getUInt32(70, 2);

    return msg;
}

// 类型11：UTC和日期响应
std::unique_ptr<AISMessage> MessageFactory::parseType11(BitBuffer &bits)
{
    auto msg = std::make_unique<UTCDateResponse>();

    msg->type = AISMessageType::UTC_DATE_RESPONSE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->year = bits.getUInt32(38, 14);
    msg->month = bits.getUInt32(52, 4);
    msg->day = bits.getUInt32(56, 5);
    msg->hour = bits.getUInt32(61, 5);
    msg->minute = bits.getUInt32(66, 6);
    msg->second = bits.getUInt32(72, 6);
    msg->positionAccuracy = bits.getBool(78);
    msg->longitude = bits.getLongitude(79, 28);
    msg->latitude = bits.getLatitude(107, 27);
    msg->epfdType = bits.getUInt32(134, 4);
    msg->spare = bits.getUInt32(138, 10);
    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);

    return msg;
}

// 类型12：安全相关编址消息
std::unique_ptr<AISMessage> MessageFactory::parseType12(BitBuffer &bits)
{
    auto msg = std::make_unique<AddressedSafetyMessage>();

    msg->type = AISMessageType::ADDRESSED_SAFETY_MESSAGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->sequenceNumber = bits.getUInt32(38, 2);
    msg->destinationMmsi = bits.getUInt32(40, 30);
    msg->retransmitFlag = bits.getBool(70);
    msg->spare = bits.getUInt32(71, 1);

    // 提取安全文本 - 从72位开始
    int textStart = 72;
    int textBits = bits.remaining() - textStart;
    if (textBits > 0)
    {
        msg->safetyText = bits.getString(textStart, textBits);
    }

    return msg;
}

// 类型13：安全相关确认
std::unique_ptr<AISMessage> MessageFactory::parseType13(BitBuffer &bits)
{
    auto msg = std::make_unique<SafetyAcknowledge>();

    msg->type = AISMessageType::SAFETY_ACKNOWLEDGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->sequenceNumber = bits.getUInt32(38, 2);
    
    // 读取目标MMSI，最多4个
    int currentPos = 40;
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi1 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi2 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi3 = bits.getUInt32(currentPos, 30);
        currentPos += 30;
    }
    
    if (bits.remaining() >= currentPos + 30)
    {
        msg->destinationMmsi4 = bits.getUInt32(currentPos, 30);
    }

    return msg;
}

// 类型14：安全相关广播消息
std::unique_ptr<AISMessage> MessageFactory::parseType14(BitBuffer &bits)
{
    auto msg = std::make_unique<SafetyRelatedBroadcast>();

    msg->type = AISMessageType::SAFETY_RELATED_BROADCAST;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare = bits.getUInt32(38, 2);

    // 提取安全文本 - 从40位开始
    int textStart = 40;
    int textBits = bits.remaining() - textStart;
    if (textBits > 0)
    {
        msg->safetyText = bits.getString(textStart, textBits);
    }

    return msg;
}

// 类型15：询问
std::unique_ptr<AISMessage> MessageFactory::parseType15(BitBuffer &bits)
{
    auto msg = std::make_unique<Interrogation>();

    msg->type = AISMessageType::INTERROGATION;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->destinationMmsi1 = bits.getUInt32(40, 30);
    msg->messageType1_1 = bits.getUInt32(70, 6);
    msg->slotOffset1_1 = bits.getUInt32(76, 12);
    msg->spare2 = bits.getUInt32(88, 2);

    // 检查是否有第二个询问
    if (bits.remaining() > 90 && bits.getUInt32(90, 2) == 0)
    {
        msg->messageType1_2 = bits.getUInt32(92, 6);
        msg->slotOffset1_2 = bits.getUInt32(98, 12);
        msg->spare3 = bits.getUInt32(110, 2);
    }

    // 检查是否有第二个目标
    if (bits.remaining() > 112)
    {
        msg->destinationMmsi2 = bits.getUInt32(112, 30);
        msg->messageType2 = bits.getUInt32(142, 6);
        msg->slotOffset2 = bits.getUInt32(148, 12);
        msg->spare4 = bits.getUInt32(160, 2);
    }

    return msg;
}

// 类型16：分配模式命令
std::unique_ptr<AISMessage> MessageFactory::parseType16(BitBuffer &bits)
{
    auto msg = std::make_unique<AssignmentModeCommand>();

    msg->type = AISMessageType::ASSIGNMENT_MODE_COMMAND;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->destinationMmsiA = bits.getUInt32(40, 30);
    msg->offsetA = bits.getUInt32(70, 12);
    msg->incrementA = bits.getUInt32(82, 10);
    msg->spare2 = bits.getUInt32(92, 4);

    // 检查是否有第二个分配
    if (bits.remaining() > 96)
    {
        msg->destinationMmsiB = bits.getUInt32(96, 30);
        msg->offsetB = bits.getUInt32(126, 12);
        msg->incrementB = bits.getUInt32(138, 10);
        msg->spare3 = bits.getUInt32(148, 4);
    }

    return msg;
}

// 类型17：DGNSS二进制广播消息
std::unique_ptr<AISMessage> MessageFactory::parseType17(BitBuffer &bits)
{
    auto msg = std::make_unique<DGNSSBinaryBroadcast>();

    msg->type = AISMessageType::DGNSS_BINARY_BROADCAST;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->longitude = bits.getLongitude(40, 18);
    msg->latitude = bits.getLatitude(58, 17);
    msg->spare2 = bits.getUInt32(75, 5);

    // 使用流式读取剩余数据
    int remainingBits = bits.remaining();
    if (remainingBits > 0)
    {
        for (int i = 0; i < remainingBits; i += 8)
        {
            int bitsToRead = std::min(8, remainingBits - i);
            uint8_t byte = static_cast<uint8_t>(bits.getUInt32(bitsToRead));
            msg->dgnssData.push_back(byte);
        }
    }

    return msg;
}

// 类型18：标准B类设备位置报告 - 修正位偏移错误
std::unique_ptr<AISMessage> MessageFactory::parseType18(BitBuffer &bits)
{
    auto msg = std::make_unique<StandardClassBReport>();

    msg->type = AISMessageType::STANDARD_CLASS_B_CS_POSITION;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 8);
    msg->speedOverGround = bits.getSpeed(46, 10);
    msg->positionAccuracy = bits.getBool(56);
    msg->longitude = bits.getLongitude(57, 28);
    msg->latitude = bits.getLatitude(85, 27);
    msg->courseOverGround = bits.getCourse(112, 12);
    msg->trueHeading = bits.getUInt32(124, 9);
    msg->timestampUTC = bits.getUInt32(133, 6);
    msg->spare2 = bits.getUInt32(139, 2);  // 2位spare
    msg->csUnit = bits.getUInt32(141, 2);
    msg->displayFlag = bits.getBool(143);
    msg->dscFlag = bits.getBool(144);
    msg->bandFlag = bits.getBool(145);
    msg->message22Flag = bits.getBool(146);
    msg->assignedModeFlag = bits.getBool(147);
    msg->raimFlag = bits.getBool(148);
    msg->communicationState = bits.getUInt32(149, 19);
    msg->spare3 = bits.getUInt32(168, 1);

    return msg;
}

// 类型19：扩展B类设备位置报告
std::unique_ptr<AISMessage> MessageFactory::parseType19(BitBuffer &bits)
{
    auto msg = std::make_unique<ExtendedClassBReport>();

    msg->type = AISMessageType::EXTENDED_CLASS_B_CS_POSITION;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 8);
    msg->speedOverGround = bits.getSpeed(46, 10);
    msg->positionAccuracy = bits.getBool(56);
    msg->longitude = bits.getLongitude(57, 28);
    msg->latitude = bits.getLatitude(85, 27);
    msg->courseOverGround = bits.getCourse(112, 12);
    msg->trueHeading = bits.getUInt32(124, 9);
    msg->timestampUTC = bits.getUInt32(133, 6);
    msg->spare2 = bits.getUInt32(139, 4);
    msg->vesselName = bits.getString(143, 120);
    msg->shipType = bits.getUInt32(263, 8);
    msg->dimensionToBow = bits.getUInt32(271, 9);
    msg->dimensionToStern = bits.getUInt32(280, 9);
    msg->dimensionToPort = bits.getUInt32(289, 6);
    msg->dimensionToStarboard = bits.getUInt32(295, 6);
    msg->epfdType = bits.getUInt32(301, 4);
    msg->spare3 = bits.getUInt32(305, 1);
    msg->raimFlag = bits.getBool(306);
    msg->dte = bits.getBool(307);
    msg->assignedModeFlag = bits.getBool(308);
    msg->spare4 = bits.getUInt32(309, 4);

    return msg;
}

// 类型20：数据链路管理消息
std::unique_ptr<AISMessage> MessageFactory::parseType20(BitBuffer &bits)
{
    auto msg = std::make_unique<DataLinkManagement>();

    msg->type = AISMessageType::DATA_LINK_MANAGEMENT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);

    // 读取第一个偏移（固定30位）
    if (bits.remaining() >= 30) {
        msg->offsetNumber1 = bits.getUInt32(40, 12);
        msg->reservedSlots1 = bits.getUInt32(52, 4);
        msg->timeout1 = bits.getUInt32(56, 3);
        msg->increment1 = bits.getUInt32(59, 11);
    }

    // 读取后续偏移（每个30位）
    int offsetIndex = 2;
    int currentPos = 70;
    while (bits.remaining() >= currentPos + 30 && offsetIndex <= 4) {
        switch (offsetIndex) {
            case 2:
                msg->offsetNumber2 = bits.getUInt32(currentPos, 12);
                msg->reservedSlots2 = bits.getUInt32(currentPos + 12, 4);
                msg->timeout2 = bits.getUInt32(currentPos + 16, 3);
                msg->increment2 = bits.getUInt32(currentPos + 19, 11);
                break;
            case 3:
                msg->offsetNumber3 = bits.getUInt32(currentPos, 12);
                msg->reservedSlots3 = bits.getUInt32(currentPos + 12, 4);
                msg->timeout3 = bits.getUInt32(currentPos + 16, 3);
                msg->increment3 = bits.getUInt32(currentPos + 19, 11);
                break;
            case 4:
                msg->offsetNumber4 = bits.getUInt32(currentPos, 12);
                msg->reservedSlots4 = bits.getUInt32(currentPos + 12, 4);
                msg->timeout4 = bits.getUInt32(currentPos + 16, 3);
                msg->increment4 = bits.getUInt32(currentPos + 19, 11);
                break;
        }
        currentPos += 30;
        offsetIndex++;
    }

    return msg;
}

// 类型21：助航设备报告
std::unique_ptr<AISMessage> MessageFactory::parseType21(BitBuffer &bits)
{
    auto msg = std::make_unique<AidToNavigationReport>();

    msg->type = AISMessageType::AID_TO_NAVIGATION_REPORT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->aidType = bits.getUInt32(38, 5);
    msg->name = bits.getString(43, 120);
    msg->positionAccuracy = bits.getBool(163);
    msg->longitude = bits.getLongitude(164, 28);
    msg->latitude = bits.getLatitude(192, 27);
    msg->dimensionToBow = bits.getUInt32(219, 9);
    msg->dimensionToStern = bits.getUInt32(228, 9);
    msg->dimensionToPort = bits.getUInt32(237, 6);
    msg->dimensionToStarboard = bits.getUInt32(243, 6);
    msg->epfdType = bits.getUInt32(249, 4);
    msg->timestampUTC = bits.getUInt32(253, 6);
    msg->offPositionIndicator = bits.getBool(259);
    msg->regional = bits.getUInt32(260, 8);
    msg->raimFlag = bits.getBool(268);
    msg->virtualAidFlag = bits.getBool(269);
    msg->assignedModeFlag = bits.getBool(270);

    // 名称扩展 - 从271位开始
    int extensionBits = bits.remaining() - 271;
    if (extensionBits > 0)
    {
        msg->nameExtension = bits.getString(271, extensionBits);
    }

    return msg;
}

// 类型22：信道管理
std::unique_ptr<AISMessage> MessageFactory::parseType22(BitBuffer &bits)
{
    auto msg = std::make_unique<ChannelManagement>();

    msg->type = AISMessageType::CHANNEL_MANAGEMENT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->channelA = bits.getUInt32(40, 12);
    msg->channelB = bits.getUInt32(52, 12);
    msg->txRxMode = bits.getUInt32(64, 4);
    msg->power = bits.getUInt32(68, 1);

    // 检查是否有地理区域定义
    if (bits.getBool(69))
    {
        // 有地理区域
        msg->longitude1 = bits.getLongitude(70, 18);
        msg->latitude1 = bits.getLatitude(88, 17);
        msg->longitude2 = bits.getLongitude(105, 18);
        msg->latitude2 = bits.getLatitude(123, 17);
        msg->addressedOrBroadcast = bits.getUInt32(140, 1);
        msg->bandwidthA = bits.getUInt32(141, 2);
        msg->bandwidthB = bits.getUInt32(143, 2);
        msg->zoneSize = bits.getUInt32(145, 3);
    }
    else
    {
        // 无地理区域
        msg->addressedOrBroadcast = bits.getUInt32(70, 1);
        msg->bandwidthA = bits.getUInt32(71, 2);
        msg->bandwidthB = bits.getUInt32(73, 2);
        msg->zoneSize = bits.getUInt32(75, 3);
    }

    return msg;
}

// 类型23：组分配命令
std::unique_ptr<AISMessage> MessageFactory::parseType23(BitBuffer &bits)
{
    auto msg = std::make_unique<GroupAssignmentCommand>();

    msg->type = AISMessageType::GROUP_ASSIGNMENT_COMMAND;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->spare1 = bits.getUInt32(38, 2);
    msg->longitude1 = bits.getLongitude(40, 18);
    msg->latitude1 = bits.getLatitude(58, 17);
    msg->longitude2 = bits.getLongitude(75, 18);
    msg->latitude2 = bits.getLatitude(93, 17);
    msg->stationType = bits.getUInt32(110, 4);
    msg->shipType = bits.getUInt32(114, 8);
    msg->txRxMode = bits.getUInt32(122, 2);
    msg->reportingInterval = bits.getUInt32(124, 4);
    msg->quietTime = bits.getUInt32(128, 4);
    msg->spare2 = bits.getUInt32(132, 6);

    return msg;
}

// 类型24：静态数据报告
std::unique_ptr<AISMessage> MessageFactory::parseType24(BitBuffer &bits)
{
    auto msg = std::make_unique<StaticDataReport>();

    msg->type = AISMessageType::STATIC_DATA_REPORT;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->partNumber = bits.getUInt32(38, 2);

    if (msg->partNumber == 0)
    {
        msg->vesselName = bits.getString(40, 120);
        msg->spare = bits.getUInt32(160, 8);
    }
    else
    {
        msg->shipType = bits.getUInt32(40, 8);
        msg->vendorId = bits.getString(48, 42);
        msg->callSign = bits.getString(90, 42);
        msg->dimensionToBow = bits.getUInt32(132, 9);
        msg->dimensionToStern = bits.getUInt32(141, 9);
        msg->dimensionToPort = bits.getUInt32(150, 6);
        msg->dimensionToStarboard = bits.getUInt32(156, 6);
        msg->mothershipMmsi = bits.getUInt32(162, 30);
        msg->spare = bits.getUInt32(192, 6);
    }

    return msg;
}

// 类型25：单时隙二进制消息
std::unique_ptr<AISMessage> MessageFactory::parseType25(BitBuffer &bits)
{
    auto msg = std::make_unique<SingleSlotBinaryMessage>();

    msg->type = AISMessageType::SINGLE_SLOT_BINARY_MESSAGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->addressed = bits.getBool(38);
    msg->structured = bits.getBool(39);

    if (msg->addressed)
    {
        msg->destinationMmsi = bits.getUInt32(40, 30);
    }

    if (msg->structured)
    {
        msg->designatedAreaCode = bits.getUInt32(70, 10);
        msg->functionalId = bits.getUInt32(80, 6);
    }

    // 使用流式读取剩余数据
    int remainingBits = bits.remaining();
    if (remainingBits > 0)
    {
        for (int i = 0; i < remainingBits; i += 8)
        {
            int bitsToRead = std::min(8, remainingBits - i);
            uint8_t byte = static_cast<uint8_t>(bits.getUInt32(bitsToRead));
            msg->binaryData.push_back(byte);
        }
    }

    return msg;
}

// 类型26：多时隙二进制消息
std::unique_ptr<AISMessage> MessageFactory::parseType26(BitBuffer &bits)
{
    auto msg = std::make_unique<MultipleSlotBinaryMessage>();

    msg->type = AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->addressed = bits.getBool(38);
    msg->structured = bits.getBool(39);

    if (msg->addressed)
    {
        msg->destinationMmsi = bits.getUInt32(40, 30);
    }

    if (msg->structured)
    {
        msg->designatedAreaCode = bits.getUInt32(70, 10);
        msg->functionalId = bits.getUInt32(80, 6);
    }

    // 使用流式读取二进制数据（保留最后16位用于通信状态）
    int dataBits = bits.remaining() - 16;
    if (dataBits > 0)
    {
        for (int i = 0; i < dataBits; i += 8)
        {
            int bitsToRead = std::min(8, dataBits - i);
            uint8_t byte = static_cast<uint8_t>(bits.getUInt32(bitsToRead));
            msg->binaryData.push_back(byte);
        }
    }

    // 读取通信状态标志
    msg->commStateFlag = bits.getUInt32(16);

    return msg;
}

// 类型27：长距离位置报告
std::unique_ptr<AISMessage> MessageFactory::parseType27(BitBuffer &bits)
{
    auto msg = std::make_unique<LongRangePositionReport>();

    msg->type = AISMessageType::POSITION_REPORT_LONG_RANGE;
    msg->repeatIndicator = bits.getUInt32(6, 2);
    msg->mmsi = bits.getUInt32(8, 30);
    msg->positionAccuracy = bits.getBool(38);
    msg->raimFlag = bits.getBool(39);
    msg->navigationStatus = bits.getUInt32(40, 4);
    msg->longitude = bits.getLongitude(44, 18);
    msg->latitude = bits.getLatitude(62, 17);
    msg->speedOverGround = bits.getSpeed(79, 6);
    msg->courseOverGround = bits.getCourse(85, 9);
    msg->gnssPositionStatus = bits.getBool(94);
    msg->assignedModeFlag = bits.getBool(95);
    msg->spare = bits.getUInt32(96, 4);

    return msg;
}

} // namespace ais