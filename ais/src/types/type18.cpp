#include "types/type18.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string StandardClassBReport::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{"
        << "\"type\":18,"
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"spare1\":" << spare1 << ","
        << "\"speedOverGround\":" << speedOverGround << ","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << longitude << ","
        << "\"latitude\":" << latitude << ","
        << "\"courseOverGround\":" << courseOverGround << ","
        << "\"trueHeading\":" << trueHeading << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"spare2\":" << spare2 << ","
        << "\"csUnit\":" << (csUnit ? "true" : "false") << ","
        << "\"displayFlag\":" << (displayFlag ? "true" : "false") << ","
        << "\"dscFlag\":" << (dscFlag ? "true" : "false") << ","
        << "\"bandFlag\":" << (bandFlag ? "true" : "false") << ","
        << "\"m22Flag\":" << (m22Flag ? "true" : "false") << ","
        << "\"assignedMode\":" << (assignedMode ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"communicationState\":" << communicationState << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string StandardClassBReport::toCsv() const {
    std::ostringstream oss;
    oss << "18,"
        << repeatIndicator << ","
        << mmsi << ","
        << spare1 << ","
        << std::fixed << std::setprecision(1) << speedOverGround << ","
        << (positionAccuracy ? "1" : "0") << ","
        << std::setprecision(6) << longitude << ","
        << latitude << ","
        << std::setprecision(1) << courseOverGround << ","
        << trueHeading << ","
        << timestampUTC << ","
        << spare2 << ","
        << (csUnit ? "1" : "0") << ","
        << (displayFlag ? "1" : "0") << ","
        << (dscFlag ? "1" : "0") << ","
        << (bandFlag ? "1" : "0") << ","
        << (m22Flag ? "1" : "0") << ","
        << (assignedMode ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << communicationState << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<StandardClassBReport> StandardClassBReport::parse(BitBuffer& bits) {
    auto report = std::make_unique<StandardClassBReport>();
    report->repeatIndicator = bits.getInt(6, 2);
    report->mmsi = bits.getInt(8, 30);
    report->spare1 = bits.getInt(38, 8);
    report->speedOverGround = bits.getInt(46, 10) / 10.0;
    report->positionAccuracy = bits.getBool(56);
    report->longitude = bits.getLongitude(57, 28);
    report->latitude = bits.getLatitude(85, 27);
    report->courseOverGround = bits.getInt(112, 12) / 10.0;
    report->trueHeading = bits.getInt(124, 9);
    report->timestampUTC = bits.getInt(133, 6);
    report->spare2 = bits.getInt(139, 2);
    report->csUnit = bits.getBool(141);
    report->displayFlag = bits.getBool(142);
    report->dscFlag = bits.getBool(143);
    report->bandFlag = bits.getBool(144);
    report->m22Flag = bits.getBool(145);
    report->assignedMode = bits.getBool(146);
    report->raimFlag = bits.getBool(147);
    report->communicationState = bits.getInt(148, 19);
    return report;
}

} // namespace ais