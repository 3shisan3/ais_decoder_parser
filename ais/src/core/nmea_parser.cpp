#include "core/nmea_parser.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ais {

bool NMEAParser::validateChecksum(const std::string& nmea) {
    if (nmea.empty() || nmea[0] != '!') {
        return false;
    }
    
    size_t starPos = nmea.find('*');
    if (starPos == std::string::npos || starPos + 3 > nmea.length()) {
        return false;
    }
    
    uint8_t calculated = 0;
    for (size_t i = 1; i < starPos; i++) {
        calculated ^= nmea[i];
    }
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << (int)calculated;
    std::string calculatedStr = ss.str();
    if (calculatedStr.length() == 1) calculatedStr = "0" + calculatedStr;
    
    std::string provided = nmea.substr(starPos + 1, 2);
    
    return calculatedStr == provided;
}

std::string NMEAParser::extractPayload(const std::string& nmea) {
    size_t start = nmea.find(',');
    if (start == std::string::npos) return "";
    
    start++; // 跳过第一个逗号
    size_t end = nmea.find('*', start);
    if (end == std::string::npos) return "";
    
    return nmea.substr(start, end - start);
}

int NMEAParser::getFragmentCount(const std::string& nmea) {
    auto parts = split(nmea, ',');
    return parts.size() > 1 ? std::stoi(parts[1]) : 1;
}

int NMEAParser::getFragmentNumber(const std::string& nmea) {
    auto parts = split(nmea, ',');
    return parts.size() > 2 ? std::stoi(parts[2]) : 1;
}

std::string NMEAParser::getMessageId(const std::string& nmea) {
    auto parts = split(nmea, ',');
    return parts.size() > 3 ? parts[3] : "";
}

std::string NMEAParser::decode6bitASCII(const std::string& payload) {
    std::string binary;
    for (char c : payload) {
        if (c < 48 || c > 119) continue;
        int value = (c - 48) & 0x3F;
        binary += std::bitset<6>(value).to_string();
    }
    return binary;
}

std::vector<std::string> NMEAParser::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace ais