#include "types/type27.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string LongRangePositionReport::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{"
        << "\"type\":27,"
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"longitude\":" << longitude << ","
        << "\"latitude\":" << latitude << ","
        << "\"speedOverGround\":" << speedOverGround << ","
        << "\"courseOverGround\":" << courseOverGround << ","
        << "\"gnssPositionStatus\":" << (gnssPositionStatus ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string LongRangePositionReport::toCsv() const {
    std::ostringstream oss;
    oss << "27,"
        << repeatIndicator << ","
        << mmsi << ","
        << (positionAccuracy ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << navigationStatus << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << latitude << ","
        << std::setprecision(1) << speedOverGround << ","
        << courseOverGround << ","
        << (gnssPositionStatus ? "1" : "0") << ","
        << spare << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<LongRangePositionReport> LongRangePositionReport::parse(BitBuffer& bits) {
    auto report = std::make_unique<LongRangePositionReport>();
    report->repeatIndicator = bits.getInt(6, 2);
    report->mmsi = bits.getInt(8, 30);
    report->positionAccuracy = bits.getBool(38);
    report->raimFlag = bits.getBool(39);
    report->navigationStatus = bits.getInt(40, 4);
    report->longitude = bits.getLongitude(44, 18);
    report->latitude = bits.getLatitude(62, 17);
    report->speedOverGround = bits.getInt(79, 6);
    report->courseOverGround = bits.getInt(85, 6);
    report->gnssPositionStatus = bits.getBool(91);
    report->spare = bits.getInt(92, 4);
    return report;
}

} // namespace ais