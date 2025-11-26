#include "ais_message_generator.h"

#include <QRandomGenerator>
#include <QDateTime>

AISMessageGenerator::AISMessageGenerator(QObject *parent)
    : QObject(parent)
    , m_encoder(ais::AISGenerateCfg())
{
}

std::string AISMessageGenerator::generateType1Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::PositionReport>();
    message->type = ais::AISMessageType::POSITION_REPORT_CLASS_A;
    message->mmsi = data.mmsi;
    
    fillPositionReportFields(*message, data);
    
    // Type 1特定字段
    message->navigationStatus = data.navigationStatus;
    message->rateOfTurn = data.rateOfTurn;
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->specialManeuver = 0;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType2Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::PositionReportAssigned>();
    message->type = ais::AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED;
    message->mmsi = data.mmsi;
    
    // 使用模板版本的填充函数
    fillPositionReportFields(*message, data);
    
    // Type 2特定字段
    message->navigationStatus = data.navigationStatus;
    message->rateOfTurn = data.rateOfTurn;
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->specialManeuver = 0;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType3Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::PositionReportResponse>();
    message->type = ais::AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE;
    message->mmsi = data.mmsi;
    
    // 使用模板版本的填充函数
    fillPositionReportFields(*message, data);
    
    // Type 3特定字段
    message->navigationStatus = data.navigationStatus;
    message->rateOfTurn = data.rateOfTurn;
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->specialManeuver = 0;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType4Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::BaseStationReport>();
    message->type = ais::AISMessageType::BASE_STATION_REPORT;
    message->mmsi = data.mmsi;
    
    // 基站特定字段
    QDateTime currentTime = QDateTime::currentDateTime();
    message->year = currentTime.date().year();
    message->month = currentTime.date().month();
    message->day = currentTime.date().day();
    message->hour = currentTime.time().hour();
    message->minute = currentTime.time().minute();
    message->second = currentTime.time().second();
    
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->epfdType = data.epfdType;
    message->raimFlag = false;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType5Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::StaticVoyageData>();
    message->type = ais::AISMessageType::STATIC_VOYAGE_DATA;
    message->mmsi = data.mmsi;
    
    fillStaticDataFields(*message, data);
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType6Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::BinaryAddressedMessage>();
    message->type = ais::AISMessageType::BINARY_ADDRESSED_MESSAGE;
    message->mmsi = data.mmsi;
    
    message->sequenceNumber = 0;
    message->destinationMmsi = data.destinationMmsi;
    message->retransmitFlag = false;
    message->designatedAreaCode = data.designatedAreaCode;
    message->functionalId = data.functionalId;
    message->binaryData = data.binaryData;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType7Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::BinaryAcknowledge>();
    message->type = ais::AISMessageType::BINARY_ACKNOWLEDGE;
    message->mmsi = data.mmsi;
    
    message->sequenceNumber = 0;
    message->destinationMmsi1 = data.destinationMmsi;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType8Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::BinaryBroadcastMessage>();
    message->type = ais::AISMessageType::BINARY_BROADCAST_MESSAGE;
    message->mmsi = data.mmsi;
    
    message->spare = 0;
    message->designatedAreaCode = data.designatedAreaCode;
    message->functionalId = data.functionalId;
    message->binaryData = data.binaryData;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType9Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::StandardSARAircraftReport>();
    message->type = ais::AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT;
    message->mmsi = data.mmsi;
    
    // SAR飞机特定字段
    message->altitude = 1000; // 默认海拔高度
    message->speedOverGround = data.speed;
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->courseOverGround = data.courseOverGround;
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->spare = 0;
    message->assignedModeFlag = false;
    message->raimFlag = false;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType10Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::UTCDateInquiry>();
    message->type = ais::AISMessageType::UTC_DATE_INQUIRY;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->destinationMmsi = data.destinationMmsi;
    message->spare2 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType11Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::UTCDateResponse>();
    message->type = ais::AISMessageType::UTC_DATE_RESPONSE;
    message->mmsi = data.mmsi;
    
    QDateTime currentTime = QDateTime::currentDateTime();
    message->year = currentTime.date().year();
    message->month = currentTime.date().month();
    message->day = currentTime.date().day();
    message->hour = currentTime.time().hour();
    message->minute = currentTime.time().minute();
    message->second = currentTime.time().second();
    
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->epfdType = data.epfdType;
    message->spare = 0;
    message->raimFlag = false;
    message->communicationState = generateCommunicationState();
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType12Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::AddressedSafetyMessage>();
    message->type = ais::AISMessageType::ADDRESSED_SAFETY_MESSAGE;
    message->mmsi = data.mmsi;
    
    message->sequenceNumber = 0;
    message->destinationMmsi = data.destinationMmsi;
    message->retransmitFlag = false;
    message->spare = 0;
    message->safetyText = "SAFETY MESSAGE";
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType13Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::SafetyAcknowledge>();
    message->type = ais::AISMessageType::SAFETY_ACKNOWLEDGE;
    message->mmsi = data.mmsi;
    
    message->sequenceNumber = 0;
    message->destinationMmsi1 = data.destinationMmsi;
    message->spare = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType14Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::SafetyRelatedBroadcast>();
    message->type = ais::AISMessageType::SAFETY_RELATED_BROADCAST;
    message->mmsi = data.mmsi;
    
    message->spare = 0;
    message->safetyText = "BROADCAST SAFETY MESSAGE";
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType15Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::Interrogation>();
    message->type = ais::AISMessageType::INTERROGATION;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->destinationMmsi1 = data.destinationMmsi;
    message->messageType1_1 = 1;
    message->slotOffset1_1 = 0;
    message->spare2 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType16Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::AssignmentModeCommand>();
    message->type = ais::AISMessageType::ASSIGNMENT_MODE_COMMAND;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->destinationMmsiA = data.destinationMmsi;
    message->offsetA = 0;
    message->incrementA = 0;
    message->spare2 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType17Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::DGNSSBinaryBroadcast>();
    message->type = ais::AISMessageType::DGNSS_BINARY_BROADCAST;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->spare2 = 0;
    message->dgnssData = data.binaryData;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType18Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::StandardClassBReport>();
    message->type = ais::AISMessageType::STANDARD_CLASS_B_CS_POSITION;
    message->mmsi = data.mmsi;
    
    // B类设备特定字段
    message->spare1 = 0;
    message->speedOverGround = data.speed;
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->courseOverGround = data.courseOverGround;
    message->trueHeading = static_cast<int>(data.heading);
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->spare2 = 0;
    message->csUnit = 0;
    message->displayFlag = false;
    message->dscFlag = false;
    message->bandFlag = false;
    message->message22Flag = false;
    message->assignedModeFlag = false;
    message->raimFlag = false;
    message->communicationState = generateCommunicationState();
    message->spare3 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType19Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::ExtendedClassBReport>();
    message->type = ais::AISMessageType::EXTENDED_CLASS_B_CS_POSITION;
    message->mmsi = data.mmsi;
    
    // 扩展B类设备字段
    message->spare1 = 0;
    message->speedOverGround = data.speed;
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->courseOverGround = data.courseOverGround;
    message->trueHeading = static_cast<int>(data.heading);
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->spare2 = 0;
    message->vesselName = data.vesselName.toStdString();
    message->shipType = data.shipType;
    message->dimensionToBow = data.length / 2;
    message->dimensionToStern = data.length / 2;
    message->dimensionToPort = data.width / 2;
    message->dimensionToStarboard = data.width / 2;
    message->epfdType = data.epfdType;
    message->spare3 = 0;
    message->raimFlag = false;
    message->dte = false;
    message->assignedModeFlag = false;
    message->spare4 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType20Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::DataLinkManagement>();
    message->type = ais::AISMessageType::DATA_LINK_MANAGEMENT;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    // 设置默认的数据链路管理参数
    message->offsetNumber1 = 1;
    message->reservedSlots1 = 1;
    message->timeout1 = 3;
    message->increment1 = 1;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType21Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::AidToNavigationReport>();
    message->type = ais::AISMessageType::AID_TO_NAVIGATION_REPORT;
    message->mmsi = data.mmsi;
    
    // 助航设备特定字段
    message->aidType = data.aidType;
    message->name = data.vesselName.toStdString();
    message->positionAccuracy = true;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->dimensionToBow = 0;
    message->dimensionToStern = 0;
    message->dimensionToPort = 0;
    message->dimensionToStarboard = 0;
    message->epfdType = data.epfdType;
    message->timestampUTC = QDateTime::currentDateTime().time().second();
    message->offPositionIndicator = data.offPositionIndicator;
    message->regional = data.regional;
    message->raimFlag = false;
    message->virtualAidFlag = data.virtualAidFlag;
    message->assignedModeFlag = false;
    message->nameExtension = data.nameExtension.toStdString();
    message->spare = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType22Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::ChannelManagement>();
    message->type = ais::AISMessageType::CHANNEL_MANAGEMENT;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->channelA = 2087;
    message->channelB = 2088;
    message->txRxMode = 0;
    message->power = 1;
    message->longitude1 = data.position.longitude() - 0.1;
    message->latitude1 = data.position.latitude() - 0.1;
    message->longitude2 = data.position.longitude() + 0.1;
    message->latitude2 = data.position.latitude() + 0.1;
    message->addressedOrBroadcast = 0;
    message->bandwidthA = 0;
    message->bandwidthB = 0;
    message->zoneSize = 0;
    message->spare2 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType23Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::GroupAssignmentCommand>();
    message->type = ais::AISMessageType::GROUP_ASSIGNMENT_COMMAND;
    message->mmsi = data.mmsi;
    
    message->spare1 = 0;
    message->longitude1 = data.position.longitude() - 0.5;
    message->latitude1 = data.position.latitude() - 0.5;
    message->longitude2 = data.position.longitude() + 0.5;
    message->latitude2 = data.position.latitude() + 0.5;
    message->stationType = 0;
    message->shipType = data.shipType;
    message->txRxMode = 0;
    message->reportingInterval = 0;
    message->quietTime = 0;
    message->spare2 = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType24Message(const AISVesselData &data, int partNumber)
{
    auto message = std::make_unique<ais::StaticDataReport>();
    message->type = ais::AISMessageType::STATIC_DATA_REPORT;
    message->mmsi = data.mmsi;
    
    message->partNumber = partNumber;
    
    if (partNumber == 0) {
        // Part A: 船名
        message->vesselName = data.vesselName.toStdString();
        message->spare = 0;
    } else {
        // Part B: 其他静态数据
        message->shipType = data.shipType;
        message->vendorId = "TEST";
        message->callSign = data.callSign.toStdString();
        message->dimensionToBow = data.length / 2;
        message->dimensionToStern = data.length / 2;
        message->dimensionToPort = data.width / 2;
        message->dimensionToStarboard = data.width / 2;
        message->mothershipMmsi = 0;
        message->spare = 0;
    }
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType25Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::SingleSlotBinaryMessage>();
    message->type = ais::AISMessageType::SINGLE_SLOT_BINARY_MESSAGE;
    message->mmsi = data.mmsi;
    
    message->addressed = false;
    message->structured = true;
    message->destinationMmsi = 0;
    message->designatedAreaCode = data.designatedAreaCode;
    message->functionalId = data.functionalId;
    message->binaryData = data.binaryData;
    message->spare = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType26Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::MultipleSlotBinaryMessage>();
    message->type = ais::AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE;
    message->mmsi = data.mmsi;
    
    message->addressed = false;
    message->structured = true;
    message->destinationMmsi = 0;
    message->designatedAreaCode = data.designatedAreaCode;
    message->functionalId = data.functionalId;
    message->binaryData = data.binaryData;
    message->commStateFlag = 0;
    message->spare = 0;
    
    return encodeMessage(*message);
}

