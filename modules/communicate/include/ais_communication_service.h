#ifndef AIS_COMMUNICATION_SERVICE_H
#define AIS_COMMUNICATION_SERVICE_H

#include "udp-tcp-communicate/communicate_api.h"

#include "ais_parser.h"
#include "config.h"
#include "lru.h"

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
     * @param configPath 用于初始化通信库的配置文件
     * @return 成功返回0，失败返回错误码
     */
    int initialize(const CommunicateCfg& commCfg, const std::string& configPath = "");

    /**
     * @brief 销毁重置服务信息
     */
    void destroy();

    /**
     * @brief 处理接收到的AIS消息（重写基类接口）
     * @param msg 接收到的消息
     * @return 错误码
     */
    virtual int handleMsg(std::shared_ptr<void> msg) override;

    // 获取最新处理结果
    std::string getLastMsgDealResult() const;

    /**
     * @brief 获取当前船舶数量
     * @return 船舶数量
     */
    size_t getShipCount() const;

    /**
     * @brief 清空船舶信息
     */
    void clearShipInfo();

protected:
    /**
     * @brief 处理AIS消息并更新船舶信息
     * @param aisMsg AIS消息
     * 
     * @note 当前存入到shipInfoMap_中，按csv格式存储(额外处理，可自定义实现)
     */
    virtual void processAISMessage(const AISMessage& aisMsg);
    
    // LRU缓存管理船舶信息，key为MMSI，value为CSV格式的船舶信息
    // 使用LRU缓存管理船舶信息，自动控制大小和时效性
    CLRU<uint32_t, std::string, std::mutex> shipInfoCache_;

private:
    std::shared_ptr<AISParser> aisParser_;          // 外部提供的AIS解析器
    
    // 运行状态
    std::atomic<bool> isInitialized_{false};

    // 配置记录
    CommunicateCfg commCfg_;
};

} // namespace ais

#endif // AIS_COMMUNICATION_SERVICE_H