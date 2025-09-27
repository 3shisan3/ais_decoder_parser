#include "config_manager.h"

#include <iostream>

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
        
        isLoaded_ = true;
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

// 检查配置是否已加载
bool ConfigManager::isConfigLoaded() const
{
    return isLoaded_;
}

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

// 获取完整的配置节点
const YAML::Node& ConfigManager::getConfigNode()
{
    return configNode_;
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

} // namespace ais