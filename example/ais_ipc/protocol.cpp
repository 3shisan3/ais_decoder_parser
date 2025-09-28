#include "protocol.h"

#include <sstream>
#include <stdexcept>

namespace ais {
namespace protocol {

using namespace std;

string CommandMessage::toJson() const
{
    nlohmann::json j;
    j["type"] = static_cast<int>(type);
    j["sequence"] = sequence;
    j["data"] = data;
    
    return j.dump(); // 紧凑格式，无缩进
}

CommandMessage CommandMessage::fromJson(const string& jsonStr)
{
    try {
        auto j = nlohmann::json::parse(jsonStr);
        
        CommandMessage msg;
        msg.type = static_cast<CommandType>(j["type"].get<int>());
        msg.sequence = j["sequence"].get<uint32_t>();
        msg.data = j["data"].get<string>();
        
        return msg;
        
    } catch (const nlohmann::json::exception& e) {
        throw runtime_error("JSON解析失败: " + string(e.what()));
    } catch (const exception& e) {
        throw runtime_error("命令消息解析失败: " + string(e.what()));
    }
}

string ResponseMessage::toJson() const
{
    nlohmann::json j;
    j["status"] = static_cast<int>(status);
    j["sequence"] = sequence;
    j["data"] = data;
    
    return j.dump();
}

ResponseMessage ResponseMessage::fromJson(const string& jsonStr)
{
    try {
        auto j = nlohmann::json::parse(jsonStr);
        
        ResponseMessage msg;
        msg.status = static_cast<ResponseStatus>(j["status"].get<int>());
        msg.sequence = j["sequence"].get<uint32_t>();
        msg.data = j["data"].get<string>();
        
        return msg;
        
    } catch (const nlohmann::json::exception& e) {
        throw runtime_error("JSON解析失败: " + string(e.what()));
    } catch (const exception& e) {
        throw runtime_error("响应消息解析失败: " + string(e.what()));
    }
}

nlohmann::json ServiceStatus::toJson() const
{
    nlohmann::json j;
    j["is_running"] = isRunning;
    j["ship_count"] = shipCount;
    j["uptime"] = uptime;
    j["messages_received"] = messagesReceived;
    j["messages_sent"] = messagesSent;
    j["messages_processed"] = messagesProcessed;
    j["last_message_time"] = lastMessageTime;
    
    return j;
}

ServiceStatus ServiceStatus::fromJson(const nlohmann::json& j)
{
    ServiceStatus status;
    status.isRunning = j.value("is_running", false);
    status.shipCount = j.value("ship_count", 0);
    status.uptime = j.value("uptime", 0);
    status.messagesReceived = j.value("messages_received", 0);
    status.messagesSent = j.value("messages_sent", 0);
    status.messagesProcessed = j.value("messages_processed", 0);
    status.lastMessageTime = j.value("last_message_time", 0);
    
    return status;
}

} // namespace protocol
} // namespace ais