std::string AISMessageGenerator::generateType27Message(const AISVesselData &data)
{
    auto message = std::make_unique<ais::LongRangePositionReport>();
    message->type = ais::AISMessageType::POSITION_REPORT_LONG_RANGE;
    message->mmsi = data.mmsi;
    
    // 长距离位置报告特定字段
    message->positionAccuracy = true;
    message->raimFlag = false;
    message->navigationStatus = data.navigationStatus;
    message->longitude = data.position.longitude();
    message->latitude = data.position.latitude();
    message->speedOverGround = data.speed;
    message->courseOverGround = data.courseOverGround;
    message->gnssPositionStatus = true;
    message->assignedModeFlag = false;
    message->spare = 0;
    
    return encodeMessage(*message);
}

// ================= 私有方法实现 =================

std::unique_ptr<ais::AISMessage> AISMessageGenerator::createBaseMessage(ais::AISMessageType type, const AISVesselData &data)
{
    auto message = std::make_unique<ais::AISMessage>();
    message->type = type;
    message->mmsi = data.mmsi;
    message->repeatIndicator = 0;
    return message;
}

std::string AISMessageGenerator::encodeMessage(const ais::AISMessage &message)
{
    try {
        auto nmeaSentences = m_encoder.encode(message);
        if (!nmeaSentences.empty()) {
            return nmeaSentences[0]; // 返回第一条NMEA语句
        }
    } catch (const std::exception &e) {
        qWarning() << "AIS message encoding failed:" << e.what();
    }
    return "";
}

