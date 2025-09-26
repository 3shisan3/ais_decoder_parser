#ifndef AIS_CONFIG_H
#define AIS_CONFIG_H

#include <string>

namespace ais
{

/**
 * @brief 日志内容配置结构体
 */
struct LoggerCfg
{
    bool enableLogging = true;                  // 是否启用日志
    std::string logFile = "ais_parser.log";     // 日志文件路径
};

/**
 * @brief AIS解析器配置结构体
 */
struct AISParseCfg
{
    bool validateChecksum = true;               // 是否验证校验和
    bool enableMultipartReassembly = true;      // 是否启用多部分消息重组
    int maxMultipartAge = 300;                  // 多部分消息最大保留时间(秒)
};

/**
 * @brief 存储类型枚举
 */
enum class StorageType
{
    NONE,     // 不存储
    DATABASE, // SQLite数据库
    CSV,      // CSV文件
    MEMORY    // 内存存储（调试用）
};

/**
 * @brief AIS数据本地化配置结构体
 */
struct AISSaveCfg
{
    StorageType storageType = StorageType::CSV; // 存储类型
    std::string storagePath = "ais_data.csv";   // 存储路径
};

/**
 * @brief AIS通讯配置结构体
 */
struct CommunicateCfg
{
    int subPort;        // 本地监听信息端口
    std::string sendIP; // 对外发送目标IP
    int sendPort;       // 对外发送目标端口
};

} // namespace ais

#endif // AIS_CONFIG_H