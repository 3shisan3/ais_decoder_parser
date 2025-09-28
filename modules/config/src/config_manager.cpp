#include "config_manager.h"

#include <iostream>
#include <fstream>

namespace ais
{

// ConfigManager 构造函数
ConfigManager::ConfigManager(const std::string& configFile)
    : configFile_(configFile), isLoaded_(false)
{
    // 初始化默认值
    loggerCfg_ = {true, "ais_parser.log"};
    parseCfg_ = {true, true, 300};
    saveCfg_ = {false, StorageType::CSV, "ais_data.csv"};
    communicateCfg_ = std::nullopt;
    udpTcpCommunicateCfgPath_ = "";
}

// ConfigManager 析构函数
ConfigManager::~ConfigManager()
{
    // 清理资源（如果需要）
}

// 加载配置文件
bool ConfigManager::loadConfig()
{
    try {
        // 加载YAML文件
        configNode_ = YAML::LoadFile(configFile_);
        
        // 解析各配置段
        parseLoggerConfig();
        parseParserConfig();
        parseSaveConfig();
        parseCommunicateConfig();
        parseUdpTcpCommunicateCfgPath();
        
        isLoaded_ = true;
        notifyConfigChange();
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "Warning: Failed to load config file: " << e.what() 
                  << ", using default values." << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Warning: Config parsing error: " << e.what() 
                  << ", using default values." << std::endl;
        return false;
    }
}

// 重新加载配置文件
bool ConfigManager::reloadConfig()
{
    isLoaded_ = false;
    return loadConfig();
}

// 保存配置到文件
bool ConfigManager::saveConfig()
{
    try {
        // 更新YAML节点
        // 日志配置
        configNode_["ais"]["logger"]["enableLogging"] = loggerCfg_.enableLogging;
        configNode_["ais"]["logger"]["logFile"] = loggerCfg_.logFile;
        
        // 解析器配置
        configNode_["ais"]["parser"]["validateChecksum"] = parseCfg_.validateChecksum;
        configNode_["ais"]["parser"]["enableMultipartReassembly"] = parseCfg_.enableMultipartReassembly;
        configNode_["ais"]["parser"]["maxMultipartAge"] = parseCfg_.maxMultipartAge;
        
        // 存储配置
        configNode_["ais"]["save"]["saveSwitch"] = saveCfg_.saveSwitch;
        
        // 存储类型枚举转字符串
        std::string storageTypeStr;
        switch (saveCfg_.storageType) {
            case StorageType::NONE: storageTypeStr = "NONE"; break;
            case StorageType::DATABASE: storageTypeStr = "DATABASE"; break;
            case StorageType::CSV: storageTypeStr = "CSV"; break;
            case StorageType::MEMORY: storageTypeStr = "MEMORY"; break;
            default: storageTypeStr = "CSV";
        }
        configNode_["ais"]["save"]["storageType"] = storageTypeStr;
        configNode_["ais"]["save"]["storagePath"] = saveCfg_.storagePath;
        
        // 通讯配置
        if (communicateCfg_.has_value()) {
            configNode_["ais"]["communicate"]["subPort"] = communicateCfg_->subPort;
            configNode_["ais"]["communicate"]["sendIP"] = communicateCfg_->sendIP;
            configNode_["ais"]["communicate"]["sendPort"] = communicateCfg_->sendPort;
            configNode_["ais"]["communicate"]["msgSaveSize"] = communicateCfg_->msgSaveSize;
            configNode_["ais"]["communicate"]["msgSaveTime"] = communicateCfg_->msgSaveTime;
        }
        
        // 通讯库配置文件路径
        configNode_["udp_tcp_communicate_cfg_path"] = udpTcpCommunicateCfgPath_;
        
        // 写入文件
        std::ofstream fout(configFile_);
        fout << configNode_;
        fout.close();
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to save config file: " << e.what() << std::endl;
        return false;
    }
}

// 检查配置是否已加载
bool ConfigManager::isConfigLoaded() const
{
    return isLoaded_;
}

// ============ 获取配置接口 ============

// 获取日志配置
const LoggerCfg& ConfigManager::getLoggerConfig()
{
    return loggerCfg_;
}

// 获取解析器配置
const AISParseCfg& ConfigManager::getParserConfig()
{
    return parseCfg_;
}

// 获取存储配置
const AISSaveCfg& ConfigManager::getSaveConfig()
{
    return saveCfg_;
}

// 获取通讯配置
std::optional<CommunicateCfg> ConfigManager::getCommunicateConfig()
{
    return communicateCfg_;
}

// 获取通讯库配置文件路径
std::string ConfigManager::getUdpTcpCommunicateCfgPath()
{
    return udpTcpCommunicateCfgPath_;
}

// 获取完整的配置节点
const YAML::Node& ConfigManager::getConfigNode()
{
    return configNode_;
}

// ============ 修改配置接口 ============

// 设置日志配置
void ConfigManager::setLoggerConfig(const LoggerCfg& cfg)
{
    loggerCfg_ = cfg;
    notifyConfigChange();
}

// 设置解析器配置
void ConfigManager::setParserConfig(const AISParseCfg& cfg)
{
    parseCfg_ = cfg;
    notifyConfigChange();
}

// 设置存储配置
void ConfigManager::setSaveConfig(const AISSaveCfg& cfg)
{
    saveCfg_ = cfg;
    notifyConfigChange();
}

// 设置通讯配置
void ConfigManager::setCommunicateConfig(const std::optional<CommunicateCfg>& cfg)
{
    communicateCfg_ = cfg;
    notifyConfigChange();
}

// 设置通讯库配置文件路径
void ConfigManager::setUdpTcpCommunicateCfgPath(const std::string& path)
{
    udpTcpCommunicateCfgPath_ = path;
    notifyConfigChange();
}

// 设置配置变更回调函数
void ConfigManager::setConfigChangeCallback(const ConfigChangeCallback& callback)
{
    configChangeCallback_ = callback;
}

// 重置所有配置为默认值
void ConfigManager::resetToDefaults()
{
    loggerCfg_ = {true, "ais_parser.log"};
    parseCfg_ = {true, true, 300};
    saveCfg_ = {false, StorageType::CSV, "ais_data.csv"};
    communicateCfg_ = std::nullopt;
    udpTcpCommunicateCfgPath_ = "";
    notifyConfigChange();
}

// ============ 私有方法 ============

// 触发配置变更回调
void ConfigManager::notifyConfigChange()
{
    if (configChangeCallback_) {
        configChangeCallback_();
    }
}

// 解析日志配置
void ConfigManager::parseLoggerConfig()
{
    try {
        const auto& node = configNode_["ais"]["logger"];
        if (node && node.IsMap()) {
            if (node["enableLogging"]) {
                loggerCfg_.enableLogging = node["enableLogging"].as<bool>();
            }
            if (node["logFile"]) {
                loggerCfg_.logFile = node["logFile"].as<std::string>();
            }
        }
    } catch (...) {
        // 忽略解析错误，使用默认值
    }
}

// 解析解析器配置
void ConfigManager::parseParserConfig()
{
    try {
        const auto& node = configNode_["ais"]["parser"];
        if (node && node.IsMap()) {
            if (node["validateChecksum"]) {
                parseCfg_.validateChecksum = node["validateChecksum"].as<bool>();
            }
            if (node["enableMultipartReassembly"]) {
                parseCfg_.enableMultipartReassembly = node["enableMultipartReassembly"].as<bool>();
            }
            if (node["maxMultipartAge"]) {
                parseCfg_.maxMultipartAge = node["maxMultipartAge"].as<int>();
            }
        }
    } catch (...) {
        // 忽略解析错误，使用默认值
    }
}

// 解析存储配置
void ConfigManager::parseSaveConfig()
{
    try {
        const auto& node = configNode_["ais"]["save"];
        if (node && node.IsMap()) {
            if (node["saveSwitch"]) {
                saveCfg_.saveSwitch = node["saveSwitch"].as<bool>();
            }
            
            // 解析存储类型字符串到枚举
            if (node["storageType"]) {
                std::string typeStr = node["storageType"].as<std::string>();
                if (typeStr == "NONE") {
                    saveCfg_.storageType = StorageType::NONE;
                } else if (typeStr == "DATABASE") {
                    saveCfg_.storageType = StorageType::DATABASE;
                } else if (typeStr == "CSV") {
                    saveCfg_.storageType = StorageType::CSV;
                } else if (typeStr == "MEMORY") {
                    saveCfg_.storageType = StorageType::MEMORY;
                }
            }
            
            if (node["storagePath"]) {
                saveCfg_.storagePath = node["storagePath"].as<std::string>();
            }
        }
    } catch (...) {
        // 忽略解析错误，使用默认值
    }
}

// 解析通讯配置
void ConfigManager::parseCommunicateConfig()
{
    try {
        const auto& node = configNode_["ais"]["communicate"];
        if (node && node.IsMap()) {
            CommunicateCfg cfg;
            
            if (node["subPort"]) {
                cfg.subPort = node["subPort"].as<int>();
            }
            if (node["sendIP"]) {
                cfg.sendIP = node["sendIP"].as<std::string>();
            }
            if (node["sendPort"]) {
                cfg.sendPort = node["sendPort"].as<int>();
            }
            if (node["msgSaveSize"]) {
                cfg.msgSaveSize = node["msgSaveSize"].as<int>();
            }
            if (node["msgSaveTime"]) {
                cfg.msgSaveTime = node["msgSaveTime"].as<int>();
            }
            
            communicateCfg_ = cfg;
        } else {
            communicateCfg_ = std::nullopt;
        }
    } catch (...) {
        // 忽略解析错误，通讯配置保持为空
        communicateCfg_ = std::nullopt;
    }
}

// 解析通讯库配置文件路径
void ConfigManager::parseUdpTcpCommunicateCfgPath()
{
    try {
        if (configNode_["udp_tcp_communicate_cfg_path"]) {
            udpTcpCommunicateCfgPath_ = configNode_["udp_tcp_communicate_cfg_path"].as<std::string>();
        } else {
            udpTcpCommunicateCfgPath_ = "";
        }
    } catch (...) {
        // 忽略解析错误，使用默认值（空字符串）
        udpTcpCommunicateCfgPath_ = "";
    }
}

} // namespace ais