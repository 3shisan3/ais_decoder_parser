#include "types/type5.h"

#include <iomanip>
#include <sstream>

namespace ais {

std::string StaticVoyageData::toJson() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "{"
        << "\"type\":5,"
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
        << "\"etaMonth\":" << etaMonth << ","
        << "\"etaDay\":" << etaDay << ","
        << "\"etaHour\":" << etaHour << ","
        << "\"etaMinute\":" << etaMinute << ","
        << "\"draught\":" << draught << ","
        << "\"destination\":\"" << destination << "\","
        << "\"dte\":" << (dte ? "true" : "false") << ","
        << "\"spare\":" << spare << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string StaticVoyageData::toCsv() const {
    std::ostringstream oss;
    oss << "5,"
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
        << etaMonth << ","
        << etaDay << ","
        << etaHour << ","
        << etaMinute << ","
        << std::fixed << std::setprecision(1) << draught << ","
        << "\"" << destination << "\","
        << (dte ? "1" : "0") << ","
        << spare << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<StaticVoyageData> StaticVoyageData::parse(BitBuffer& bits) {
    auto data = std::make_unique<StaticVoyageData>();
    data->repeatIndicator = bits.getInt(6, 2);
    data->mmsi = bits.getInt(8, 30);
    data->aisVersion = bits.getInt(38, 2);
    data->imoNumber = bits.getInt(40, 30);
    data->callSign = bits.getString(70, 42);
    data->vesselName = bits.getString(112, 120);
    data->shipType = bits.getInt(232, 8);
    data->dimensionToBow = bits.getInt(240, 9);
    data->dimensionToStern = bits.getInt(249, 9);
    data->dimensionToPort = bits.getInt(258, 6);
    data->dimensionToStarboard = bits.getInt(264, 6);
    data->epfdType = bits.getInt(270, 4);
    data->etaMonth = bits.getInt(274, 4);
    data->etaDay = bits.getInt(278, 5);
    data->etaHour = bits.getInt(283, 5);
    data->etaMinute = bits.getInt(288, 6);
    data->draught = bits.getInt(294, 8) / 10.0;
    data->destination = bits.getString(302, 120);
    data->dte = bits.getBool(422);
    data->spare = bits.getInt(423, 1);
    return data;
}

} // namespace ais