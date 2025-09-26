#include "ais_communication_service.h"

#include "logger_define.h"
#include "messages/type_definitions.h"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace ais {

using namespace std::chrono;

AISCommunicationService::AISCommunicationService(std::shared_ptr<AISParser> aisParser)
    : aisParser_(std::move(aisParser))
{
    if (!aisParser_) {
        LOG_WARNING("AISParser is null, service may not work properly");
    }
}

AISCommunicationService::~AISCommunicationService()
{
    stop();
}

int AISCommunicationService::initialize(const CommunicateCfg& commCfg,
                                       int taskId,
                                       const std::string& configPath)
{
    if (isInitialized_) {
        LOG_WARNING("AISCommunicationService already initialized");
        return 0;
    }

    if (!aisParser_) {
        LOG_ERROR("AISParser is not provided");
        return -1;
    }

    // 验证配置
    if (commCfg.subPort <= 0 || commCfg.sendPort <= 0 || commCfg.sendIP.empty()) {
        LOG_ERROR("Invalid communication configuration");
        return -2;
    }

    try {
        // 保存配置
        configPath_ = configPath;
        commCfg_ = commCfg;
        taskId_ = taskId;

        // 初始化通信模块
        int ret = configPath_.empty() ? communicate::Initialize() : communicate::Initialize(configPath_.c_str());
        if (ret != 0) {
            LOG_ERROR("Failed to initialize communication module: {}", ret);
            return -3;
        }

        // 添加本地监听端口（使用配置中的subPort）
        // ret = communicate::AddListener(nullptr, commCfg_.subPort);
        // if (ret != 0) {
        //     LOG_ERROR("Failed to add local listener on port {}: {}", commCfg_.subPort, ret);
        //     return -4;
        // }

        // 订阅本地AIS数据
        ret = communicate::SubscribeLocal(nullptr, commCfg_.subPort, this);
        if (ret != 0) {
            LOG_ERROR("Failed to subscribe to local AIS data on port {}: {}", commCfg_.subPort, ret);
            return -5;
        }

        LOG_INFO("AIS communication service initialized: ListenPort={}, Target={}:{}, TaskID={}",
                 commCfg_.subPort, commCfg_.sendIP, commCfg_.sendPort, taskId_);

        isInitialized_ = true;
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during initialization: {}", e.what());
        return -6;
    }
}

int AISCommunicationService::start()
{
    if (!isInitialized_) {
        LOG_ERROR("AISCommunicationService not initialized");
        return -1;
    }

    if (isRunning_) {
        LOG_WARNING("AISCommunicationService already running");
        return 0;
    }

    try {
        // 添加周期性发送任务（1Hz）
        // 使用空数据，实际数据在任务函数中生成
        std::vector<uint8_t> emptyData;

        int ret = communicate::AddPeriodicSendTask(
            commCfg_.sendIP.c_str(),
            commCfg_.sendPort,
            emptyData.data(),
            emptyData.size(),
            1, // 1Hz频率
            taskId_ // 使用指定的任务ID
        );

        if (ret != 0) {
            LOG_ERROR("Failed to add periodic send task (ID: {}): {}", taskId_, ret);
            return -2;
        }

        isRunning_ = true;
        LOG_INFO("AISCommunicationService started successfully. TaskID: {}", taskId_);
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during start: {}", e.what());
        return -3;
    }
}

int AISCommunicationService::stop()
{
    if (!isRunning_) {
        return 0;
    }

    try {
        // 移除周期性发送任务
        int ret = communicate::RemovePeriodicSendTask(taskId_);
        if (ret != 0) {
            LOG_WARNING("Failed to remove periodic send task (ID: {}): {}", taskId_, ret);
        }

        isRunning_ = false;
        LOG_INFO("AISCommunicationService stopped. TaskID: {}", taskId_);
        return 0;

    } catch (const std::exception& e) {
        LOG_ERROR("Exception during stop: {}", e.what());
        return -1;
    }
}

