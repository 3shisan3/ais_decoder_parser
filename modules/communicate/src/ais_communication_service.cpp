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
    , shipInfoCache_(0, 0, 0)  // 默认值：不限制大小，和存活时间
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
        // 根据配置初始化LRU缓存参数
        // 处理缓存大小限制：msgSaveSize <= 0 表示不限制大小
        size_t maxSize = 0;  // 0表示不限制
        size_t elasticity = 0;
        
        if (commCfg.msgSaveSize > 0) {
            maxSize = static_cast<size_t>(commCfg.msgSaveSize);
            elasticity = maxSize > 100 ? maxSize / 10 : 10;  // 弹性大小为10%，最小10
        }
        
        // 处理存活时间限制：msgSaveTime <= 0 表示不限制时间
        time_t maxTimeSpan = 0;  // 0表示不限制
        
        if (commCfg.msgSaveTime > 0) {
            maxTimeSpan = static_cast<time_t>(commCfg.msgSaveTime);
        }
        
        // 使用Reset方法重新配置LRU缓存
        shipInfoCache_.Reset(maxSize, elasticity, maxTimeSpan);
        
        if (maxSize > 0 && maxTimeSpan > 0) {
            LOG_INFO("LRU cache configured: MaxSize={}, Elasticity={}, MaxTimeSpan={}s", 
                     maxSize, elasticity, maxTimeSpan);
        } else if (maxSize > 0) {
            LOG_INFO("LRU cache configured: MaxSize={}, Elasticity={}, No time limit", 
                     maxSize, elasticity);
        } else if (maxTimeSpan > 0) {
            LOG_INFO("LRU cache configured: No size limit, MaxTimeSpan={}s", maxTimeSpan);
        } else {
            LOG_INFO("LRU cache configured: No size or time limits (unlimited cache)");
        }

        // 初始化通信模块
        int ret = configPath.empty() ? communicate::Initialize() : communicate::Initialize(configPath.c_str());
        if (ret != 0) {
            LOG_ERROR("Failed to initialize communication module: {}", ret);
            return errorCode;
        }
        --errorCode;

        commCfg_ = commCfg;

        // 订阅本地AIS数据
        ret = communicate::SubscribeLocal("127.0.0.1", commCfg.subPort, this);
        if (ret != 0) {
            LOG_ERROR("Failed to subscribe to local AIS data on port {}: {}", commCfg.subPort, ret);
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

void AISCommunicationService::destroy()
{
    if (!isInitialized_) {
        return;
    }

    aisParser_.reset();
    aisParser_ = nullptr;

    communicate::Destroy();
}

int AISCommunicationService::handleMsg(std::shared_ptr<void> msg)
{
    if (!isInitialized_ || !aisParser_) {
        return 0;
    }

    try {
        // 假设消息是AIS原始数据字符串
        const char* aisData = static_cast<const char*>(msg.get());
        if (!aisData) {
            LOG_WARNING("Received empty AIS message");
            return 0;
        }

        LOG_DEBUG("Received AIS data: {}", aisData);

        // 使用外部提供的AISParser解析消息
        auto parsedMessage = aisParser_->parse(aisData);
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
    uint32_t mmsi = aisMsg.mmsi;
    std::string csvData = aisMsg.toCsv();

    if (communicate::SendGeneralMessage(csvData.data(), csvData.size() + 1, 
                                        commCfg_.sendIP.data(), commCfg_.sendPort) != 0)
    {
        // 使用LRU缓存自动管理船舶信息
        bool inserted = shipInfoCache_.Insert(mmsi, csvData);
        
        if (inserted) {
            LOG_INFO("New/Updated ship info: MMSI={}", mmsi);
        } else {
            LOG_WARNING("Failed to insert ship info into cache: MMSI={}", mmsi);
        }
    }
    else
    {
        LOG_ERROR("Failed to send ship info: MMSI={}", mmsi);
    }

    LOG_DEBUG("Processed ship info: MMSI={}, Content={}", mmsi, csvData);
}

size_t AISCommunicationService::getShipCount() const
{
    return shipInfoCache_.GetSize();
}

void AISCommunicationService::clearShipInfo()
{
    shipInfoCache_.Clear();
    LOG_INFO("Cleared all ship information");
}

std::string AISCommunicationService::getLastMsgDealResult() const
{
    // 获取最新处理结果
    if (shipInfoCache_.IsEmpty()) {
        return "";
    }
    
    return shipInfoCache_.GetLatest().value().second;
}

} // namespace ais