#include "types/type21.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string AidToNavigationReport::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{"
        << "\"type\":21,"
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"aidType\":" << aidType << ","
        << "\"name\":\"" << name << "\","
        << "\"positionAccuracy\":" << (positionAccuracy ? "true" : "false") << ","
        << "\"longitude\":" << longitude << ","
        << "\"latitude\":" << latitude << ","
        << "\"dimensionToBow\":" << dimensionToBow << ","
        << "\"dimensionToStern\":" << dimensionToStern << ","
        << "\"dimensionToPort\":" << dimensionToPort << ","
        << "\"dimensionToStarboard\":" << dimensionToStarboard << ","
        << "\"epfdType\":" << epfdType << ","
        << "\"timestampUTC\":" << timestampUTC << ","
        << "\"offPosition\":" << (offPosition ? "true" : "false") << ","
        << "\"raimFlag\":" << (raimFlag ? "true" : "false") << ","
        << "\"virtualAid\":" << (virtualAid ? "true" : "false") << ","
        << "\"assignedMode\":" << (assignedMode ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string AidToNavigationReport::toCsv() const {
    std::ostringstream oss;
    oss << "21,"
        << repeatIndicator << ","
        << mmsi << ","
        << aidType << ","
        << "\"" << name << "\","
        << (positionAccuracy ? "1" : "0") << ","
        << std::fixed << std::setprecision(6) << longitude << ","
        << latitude << ","
        << dimensionToBow << ","
        << dimensionToStern << ","
        << dimensionToPort << ","
        << dimensionToStarboard << ","
        << epfdType << ","
        << timestampUTC << ","
        << (offPosition ? "1" : "0") << ","
        << (raimFlag ? "1" : "0") << ","
        << (virtualAid ? "1" : "0") << ","
        << (assignedMode ? "1" : "0") << ","
        << spare << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<AidToNavigationReport> AidToNavigationReport::parse(BitBuffer& bits) {
    auto report = std::make_unique<AidToNavigationReport>();
    report->repeatIndicator = bits.getInt(6, 2);
    report->mmsi = bits.getInt(8, 30);
    report->aidType = bits.getInt(38, 5);
    report->name = bits.getString(43, 120);
    report->positionAccuracy = bits.getBool(163);
    report->longitude = bits.getLongitude(164);
    report->latitude = bits.getLatitude(192);
    report->dimensionToBow = bits.getInt(219, 9);
    report->dimensionToStern = bits.getInt(228, 9);
    report->dimensionToPort = bits.getInt(237, 6);
    report->dimensionToStarboard = bits.getInt(243, 6);
    report->epfdType = bits.getInt(249, 4);
    report->timestampUTC = bits.getInt(253, 6);
    report->offPosition = bits.getBool(259);
    report->raimFlag = bits.getBool(268);
    report->virtualAid = bits.getBool(269);
    report->assignedMode = bits.getBool(270);
    report->spare = bits.getInt(271, 1);
    return report;
}

} // namespace ais