int AISCommunicationService::handleMsg(std::shared_ptr<void> msg)
{
    if (!isRunning_ || !aisParser_) {
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

void AISCommunicationService::processAISMessage(const AISMessage& aisMsg)
{
    std::lock_guard<std::mutex> lock(dataMutex_);

    uint32_t mmsi = aisMsg.mmsi;
    auto currentTime = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch()).count();

    // 更新或创建船舶信息
    auto it = shipInfoMap_.find(mmsi);
    if (it == shipInfoMap_.end()) {
        ShipInfo newShip;
        newShip.mmsi = mmsi;
        newShip.lastUpdateTime = currentTime;
        shipInfoMap_.emplace(mmsi, newShip);
        it = shipInfoMap_.find(mmsi);
        
        LOG_INFO("New ship detected: MMSI={}", mmsi);
    }

    ShipInfo& ship = it->second;
    ship.lastUpdateTime = currentTime;

    // 根据消息类型更新船舶信息
    switch (aisMsg.type) {
        case AISMessageType::POSITION_REPORT_CLASS_A:
        case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED:
        case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE:
        {
            const auto& posReport = static_cast<const PositionReport&>(aisMsg);
            ship.latitude = posReport.latitude;
            ship.longitude = posReport.longitude;
            ship.speed = posReport.speedOverGround;
            ship.course = posReport.courseOverGround;
            ship.heading = posReport.trueHeading;
            break;
        }
        case AISMessageType::STATIC_VOYAGE_DATA:
        {
            const auto& staticData = static_cast<const StaticVoyageData&>(aisMsg);
            ship.name = staticData.vesselName;
            ship.shipType = staticData.shipType;
            break;
        }
        case AISMessageType::STANDARD_CLASS_B_CS_POSITION:
        case AISMessageType::EXTENDED_CLASS_B_CS_POSITION:
        {
            const auto& classBReport = static_cast<const StandardClassBReport&>(aisMsg);
            ship.latitude = classBReport.latitude;
            ship.longitude = classBReport.longitude;
            ship.speed = classBReport.speedOverGround;
            ship.course = classBReport.courseOverGround;
            ship.heading = classBReport.trueHeading;
            break;
        }
        case AISMessageType::STATIC_DATA_REPORT:
        {
            const auto& staticReport = static_cast<const StaticDataReport&>(aisMsg);
            if (staticReport.partNumber == 0 && !staticReport.vesselName.empty()) {
                ship.name = staticReport.vesselName;
            }
            break;
        }
        default:
            // 忽略其他消息类型
            LOG_DEBUG("Unhandled AIS message type: {}", static_cast<int>(aisMsg.type));
            break;
    }

    LOG_DEBUG("Updated ship info: MMSI={}, Name={}, Lat={:.6f}, Lon={:.6f}, Speed={:.1f}kn", 
              ship.mmsi, ship.name, ship.latitude, ship.longitude, ship.speed);
}

std::string AISCommunicationService::generateSituationNMEA(const ShipInfo& ship)
{
    if (!ship.isValid()) {
        return "";
    }

    // 获取当前时间戳（Unix时间戳，秒.毫秒）
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();
    
    // 修复：使用完全限定的类型名称避免冲突
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - 
                     std::chrono::duration_cast<std::chrono::milliseconds>(secs);
    
    double timestamp = secs.count() + millisecs.count() / 1000.0;

    // 构建NMEA语句
    std::ostringstream nmeaStream;
    nmeaStream << "USVSITUATION,"
               << std::fixed << std::setprecision(3) << timestamp << ","
               << ship.mmsi << ","
               << (ship.name.empty() ? "UNKNOWN" : ship.name) << ","
               << "5," // Source: AIS
               << "1," // Type: 船舶
               << ship.mmsi << ","
               << (ship.name.empty() ? "UNKNOWN" : ship.name) << ","
               << std::setprecision(2) << ship.course << ","
               << "0.0," // Distance (暂设为0)
               << "0.0," // Height (暂设为0)
               << "0.0," // Diameter (暂设为0)
               << std::setprecision(7) << ship.latitude << ","
               << std::setprecision(7) << ship.longitude << ","
               << std::setprecision(2) << ship.heading << ","
               << std::setprecision(1) << (ship.speed * 0.5144) << "," // 节转米/秒
               << "NAVIGATING"; // Other info

    std::string nmeaBody = nmeaStream.str();
    std::string checksum = calculateChecksum(nmeaBody);

    // 完整NMEA语句
    std::ostringstream fullNmea;
    fullNmea << "$" << nmeaBody << "*" << checksum << "\r\n";

    return fullNmea.str();
}

std::string AISCommunicationService::calculateChecksum(const std::string& nmea)
{
    uint8_t checksum = 0;
    for (char c : nmea) {
        checksum ^= static_cast<uint8_t>(c);
    }

    std::ostringstream hexStream;
    hexStream << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
              << static_cast<int>(checksum);
    return hexStream.str();
}

} // namespace ais