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
     * @param configPath 用于初始化通信库的配置文件
     * @return 成功返回0，失败返回错误码
     */
    int initialize(const CommunicateCfg& commCfg, const std::string& configPath = "");

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

protected:
    /**
     * @brief 处理AIS消息并更新船舶信息
     * @param aisMsg AIS消息
     * 
     * @note 当前存入到shipInfoMap_中，按csv格式存储(额外处理，可自定义实现)
     */
    void processAISMessage(const AISMessage& aisMsg);

    // AIS数据/按船舶编号（mssi）记录
    std::unordered_map<uint32_t, std::string> shipInfoMap_;
    mutable std::mutex dataMutex_;

private:
    std::shared_ptr<AISParser> aisParser_;          // 外部提供的AIS解析器
    
    // 运行状态
    std::atomic<bool> isInitialized_{false};
};

} // namespace ais

#endif // AIS_COMMUNICATION_SERVICE_H