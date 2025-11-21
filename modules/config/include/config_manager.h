#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config.h"

#include <yaml-cpp/yaml.h>

#include <string>
#include <memory>
#include <optional>
#include <functional>

namespace ais
{

/**
 * @brief 配置管理类
 * 负责加载、解析和管理YAML配置文件，并提供各配置结构的访问和修改接口
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
    AISGenerateCfg generateCfg_;                   // 生成nmea消息默认参数
    std::optional<CommunicateCfg> communicateCfg_; // 通讯配置（可选）
    std::string udpTcpCommunicateCfgPath_;         // 通讯库配置文件路径
    
    bool isLoaded_;                                // 配置是否已加载

    // 配置变更回调函数类型
    using ConfigChangeCallback = std::function<void()>;
    ConfigChangeCallback configChangeCallback_;     // 配置变更回调

    // 私有方法
    void parseLoggerConfig();
    void parseParserConfig();
    void parseSaveConfig();
    void parseGenerateConfig();
    void parseCommunicateConfig();
    void parseUdpTcpCommunicateCfgPath();

    /**
     * @brief 触发配置变更回调(可由外部传入函数指针方便配置修改触发一些外部行为)
     */
    void notifyConfigChange();

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
     * @brief 保存配置到文件
     * @return bool true-保存成功, false-保存失败
     */
    bool saveConfig();
    
    /**
     * @brief 检查配置是否已加载
     * @return bool true-已加载, false-未加载
     */
    bool isConfigLoaded() const;
    
    // ============ 获取配置接口 ============
    
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
     * @brief 获取生成器配置
     * @return const AISGenerateCfg& 生成器配置结构体引用
     */
    const AISGenerateCfg& getGenerateConfig();
    
    /**
     * @brief 获取通讯配置
     * @return std::optional<CommunicateCfg> 通讯配置，如果不存在则返回空
     */
    std::optional<CommunicateCfg> getCommunicateConfig();
    
    /**
     * @brief 获取通讯库配置文件路径
     * @return std::string 通讯库配置文件路径，如果不存在则返回空字符串
     */
    std::string getUdpTcpCommunicateCfgPath();
    
    /**
     * @brief 获取完整的配置节点（用于高级操作）
     * @return const YAML::Node& YAML配置节点引用
     */
    const YAML::Node& getConfigNode();
    
    // ============ 修改配置接口 ============
    
    /**
     * @brief 设置日志配置
     * @param cfg 新的日志配置
     */
    void setLoggerConfig(const LoggerCfg& cfg);
    
    /**
     * @brief 设置解析器配置
     * @param cfg 新的解析器配置
     */
    void setParserConfig(const AISParseCfg& cfg);
    
    /**
     * @brief 设置存储配置
     * @param cfg 新的存储配置
     */
    void setSaveConfig(const AISSaveCfg& cfg);
    
    /**
     * @brief 设置生成器配置
     * @param cfg 新的生成器配置
     */
    void setGenerateConfig(const AISGenerateCfg& cfg);
    
    /**
     * @brief 设置通讯配置
     * @param cfg 新的通讯配置，如果传入std::nullopt则清除通讯配置
     */
    void setCommunicateConfig(const std::optional<CommunicateCfg>& cfg);
    
    /**
     * @brief 设置通讯库配置文件路径
     * @param path 新的通讯库配置文件路径
     */
    void setUdpTcpCommunicateCfgPath(const std::string& path);
    
    /**
     * @brief 设置配置变更回调函数
     * @param callback 回调函数，当配置发生变化时调用
     */
    void setConfigChangeCallback(const ConfigChangeCallback& callback);
    
    /**
     * @brief 重置所有配置为默认值
     */
    void resetToDefaults();
};

} // namespace ais

#endif // CONFIG_MANAGER_H