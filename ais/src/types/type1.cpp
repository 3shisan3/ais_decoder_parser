#include "types/type1.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string PositionReport::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{"
        << "\"type\":1,"
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"navigationStatus\":" << navigationStatus << ","
        << "\"rateOfTurn\":" << rateOfTurn << ","
        << "\"speedOverGround\":" << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << longitude << ","
        << "\"latitude\":" << latitude << ","
        << "\"courseOverGround\":" << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"specialManeuver\":" << specialManeuver << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string PositionReport::toCsv() const {
    std::ostringstream oss;
    oss << "1,"
        << repeatIndicator << ","
        << mmsi << ","
        << navigationStatus << ","
        << rateOfTurn << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::setprecision(6) << longitude << ","
        << latitude << ","
        << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << specialManeuver << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<PositionReport> PositionReport::parse(BitBuffer& bits) {
    auto report = std::make_unique<PositionReport>();
    report->repeatIndicator = bits.getInt(6, 2);
    report->mmsi = bits.getInt(8, 30);
    report->navigationStatus = bits.getInt(38, 4);
    report->rateOfTurn = bits.getInt(42, 8);
    report->speedOverGround = bits.getInt(50, 10) / 10.0;
    report->positionAccuracy = bits.getBool(60);
    report->longitude = bits.getLongitude(61);
    report->latitude = bits.getLatitude(89);
    report->courseOverGround = bits.getInt(116, 12) / 10.0;
    report->trueHeading = bits.getInt(128, 9);
    report->timestampUTC = bits.getInt(137, 6);
    report->specialManeuver = bits.getInt(143, 2);
    report->raimFlag = bits.getBool(148);
    report->communicationState = bits.getInt(149, 19);
    return report;
}

} // namespace ais