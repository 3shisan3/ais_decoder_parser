#include "message.h"

#include "types/types.h"

#include <sstream>
#include <iomanip>

namespace ais {

std::string AISMessage::toJson() const {
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string AISMessage::toCsv() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<AISMessage> AISMessage::create(AISMessageType type) {
    switch(type) {
        case AISMessageType::POSITION_REPORT_CLASS_A:
            return std::make_unique<PositionReport>();
        case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED:
            return std::make_unique<PositionReportAssigned>();
        case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE:
            return std::make_unique<PositionReportResponse>();
        case AISMessageType::BASE_STATION_REPORT:
            return std::make_unique<BaseStationReport>();
        case AISMessageType::STATIC_VOYAGE_DATA:
            return std::make_unique<StaticVoyageData>();
        // 其他消息类型...
        case AISMessageType::POSITION_REPORT_LONG_RANGE:
            return std::make_unique<LongRangePositionReport>();
        default:
            return std::make_unique<AISMessage>();
    }
}

} // namespace ais