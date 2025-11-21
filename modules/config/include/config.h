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
    bool saveSwitch = false;                    // 启用本地缓存
    StorageType storageType = StorageType::CSV; // 存储类型
    std::string storagePath = "ais_data.csv";   // 存储路径
};

/**
 * @brief AIS消息生成配置
 */
struct AISGenerateCfg
{
    bool enableFragmentation = false;           // 是否启用分片
    int defaultFragmentSize = 64;               // 默认分片大小（字符）
    char defaultChannel = 'A';                  // 默认信道
    std::string defaultSequenceId = "";         // 默认序列ID
};

/**
 * @brief AIS通讯配置结构体
 */
struct CommunicateCfg
{
    int subPort;        // 本地监听信息端口
    std::string sendIP; // 对外发送目标IP
    int sendPort;       // 对外发送目标端口

    int msgSaveSize;    // 本地保留消息最大长度（设置非正整数表示 不限制存储数量）
    int msgSaveTime;    // 本地保留消息的有效时间（单位秒）（设置非正整数表示 永久保留）
};

} // namespace ais

#endif // AIS_CONFIG_H