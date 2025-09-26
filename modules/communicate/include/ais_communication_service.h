#ifndef AIS_COMMUNICATION_SERVICE_H
#define AIS_COMMUNICATION_SERVICE_H

#include "udp-tcp-communicate/communicate_api.h"

#include "ais_parser.h"
#include "config.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace ais {

/**
 * @brief AIS通信服务类
 * 
 * 负责订阅AIS数据，使用外部提供的AISParser解析数据，
 * 生成船舶综合态势信息并周期性发送到指定目标地址
 */
class AISCommunicationService : public communicate::SubscribebBase
{
public:
    /**
     * @brief 构造函数
     * @param aisParser 外部提供的AIS解析器指针
     */
    explicit AISCommunicationService(std::shared_ptr<AISParser> aisParser);
    
    virtual ~AISCommunicationService();

    /**
     * @brief 初始化通信服务
     * @param commCfg 通信配置信息
     * @param taskId 周期性任务ID（默认1001）
     * @param configPath 用于初始化通信库的配置文件
     * @return 成功返回0，失败返回错误码
     * 
     * @note taskId作用：唯一标识周期性发送任务，用于后续的任务管理（启动、停止、删除）
     */
    int initialize(const CommunicateCfg& commCfg,
                  int taskId = 1001,
                  const std::string& configPath = "");

    /**
     * @brief 启动服务
     * @return 成功返回0，失败返回错误码
     */
    int start();

    /**
     * @brief 停止服务
     * @return 成功返回0，失败返回错误码
     */
    int stop();

    /**
     * @brief 处理接收到的AIS消息（重写基类接口）
     * @param msg 接收到的消息
     * @return 错误码
     */
    virtual int handleMsg(std::shared_ptr<void> msg) override;

    /**
     * @brief 获取当前船舶数量
     * @return 船舶数量
     */
    size_t getShipCount() const;

    /**
     * @brief 清空船舶信息
     */
    void clearShipInfo();

    /**
     * @brief 获取当前任务ID
     * @return 任务ID
     */
    int getTaskId() const { return taskId_; }

private:
    // 船舶信息结构
    struct ShipInfo {
        uint32_t mmsi = 0;
        std::string name;
        double latitude = 0.0;
        double longitude = 0.0;
        double speed = 0.0;        // 节
        double course = 0.0;       // 度
        double heading = 0.0;      // 度
        int shipType = 0;
        uint64_t lastUpdateTime = 0;
        
        // 判断信息是否有效
        bool isValid() const {
            return mmsi != 0 && lastUpdateTime > 0;
        }
    };

    /**
     * @brief 处理AIS消息并更新船舶信息
     * @param aisMsg AIS消息
     */
    void processAISMessage(const AISMessage& aisMsg);

    /**
     * @brief 生成船舶综合态势信息NMEA字符串
     * @param ship 船舶信息
     * @return 生成的NMEA字符串
     */
    std::string generateSituationNMEA(const ShipInfo& ship);

    /**
     * @brief 计算NMEA校验和
     * @param nmea NMEA语句（不包含$和*）
     * @return 校验和字符串
     */
    static std::string calculateChecksum(const std::string& nmea);

private:
    std::shared_ptr<AISParser> aisParser_;          // 外部提供的AIS解析器
    
    // 运行状态
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isRunning_{false};
    
    // 配置信息
    std::string configPath_;
    CommunicateCfg commCfg_;
    int taskId_ = 1001;                             // 周期性任务ID
    
    // 船舶数据
    std::unordered_map<uint32_t, ShipInfo> shipInfoMap_;
    mutable std::mutex dataMutex_;
};

} // namespace ais

#endif // AIS_COMMUNICATION_SERVICE_H