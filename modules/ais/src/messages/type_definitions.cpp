#include "messages/type_definitions.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ais
{

// 类型1：A类位置报告
std::string PositionReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"rateOfTurn\":" << rateOfTurn << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"specialManeuver\":" << specialManeuver << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string PositionReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << navigationStatus << ","
        << rateOfTurn << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << specialManeuver << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型2：A类位置报告（分配时隙）
std::string PositionReportAssigned::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"rateOfTurn\":" << rateOfTurn << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"specialManeuver\":" << specialManeuver << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string PositionReportAssigned::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << navigationStatus << ","
        << rateOfTurn << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << specialManeuver << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型3：A类位置报告（响应询问）
std::string PositionReportResponse::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"rateOfTurn\":" << rateOfTurn << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"specialManeuver\":" << specialManeuver << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string PositionReportResponse::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << navigationStatus << ","
        << rateOfTurn << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << specialManeuver << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型4：基站报告
std::string BaseStationReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"year\":" << year << ","
        << "\"month\":" << month << ","
        << "\"day\":" << day << ","
        << "\"hour\":" << hour << ","
        << "\"minute\":" << minute << ","
        << "\"second\":" << second << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string BaseStationReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << year << ","
        << month << ","
        << day << ","
        << hour << ","
        << minute << ","
        << second << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << epfdType << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型5：静态和航程相关数据
std::string StaticVoyageData::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"aisVersion\":" << aisVersion << ","
        << "\"imoNumber\":" << imoNumber << ","
        << "\"callSign\":\"" << callSign << "\","
        << "\"vesselName\":\"" << vesselName << "\","
        << "\"shipType\":" << shipType << ","
        << "\"dimensionToBow\":" << dimensionToBow << ","
        << "\"dimensionToStern\":" << dimensionToStern << ","
        << "\"dimensionToPort\":" << dimensionToPort << ","
        << "\"dimensionToStarboard\":" << dimensionToStarboard << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"etaMonth\":" << month << ","
        << "\"etaDay\":" << day << ","
        << "\"etaHour\":" << hour << ","
        << "\"etaMinute\":" << minute << ","
        << "\"draught\":" << std::fixed << std::setprecision(1) << draught << ","
        << "\"destination\":\"" << destination << "\","
        << "\"dte\":" << (dte ? "true" : "false") << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string StaticVoyageData::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << aisVersion << ","
        << imoNumber << ","
        << "\"" << callSign << "\","
        << "\"" << vesselName << "\","
        << shipType << ","
        << dimensionToBow << ","
        << dimensionToStern << ","
        << dimensionToPort << ","
        << dimensionToStarboard << ","
        << epfdType << ","
        << month << ","
        << day << ","
        << hour << ","
        << minute << ","
        << std::fixed << std::setprecision(1) << draught << ","
        << "\"" << destination << "\","
        << (dte ? "1" : "0") << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型6：二进制编址消息
std::string BinaryAddressedMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"sequenceNumber\":" << sequenceNumber << ","
        << "\"destinationMmsi\":" << destinationMmsi << ","
        << "\"retransmitFlag\":" << (retransmitFlag ? "true" : "false") << ","
        << "\"designatedAreaCode\":" << designatedAreaCode << ","
        << "\"functionalId\":" << functionalId << ","
        << "\"binaryDataSize\":" << binaryData.size() << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string BinaryAddressedMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << sequenceNumber << ","
        << destinationMmsi << ","
        << (retransmitFlag ? "1" : "0") << ","
        << designatedAreaCode << ","
        << functionalId << ","
        << binaryData.size() << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型7：二进制确认
std::string BinaryAcknowledge::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"sequenceNumber\":" << sequenceNumber << ","
        << "\"destinationMmsi1\":" << destinationMmsi1 << ","
        << "\"destinationMmsi2\":" << destinationMmsi2 << ","
        << "\"destinationMmsi3\":" << destinationMmsi3 << ","
        << "\"destinationMmsi4\":" << destinationMmsi4 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string BinaryAcknowledge::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << sequenceNumber << ","
        << destinationMmsi1 << ","
        << destinationMmsi2 << ","
        << destinationMmsi3 << ","
        << destinationMmsi4 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型8：二进制广播消息
std::string BinaryBroadcastMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare\":" << spare << ","
        << "\"designatedAreaCode\":" << designatedAreaCode << ","
        << "\"functionalId\":" << functionalId << ","
        << "\"binaryDataSize\":" << binaryData.size() << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string BinaryBroadcastMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare << ","
        << designatedAreaCode << ","
        << functionalId << ","
        << binaryData.size() << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型9：标准搜救飞机位置报告
std::string StandardSARAircraftReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"altitude\":" << altitude << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"spare\":" << spare << ","
        << "\"assignedModeFlag\":" << (assignedModeFlag ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string StandardSARAircraftReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << altitude << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << timestampUTC << ","
        << spare << ","
        << (assignedModeFlag ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型10：UTC和日期询问
std::string UTCDateInquiry::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"destinationMmsi\":" << destinationMmsi << ","
        << "\"spare2\":" << spare2 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string UTCDateInquiry::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << destinationMmsi << ","
        << spare2 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型11：UTC和日期响应
std::string UTCDateResponse::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"year\":" << year << ","
        << "\"month\":" << month << ","
        << "\"day\":" << day << ","
        << "\"hour\":" << hour << ","
        << "\"minute\":" << minute << ","
        << "\"second\":" << second << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"spare\":" << spare << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string UTCDateResponse::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << year << ","
        << month << ","
        << day << ","
        << hour << ","
        << minute << ","
        << second << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << epfdType << ","
        << spare << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型12：安全相关编址消息
std::string AddressedSafetyMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"sequenceNumber\":" << sequenceNumber << ","
        << "\"destinationMmsi\":" << destinationMmsi << ","
        << "\"retransmitFlag\":" << (retransmitFlag ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"safetyText\":\"" << safetyText << "\","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string AddressedSafetyMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << sequenceNumber << ","
        << destinationMmsi << ","
        << (retransmitFlag ? "1" : "0") << ","
        << spare << ","
        << "\"" << safetyText << "\","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型13：安全相关确认
std::string SafetyAcknowledge::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"sequenceNumber\":" << sequenceNumber << ","
        << "\"destinationMmsi1\":" << destinationMmsi1 << ","
        << "\"destinationMmsi2\":" << destinationMmsi2 << ","
        << "\"destinationMmsi3\":" << destinationMmsi3 << ","
        << "\"destinationMmsi4\":" << destinationMmsi4 << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string SafetyAcknowledge::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << sequenceNumber << ","
        << destinationMmsi1 << ","
        << destinationMmsi2 << ","
        << destinationMmsi3 << ","
        << destinationMmsi4 << ","
        << spare << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型14：安全相关广播消息
std::string SafetyRelatedBroadcast::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare\":" << spare << ","
        << "\"safetyText\":\"" << safetyText << "\","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string SafetyRelatedBroadcast::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare << ","
        << "\"" << safetyText << "\","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型15：询问
std::string Interrogation::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"destinationMmsi1\":" << destinationMmsi1 << ","
        << "\"messageType1_1\":" << messageType1_1 << ","
        << "\"slotOffset1_1\":" << slotOffset1_1 << ","
        << "\"spare2\":" << spare2 << ","
        << "\"messageType1_2\":" << messageType1_2 << ","
        << "\"slotOffset1_2\":" << slotOffset1_2 << ","
        << "\"spare3\":" << spare3 << ","
        << "\"destinationMmsi2\":" << destinationMmsi2 << ","
        << "\"messageType2\":" << messageType2 << ","
        << "\"slotOffset2\":" << slotOffset2 << ","
        << "\"spare4\":" << spare4 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string Interrogation::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << destinationMmsi1 << ","
        << messageType1_1 << ","
        << slotOffset1_1 << ","
        << spare2 << ","
        << messageType1_2 << ","
        << slotOffset1_2 << ","
        << spare3 << ","
        << destinationMmsi2 << ","
        << messageType2 << ","
        << slotOffset2 << ","
        << spare4 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型16：分配模式命令
std::string AssignmentModeCommand::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"destinationMmsiA\":" << destinationMmsiA << ","
        << "\"offsetA\":" << offsetA << ","
        << "\"incrementA\":" << incrementA << ","
        << "\"spare2\":" << spare2 << ","
        << "\"destinationMmsiB\":" << destinationMmsiB << ","
        << "\"offsetB\":" << offsetB << ","
        << "\"incrementB\":" << incrementB << ","
        << "\"spare3\":" << spare3 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string AssignmentModeCommand::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << destinationMmsiA << ","
        << offsetA << ","
        << incrementA << ","
        << spare2 << ","
        << destinationMmsiB << ","
        << offsetB << ","
        << incrementB << ","
        << spare3 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型17：DGNSS二进制广播消息
std::string DGNSSBinaryBroadcast::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"spare2\":" << spare2 << ","
        << "\"dgnssDataSize\":" << dgnssData.size() << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string DGNSSBinaryBroadcast::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << spare2 << ","
        << dgnssData.size() << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型18：标准B类设备位置报告
std::string StandardClassBReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"spare2\":" << spare2 << ","
        << "\"csUnit\":" << csUnit << ","
        << "\"displayFlag\":" << (displayFlag ? "true" : "false") << ","
        << "\"dscFlag\":" << (dscFlag ? "true" : "false") << ","
        << "\"bandFlag\":" << (bandFlag ? "true" : "false") << ","
        << "\"message22Flag\":" << (message22Flag ? "true" : "false") << ","
        << "\"assignedModeFlag\":" << (assignedModeFlag ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"spare3\":" << spare3 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string StandardClassBReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << spare2 << ","
        << csUnit << ","
        << (displayFlag ? "1" : "0") << ","
        << (dscFlag ? "1" : "0") << ","
        << (bandFlag ? "1" : "0") << ","
        << (message22Flag ? "1" : "0") << ","
        << (assignedModeFlag ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << spare3 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型19：扩展B类设备位置报告
std::string ExtendedClassBReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"spare2\":" << spare2 << ","
        << "\"vesselName\":\"" << vesselName << "\","
        << "\"shipType\":" << shipType << ","
        << "\"dimensionToBow\":" << dimensionToBow << ","
        << "\"dimensionToStern\":" << dimensionToStern << ","
        << "\"dimensionToPort\":" << dimensionToPort << ","
        << "\"dimensionToStarboard\":" << dimensionToStarboard << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"spare3\":" << spare3 << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"dte\":" << (dte ? "true" : "false") << ","
        << "\"assignedModeFlag\":" << (assignedModeFlag ? "true" : "false") << ","
        << "\"spare4\":" << spare4 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string ExtendedClassBReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << spare2 << ","
        << "\"" << vesselName << "\","
        << shipType << ","
        << dimensionToBow << ","
        << dimensionToStern << ","
        << dimensionToPort << ","
        << dimensionToStarboard << ","
        << epfdType << ","
        << spare3 << ","
        << (raimFlag ? "1" : "0") << ","
        << (dte ? "1" : "0") << ","
        << (assignedModeFlag ? "1" : "0") << ","
        << spare4 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型20：数据链路管理消息
std::string DataLinkManagement::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"offsetNumber1\":" << offsetNumber1 << ","
        << "\"reservedSlots1\":" << reservedSlots1 << ","
        << "\"timeout1\":" << timeout1 << ","
        << "\"increment1\":" << increment1 << ","
        << "\"offsetNumber2\":" << offsetNumber2 << ","
        << "\"reservedSlots2\":" << reservedSlots2 << ","
        << "\"timeout2\":" << timeout2 << ","
        << "\"increment2\":" << increment2 << ","
        << "\"offsetNumber3\":" << offsetNumber3 << ","
        << "\"reservedSlots3\":" << reservedSlots3 << ","
        << "\"timeout3\":" << timeout3 << ","
        << "\"increment3\":" << increment3 << ","
        << "\"offsetNumber4\":" << offsetNumber4 << ","
        << "\"reservedSlots4\":" << reservedSlots4 << ","
        << "\"timeout4\":" << timeout4 << ","
        << "\"increment4\":" << increment4 << ","
        << "\"spare2\":" << spare2 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string DataLinkManagement::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << offsetNumber1 << ","
        << reservedSlots1 << ","
        << timeout1 << ","
        << increment1 << ","
        << offsetNumber2 << ","
        << reservedSlots2 << ","
        << timeout2 << ","
        << increment2 << ","
        << offsetNumber3 << ","
        << reservedSlots3 << ","
        << timeout3 << ","
        << increment3 << ","
        << offsetNumber4 << ","
        << reservedSlots4 << ","
        << timeout4 << ","
        << increment4 << ","
        << spare2 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型21：助航设备报告
std::string AidToNavigationReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"aidType\":" << aidType << ","
        << "\"name\":\"" << name << "\","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"dimensionToBow\":" << dimensionToBow << ","
        << "\"dimensionToStern\":" << dimensionToStern << ","
        << "\"dimensionToPort\":" << dimensionToPort << ","
        << "\"dimensionToStarboard\":" << dimensionToStarboard << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"offPositionIndicator\":" << (offPositionIndicator ? "true" : "false") << ","
        << "\"regional\":" << regional << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"virtualAidFlag\":" << (virtualAidFlag ? "true" : "false") << ","
        << "\"assignedModeFlag\":" << (assignedModeFlag ? "true" : "false") << ","
        << "\"nameExtension\":\"" << nameExtension << "\","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string AidToNavigationReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << aidType << ","
        << "\"" << name << "\","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << dimensionToBow << ","
        << dimensionToStern << ","
        << dimensionToPort << ","
        << dimensionToStarboard << ","
        << epfdType << ","
        << timestampUTC << ","
        << (offPositionIndicator ? "1" : "0") << ","
        << regional << ","
        << (raimFlag ? "1" : "0") << ","
        << (virtualAidFlag ? "1" : "0") << ","
        << (assignedModeFlag ? "1" : "0") << ","
        << "\"" << nameExtension << "\","
        << spare << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型22：信道管理
std::string ChannelManagement::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"channelA\":" << channelA << ","
        << "\"channelB\":" << channelB << ","
        << "\"txRxMode\":" << txRxMode << ","
        << "\"power\":" << power << ","
        << "\"longitude1\":" << std::fixed << std::setprecision(6) << longitude1 << ","
        << "\"latitude1\":" << std::fixed << std::setprecision(6) << latitude1 << ","
        << "\"longitude2\":" << std::fixed << std::setprecision(6) << longitude2 << ","
        << "\"latitude2\":" << std::fixed << std::setprecision(6) << latitude2 << ","
        << "\"addressedOrBroadcast\":" << addressedOrBroadcast << ","
        << "\"bandwidthA\":" << bandwidthA << ","
        << "\"bandwidthB\":" << bandwidthB << ","
        << "\"zoneSize\":" << zoneSize << ","
        << "\"spare2\":" << spare2 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string ChannelManagement::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << channelA << ","
        << channelB << ","
        << txRxMode << ","
        << power << ","
        << std::fixed << std::setprecision(6) << longitude1 << ","
        << std::fixed << std::setprecision(6) << latitude1 << ","
        << std::fixed << std::setprecision(6) << longitude2 << ","
        << std::fixed << std::setprecision(6) << latitude2 << ","
        << addressedOrBroadcast << ","
        << bandwidthA << ","
        << bandwidthB << ","
        << zoneSize << ","
        << spare2 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型23：组分配命令
std::string GroupAssignmentCommand::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"longitude1\":" << std::fixed << std::setprecision(6) << longitude1 << ","
        << "\"latitude1\":" << std::fixed << std::setprecision(6) << latitude1 << ","
        << "\"longitude2\":" << std::fixed << std::setprecision(6) << longitude2 << ","
        << "\"latitude2\":" << std::fixed << std::setprecision(6) << latitude2 << ","
        << "\"stationType\":" << stationType << ","
        << "\"shipType\":" << shipType << ","
        << "\"txRxMode\":" << txRxMode << ","
        << "\"reportingInterval\":" << reportingInterval << ","
        << "\"quietTime\":" << quietTime << ","
        << "\"spare2\":" << spare2 << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string GroupAssignmentCommand::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << std::fixed << std::setprecision(6) << longitude1 << ","
        << std::fixed << std::setprecision(6) << latitude1 << ","
        << std::fixed << std::setprecision(6) << longitude2 << ","
        << std::fixed << std::setprecision(6) << latitude2 << ","
        << stationType << ","
        << shipType << ","
        << txRxMode << ","
        << reportingInterval << ","
        << quietTime << ","
        << spare2 << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型24：静态数据报告
std::string StaticDataReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"partNumber\":" << partNumber << ",";
    
    if (partNumber == 0) {
        oss << "\"vesselName\":\"" << vesselName << "\","
            << "\"spare\":" << spare;
    } else {
        oss << "\"shipType\":" << shipType << ","
            << "\"vendorId\":\"" << vendorId << "\","
            << "\"callSign\":\"" << callSign << "\","
            << "\"dimensionToBow\":" << dimensionToBow << ","
            << "\"dimensionToStern\":" << dimensionToStern << ","
            << "\"dimensionToPort\":" << dimensionToPort << ","
            << "\"dimensionToStarboard\":" << dimensionToStarboard << ","
            << "\"mothershipMmsi\":" << mothershipMmsi << ","
            << "\"spare\":" << spare;
    }
    
    oss << ",\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string StaticDataReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << partNumber << ",";
    
    if (partNumber == 0) {
        oss << "\"" << vesselName << "\","
            << spare;
    } else {
        oss << shipType << ","
            << "\"" << vendorId << "\","
            << "\"" << callSign << "\","
            << dimensionToBow << ","
            << dimensionToStern << ","
            << dimensionToPort << ","
            << dimensionToStarboard << ","
            << mothershipMmsi << ","
            << spare;
    }
    
    oss << ",\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型25：单时隙二进制消息
std::string SingleSlotBinaryMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"addressed\":" << (addressed ? "true" : "false") << ","
        << "\"structured\":" << (structured ? "true" : "false") << ",";
    
    if (addressed) {
        oss << "\"destinationMmsi\":" << destinationMmsi << ",";
    }
    
    if (structured) {
        oss << "\"designatedAreaCode\":" << designatedAreaCode << ","
            << "\"functionalId\":" << functionalId << ",";
    }
    
    oss << "\"binaryDataSize\":" << binaryData.size() << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string SingleSlotBinaryMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << (addressed ? "1" : "0") << ","
        << (structured ? "1" : "0") << ",";
    
    if (addressed) {
        oss << destinationMmsi << ",";
    } else {
        oss << "0,";
    }
    
    if (structured) {
        oss << designatedAreaCode << ","
            << functionalId << ",";
    } else {
        oss << "0,0,";
    }
    
    oss << binaryData.size() << ","
        << spare << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型26：多时隙二进制消息
std::string MultipleSlotBinaryMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"addressed\":" << (addressed ? "true" : "false") << ","
        << "\"structured\":" << (structured ? "true" : "false") << ",";
    
    if (addressed) {
        oss << "\"destinationMmsi\":" << destinationMmsi << ",";
    }
    
    if (structured) {
        oss << "\"designatedAreaCode\":" << designatedAreaCode << ","
            << "\"functionalId\":" << functionalId << ",";
    }
    
    oss << "\"binaryDataSize\":" << binaryData.size() << ","
        << "\"commStateFlag\":" << commStateFlag << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string MultipleSlotBinaryMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << (addressed ? "1" : "0") << ","
        << (structured ? "1" : "0") << ",";
    
    if (addressed) {
        oss << destinationMmsi << ",";
    } else {
        oss << "0,";
    }
    
    if (structured) {
        oss << designatedAreaCode << ","
            << functionalId << ",";
    } else {
        oss << "0,0,";
    }
    
    oss << binaryData.size() << ","
        << commStateFlag << ","
        << spare << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

// 类型27：长距离位置报告
std::string LongRangePositionReport::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"longitude\":" << std::fixed << std::setprecision(6) << longitude << ","
        << "\"latitude\":" << std::fixed << std::setprecision(6) << latitude << ","
        << "\"speedOverGround\":" << std::fixed << std::setprecision(1) << speedOverGround << ","
        << "\"courseOverGround\":" << std::fixed << std::setprecision(1) << courseOverGround << ","
        << "\"gnssPositionStatus\":" << (gnssPositionStatus ? "true" : "false") << ","
        << "\"assignedModeFlag\":" << (assignedModeFlag ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\","
        << "\"rawNMEA\":\"" << rawNMEA << "\""
        << "}";
    return oss.str();
}

std::string LongRangePositionReport::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << (positionAccuracy ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << navigationStatus << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << std::fixed << std::setprecision(6) << latitude << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << std::fixed << std::setprecision(1) << courseOverGround << ","
        << (gnssPositionStatus ? "1" : "0") << ","
        << (assignedModeFlag ? "1" : "0") << ","
        << spare << ","
        << "\"" << timestamp << "\","
        << "\"" << rawNMEA << "\"";
    return oss.str();
}

} // namespace ais