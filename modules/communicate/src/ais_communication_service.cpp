#include "ais_communication_service.h"

#include "logger_define.h"
// #include "messages/type_definitions.h"  // 若需按具体类型进行映射需要包含

#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace ais {

using namespace std::chrono;

AISCommunicationService::AISCommunicationService(std::shared_ptr<AISParser> aisParser)
    : aisParser_(aisParser)
{
    if (!aisParser_) {
        LOG_WARNING("AISParser is null, service may not work properly");
    }
}

AISCommunicationService::~AISCommunicationService()
{
}

int AISCommunicationService::initialize(const CommunicateCfg& commCfg,
                                        const std::string& configPath)
{
    if (isInitialized_) {
        LOG_WARNING("AISCommunicationService already initialized");
        return 0;
    }

    int errorCode = -1;

    if (!aisParser_) {
        LOG_ERROR("AISParser is not provided");
        return errorCode;
    }
    --errorCode;

    // 验证配置
    if (commCfg.subPort <= 0 || commCfg.sendPort <= 0 || commCfg.sendIP.empty()) {
        LOG_ERROR("Invalid communication configuration");
        return errorCode;
    }
    --errorCode;

    try {
        // 初始化通信模块
        int ret = configPath.empty() ? communicate::Initialize() : communicate::Initialize(configPath.c_str());
        if (ret != 0) {
            LOG_ERROR("Failed to initialize communication module: {}", ret);
            return errorCode;
        }
        --errorCode;

        // 订阅本地AIS数据
        ret = communicate::SubscribeLocal(nullptr, commCfg.subPort, this);
        if (ret != 0) {
            LOG_ERROR("Failed to subscribe to local AIS data on port {}: {}", commCfg_.subPort, ret);
            return errorCode;
        }
        --errorCode;

        LOG_INFO("AIS communication service initialized: ListenPort={}, Target={}:{}",
                 commCfg.subPort, commCfg.sendIP, commCfg.sendPort);

        isInitialized_ = true;
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during initialization: {}", e.what());
        return errorCode;
    }
}

int AISCommunicationService::handleMsg(std::shared_ptr<void> msg)
{
    if (!isInitialized_ || !aisParser_) {
        return 0;
    }

    try {
        // 假设消息是AIS原始数据字符串
        std::string* aisData = static_cast<std::string*>(msg.get());
        if (!aisData || aisData->empty()) {
            LOG_WARNING("Received empty AIS message");
            return 0;
        }

        LOG_DEBUG("Received AIS data: {}", *aisData);

        // 使用外部提供的AISParser解析消息
        auto parsedMessage = aisParser_->parse(*aisData);
        if (parsedMessage) {
            processAISMessage(*parsedMessage);
        } else {
            LOG_DEBUG("Failed to parse AIS message: {}", *aisData);
        }

        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception in handleMsg: {}", e.what());
        return -1;
    }
}

void AISCommunicationService::processAISMessage(const AISMessage& aisMsg)
{
    std::lock_guard<std::mutex> lock(dataMutex_);

    uint32_t mmsi = aisMsg.mmsi;

    // 更新或创建船舶信息
    auto it = shipInfoMap_.find(mmsi);
    if (it == shipInfoMap_.end())
    {
        shipInfoMap_.emplace(mmsi, aisMsg.toCsv());
        it = shipInfoMap_.find(mmsi);

        LOG_INFO("New ship detected: MMSI={}", mmsi);
    }

    LOG_DEBUG("Updated ship info: MMSI={}, Content={}", it->first, it->second);
}

size_t AISCommunicationService::getShipCount() const
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    return shipInfoMap_.size();
}

void AISCommunicationService::clearShipInfo()
{
    std::lock_guard<std::mutex> lock(dataMutex_);
    shipInfoMap_.clear();
    LOG_INFO("Cleared all ship information");
}

} // namespace ais