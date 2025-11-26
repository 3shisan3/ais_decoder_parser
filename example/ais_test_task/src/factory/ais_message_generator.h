#ifndef AIS_MESSAGE_GENERATOR_H
#define AIS_MESSAGE_GENERATOR_H

#include <QObject>
#include <QGeoCoordinate>
#include <QDateTime>
#include <string>
#include <memory>

#include "messages/type_definitions.h"
#include "ais_encoder.h"

// using namespace ais;

/**
 * @brief AIS船舶数据结构
 */
struct AISVesselData {
    int mmsi = 0;
    int imo = 0;
    QString callSign;
    QString vesselName;
    int shipType = 0;
    int length = 0;
    int width = 0;
    double draft = 0.0;
    QGeoCoordinate position;
    double speed = 0.0;           // 节
    double heading = 0.0;         // 度
    double courseOverGround = 0.0; // 对地航向（度）
    int navigationStatus = 0;
    int rateOfTurn = 0;
    QString destination;
    int etaMonth = 0;
    int etaDay = 0;
    int etaHour = 0;
    int etaMinute = 0;
    
    // 基站特定字段
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int epfdType = 1;
    
    // 助航设备特定字段
    int aidType = 0;
    QString nameExtension;
    bool offPositionIndicator = false;
    int regional = 0;
    bool virtualAidFlag = false;
    
    // 二进制消息字段
    std::vector<uint8_t> binaryData;
    int designatedAreaCode = 0;
    int functionalId = 0;
    uint32_t destinationMmsi = 0;
};

/**
 * @brief AIS消息生成器类
 * 支持所有27种AIS消息类型的生成
 */
class AISMessageGenerator : public QObject
{
    Q_OBJECT

public:
    explicit AISMessageGenerator(QObject *parent = nullptr);
    
    // 27种AIS消息类型的生成方法
    std::string generateType1Message(const AISVesselData &data);
    std::string generateType2Message(const AISVesselData &data);
    std::string generateType3Message(const AISVesselData &data);
    std::string generateType4Message(const AISVesselData &data);
    std::string generateType5Message(const AISVesselData &data);
    std::string generateType6Message(const AISVesselData &data);
    std::string generateType7Message(const AISVesselData &data);
    std::string generateType8Message(const AISVesselData &data);
    std::string generateType9Message(const AISVesselData &data);
    std::string generateType10Message(const AISVesselData &data);
    std::string generateType11Message(const AISVesselData &data);
    std::string generateType12Message(const AISVesselData &data);
    std::string generateType13Message(const AISVesselData &data);
    std::string generateType14Message(const AISVesselData &data);
    std::string generateType15Message(const AISVesselData &data);
    std::string generateType16Message(const AISVesselData &data);
    std::string generateType17Message(const AISVesselData &data);
    std::string generateType18Message(const AISVesselData &data);
    std::string generateType19Message(const AISVesselData &data);
    std::string generateType20Message(const AISVesselData &data);
    std::string generateType21Message(const AISVesselData &data);
    std::string generateType22Message(const AISVesselData &data);
    std::string generateType23Message(const AISVesselData &data);
    std::string generateType24Message(const AISVesselData &data, int partNumber = 0);
    std::string generateType25Message(const AISVesselData &data);
    std::string generateType26Message(const AISVesselData &data);
    std::string generateType27Message(const AISVesselData &data);

private:
    /**
     * @brief 创建基础AIS消息
     * @param type 消息类型
     * @param data 船舶数据
     * @return 基础AIS消息
     */
    std::unique_ptr<ais::AISMessage> createBaseMessage(ais::AISMessageType type, const AISVesselData &data);
    
    /**
     * @brief 使用AISEncoder编码消息
     * @param message AIS消息对象
     * @return NMEA格式字符串
     */
    std::string encodeMessage(const ais::AISMessage &message);
    
    /**
     * @brief 填充位置报告通用字段
     * @param report 位置报告对象
     * @param data 船舶数据
     */
    void fillPositionReportFields(ais::PositionReport &report, const AISVesselData &data);
    
    /**
     * @brief 填充位置报告通用字段（模板版本）
     */
    template<typename T>
    void fillPositionReportFields(T &report, const AISVesselData &data);
    
    /**
     * @brief 填充静态数据通用字段
     * @param staticData 静态数据对象
     * @param data 船舶数据
     */
    void fillStaticDataFields(ais::StaticVoyageData &staticData, const AISVesselData &data);
    
    /**
     * @brief 生成随机通信状态
     * @return 通信状态值
     */
    int generateCommunicationState();
    
    ais::AISEncoder m_encoder; // AIS编码器
};

#endif // AIS_MESSAGE_GENERATOR_H