void AISMessageGenerator::fillPositionReportFields(ais::PositionReport &report, const AISVesselData &data)
{
    report.speedOverGround = data.speed;
    report.positionAccuracy = true;
    report.longitude = data.position.longitude();
    report.latitude = data.position.latitude();
    report.courseOverGround = data.courseOverGround;
    report.trueHeading = static_cast<int>(data.heading);
    report.raimFlag = false;
}

// 模板版本的填充函数，支持所有位置报告类型
template<typename T>
void AISMessageGenerator::fillPositionReportFields(T &report, const AISVesselData &data)
{
    report.speedOverGround = data.speed;
    report.positionAccuracy = true;
    report.longitude = data.position.longitude();
    report.latitude = data.position.latitude();
    report.courseOverGround = data.courseOverGround;
    report.trueHeading = static_cast<int>(data.heading);
    report.raimFlag = false;
}

void AISMessageGenerator::fillStaticDataFields(ais::StaticVoyageData &staticData, const AISVesselData &data)
{
    staticData.aisVersion = 0;
    staticData.imoNumber = data.imo;
    staticData.callSign = data.callSign.toStdString();
    staticData.vesselName = data.vesselName.toStdString();
    staticData.shipType = data.shipType;
    staticData.dimensionToBow = data.length / 2;
    staticData.dimensionToStern = data.length / 2;
    staticData.dimensionToPort = data.width / 2;
    staticData.dimensionToStarboard = data.width / 2;
    staticData.epfdType = data.epfdType;
    
    // ETA设置
    QDateTime eta = QDateTime::currentDateTime().addDays(1);
    staticData.month = eta.date().month();
    staticData.day = eta.date().day();
    staticData.hour = eta.time().hour();
    staticData.minute = eta.time().minute();
    
    staticData.draught = data.draft;
    staticData.destination = data.destination.toStdString();
    staticData.dte = false;
}

int AISMessageGenerator::generateCommunicationState()
{
    return QRandomGenerator::global()->bounded(0, 8191); // 19位通信状态
}