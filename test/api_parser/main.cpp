#include "ais_parser.h"
#include <iostream>

int main()
{
    ais::Config config;
    config.validateChecksum = true;
    config.enableMultipartReassembly = true;
    
    ais::AISParser parser(config);
    
    std::string nmea = "!AIVDM,1,1,,A,13aG`h0P000Htt<tSF0l4Q@100RS,0 * 7A";
    
    auto message = parser.parse(nmea);
    if (message) {
        std::cout << "Message type: " << static_cast<int>(message->type) << std::endl;
        std::cout << "MMSI: " << message->mmsi << std::endl;
        std::cout << "JSON: " << message->toJson() << std::endl;
        std::cout << "CSV: " << message->toCsv() << std::endl;
    } else {
        std::cout << "Failed to parse message" << std::endl;
    }
    
    return 0;
}