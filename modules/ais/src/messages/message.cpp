#include "messages/message.h"

#include "core/bit_buffer.h"
#include "messages/message_factory.h"

#include <sstream>
#include <iomanip>

namespace ais
{

std::string AISMessage::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"type\":" << static_cast<int>(type) << ","
        << "\"repeatIndicator\":" << repeatIndicator << ","
        << "\"mmsi\":" << mmsi << ","
        << "\"timestamp\":\"" << timestamp << "\""
        << "}";
    return oss.str();
}

std::string AISMessage::toCsv() const
{
    std::ostringstream oss;
    oss << static_cast<int>(type) << ","
        << repeatIndicator << ","
        << mmsi << ","
        << timestamp;
    return oss.str();
}

std::unique_ptr<AISMessage> AISMessage::parse(BitBuffer& bits)
{
    return MessageFactory::createMessage(bits);
}

} // namespace ais