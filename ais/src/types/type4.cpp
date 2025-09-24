#include "types/type4.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string BaseStationReport::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{"
        << "\"type\":4,"
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"year\":" << year << ","
        << "\"month\":" << month << ","
        << "\"day\":" << day << ","
        << "\"hour\":" << hour << ","
        << "\"minute\":" << minute << ","
        << "\"second\":" << second << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << longitude << ","
        << "\"latitude\":" << latitude << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string BaseStationReport::toCsv() const {
    std::ostringstream oss;
    oss << "4,"
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
        << latitude << ","
        << epfdType << ","
        << (raimFlag ? "1" : "0") << ","
        << spare << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<BaseStationReport> BaseStationReport::parse(BitBuffer& bits) {
    auto report = std::make_unique<BaseStationReport>();
    report->repeatIndicator = bits.getInt(6, 2);
    report->mmsi = bits.getInt(8, 30);
    report->year = bits.getInt(38, 14);
    report->month = bits.getInt(52, 4);
    report->day = bits.getInt(56, 5);
    report->hour = bits.getInt(61, 5);
    report->minute = bits.getInt(66, 6);
    report->second = bits.getInt(72, 6);
    report->positionAccuracy = bits.getBool(78);
    report->longitude = bits.getLongitude(79, 28);
    report->latitude = bits.getLatitude(107, 27);
    report->epfdType = bits.getInt(135, 4);
    report->raimFlag = bits.getBool(148);
    report->spare = bits.getInt(149, 3);
    return report;
}

} // namespace ais