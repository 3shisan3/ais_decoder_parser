#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config.h"

#include <yaml-cpp/yaml.h>

#include <string>
#include <memory>
#include <optional>

namespace ais
{

/**
 * @brief 配置管理类
 * 负责加载、解析和管理YAML配置文件，并提供各配置结构的访问接口
 * 如果配置不存在，则使用默认值；通讯配置可能返回空指针
 */
class ConfigManager
{
private:
    std::string configFile_;                       // 配置文件路径
    YAML::Node configNode_;                        // YAML配置节点
    
    LoggerCfg loggerCfg_;                          // 日志配置
    AISParseCfg parseCfg_;                         // 解析器配置
    AISSaveCfg saveCfg_;                           // 存储配置
    std::optional<CommunicateCfg> communicateCfg_; // 通讯配置（可选）
    
    bool isLoaded_;                                // 配置是否已加载

    // 私有方法
    void parseLoggerConfig();
    void parseParserConfig();
    void parseSaveConfig();
    void parseCommunicateConfig();

public:
    /**
     * @brief 构造函数
     * @param configFile 配置文件路径
     */
    explicit ConfigManager(const std::string& configFile = "ais_config.yaml");
    
    /**
     * @brief 析构函数
     */
    virtual ~ConfigManager();
    
    /**
     * @brief 加载配置文件
     * @return bool true-加载成功, false-加载失败
     */
    bool loadConfig();
    
    /**
     * @brief 重新加载配置文件
     * @return bool true-加载成功, false-加载失败
     */
    bool reloadConfig();
    
    /**
     * @brief 检查配置是否已加载
     * @return bool true-已加载, false-未加载
     */
    bool isConfigLoaded() const;
    
    /**
     * @brief 获取日志配置
     * @return const LoggerCfg& 日志配置结构体引用
     */
    const LoggerCfg& getLoggerConfig();
    
    /**
     * @brief 获取解析器配置
     * @return const AISParseCfg& 解析器配置结构体引用
     */
    const AISParseCfg& getParserConfig();
    
    /**
     * @brief 获取存储配置
     * @return const AISSaveCfg& 存储配置结构体引用
     */
    const AISSaveCfg& getSaveConfig();
    
    /**
     * @brief 获取通讯配置
     * @return std::optional<CommunicateCfg> 通讯配置，如果不存在则返回空
     */
    std::optional<CommunicateCfg> getCommunicateConfig();
    
    /**
     * @brief 获取完整的配置节点（用于高级操作）
     * @return const YAML::Node& YAML配置节点引用
     */
    const YAML::Node& getConfigNode();
};

} // namespace ais

#endif // CONFIG_MANAGER_H