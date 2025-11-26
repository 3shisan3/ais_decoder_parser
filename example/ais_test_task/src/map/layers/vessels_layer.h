#ifndef SSMAP_VESSELS_LAYER_H
#define SSMAP_VESSELS_LAYER_H

#include "base_layer.h"

#include <QGeoCoordinate>
#include <QMap>
#include <QPointF>
#include <QCache>
#include <QDateTime>
#include <QMutex>

#include "messages/message.h"
using namespace ais;

/**
 * @brief 优化的船舶显示信息结构体
 */
struct VesselDisplayInfo {
    int mmsi;
    QString vesselName;
    QGeoCoordinate position;
    double heading;
    double speed;
    QColor color;
    QString vesselId;
    AISMessageType messageType; // 消息类型，用于图标区分
    QDateTime lastUpdateTime;   // 最后更新时间，用于性能优化
    bool needsRedraw = true;    // 是否需要重绘标志
    
    VesselDisplayInfo() 
        : mmsi(0), heading(0.0), speed(0.0), color(Qt::blue), 
          messageType(AISMessageType::POSITION_REPORT_CLASS_A) {}
};

/**
 * @brief 性能优化的多船舶显示图层
 * 支持大规模船舶显示，优化渲染性能
 */
class VesselsLayer : public BaseLayer
{
    Q_OBJECT

public:
    explicit VesselsLayer(QObject *parent = nullptr);
    ~VesselsLayer();

    /**
     * @brief 添加或更新船舶（线程安全）
     * @param vesselId 船舶唯一标识
     * @param info 船舶信息
     */
    void updateVessel(const QString &vesselId, const VesselDisplayInfo &info);

    /**
     * @brief 移除船舶（线程安全）
     * @param vesselId 船舶唯一标识
     */
    void removeVessel(const QString &vesselId);

    /**
     * @brief 清空所有船舶（线程安全）
     */
    void clearVessels();

    /**
     * @brief 获取指定位置的船舶信息
     */
    const VesselDisplayInfo* getVesselAtPosition(
        const QPointF &screenPos,
        const QSize &viewport,
        const QGeoCoordinate &center,
        double zoom,
        const TileForCoord::TileAlgorithm &algorithm) const;

    /**
     * @brief 设置船舶图标大小
     */
    void setVesselIconSize(int size) { m_iconSize = size; m_needsFullRedraw = true; }

    /**
     * @brief 设置是否显示船舶名称
     */
    void setShowVesselNames(bool show) { m_showNames = show; m_needsFullRedraw = true; }

    /**
     * @brief 设置性能优化级别
     * @param level 0-禁用, 1-基础, 2-激进
     */
    void setPerformanceLevel(int level);

    /**
     * @brief 设置最大显示船舶数量
     */
    void setMaxVessels(int max) { m_maxVessels = max; }

    /**
     * @brief 实现基类渲染函数（优化版本）
     */
    void render(QPainter* painter, 
               const QSize& viewport,
               const QGeoCoordinate& center, 
               double zoom,
               const TileForCoord::TileAlgorithm& algorithm) override;

private:
    /**
     * @brief 绘制单个船舶（优化版本）
     */
    void drawVessel(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info);

    /**
     * @brief 绘制船舶工具提示
     */
    void drawTooltip(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info);

    /**
     * @brief 检查点是否在船舶图标范围内
     */
    bool isPointNearVessel(const QPointF &point, const QPointF &vesselPos, double hitRadius) const;

    /**
     * @brief 性能优化：可见性检查
     */
    bool isVesselVisible(const QPointF &screenPos, const QSize &viewport) const;

    /**
     * @brief 性能优化：细节级别控制
     */
    bool shouldDrawDetails(double zoom) const;

    /**
     * @brief 性能优化：清理过时船舶
     */
    void cleanupStaleVessels();

    /**
     * @brief 获取设备类型对应的图标颜色
     */
    QColor getDeviceColor(AISMessageType messageType) const;

    /**
     * @brief 获取设备类型对应的图标形状
     */
    void getDeviceShape(AISMessageType messageType, QPolygonF &shape) const;

private:
    QMap<QString, VesselDisplayInfo> m_vessels;  // 船舶集合
    mutable QMutex m_vesselsMutex;                        // 线程安全锁
    
    int m_iconSize = 20;                                  // 船舶图标大小
    bool m_showNames = true;                              // 是否显示船舶名称
    double m_hitRadius = 15.0;                            // 鼠标碰撞检测半径
    
    // 性能优化相关
    int m_performanceLevel = 1;                           // 性能优化级别
    int m_maxVessels = 200;                               // 最大显示船舶数量
    bool m_needsFullRedraw = true;                        // 是否需要完全重绘
    QRectF m_lastViewport;                                // 上次渲染的视口
    QCache<QString, QPixmap> m_iconCache;                 // 图标缓存
    QSet<QString> m_visibleVessels;                       // 当前可见的船舶
};

#endif // SSMAP_VESSELS_LAYER_H