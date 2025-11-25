#ifndef SSMAP_VESSELS_LAYER_H
#define SSMAP_VESSELS_LAYER_H

#include "base_layer.h"
#include <QGeoCoordinate>
#include <QMap>
#include <QPointF>

/**
 * @brief 船舶信息结构体
 */
struct VesselDisplayInfo {
    int mmsi;                       // MMSI编号
    QString vesselName;             // 船舶名称
    QGeoCoordinate position;        // 位置
    double heading;                 // 航向(度)
    double speed;                   // 速度(节)
    QColor color;                   // 显示颜色
    QString vesselId;               // 船舶ID
    
    VesselDisplayInfo() 
        : mmsi(0), heading(0.0), speed(0.0), color(Qt::blue) {}
};

/**
 * @brief 多船舶显示图层
 * 支持显示多个船舶，并提供鼠标悬停信息提示
 */
class VesselsLayer : public BaseLayer
{
    Q_OBJECT
public:
    explicit VesselsLayer(QObject *parent = nullptr);

    /**
     * @brief 添加或更新船舶
     * @param vesselId 船舶唯一标识
     * @param info 船舶信息
     */
    void updateVessel(const QString &vesselId, const VesselDisplayInfo &info);

    /**
     * @brief 移除船舶
     * @param vesselId 船舶唯一标识
     */
    void removeVessel(const QString &vesselId);

    /**
     * @brief 清空所有船舶
     */
    void clearVessels();

    /**
     * @brief 获取指定位置的船舶信息
     * @param screenPos 屏幕坐标
     * @param viewport 视口大小
     * @param center 地图中心
     * @param zoom 缩放级别
     * @param algorithm 坐标算法
     * @return 船舶信息指针，如果没有则返回nullptr
     */
    const VesselDisplayInfo* getVesselAtPosition(
        const QPointF &screenPos,
        const QSize &viewport,
        const QGeoCoordinate &center,
        double zoom,
        const TileForCoord::TileAlgorithm &algorithm) const;

    /**
     * @brief 设置船舶图标大小
     * @param size 图标大小（像素）
     */
    void setVesselIconSize(int size) { m_iconSize = size; }

    /**
     * @brief 设置是否显示船舶名称
     * @param show 是否显示
     */
    void setShowVesselNames(bool show) { m_showNames = show; }

    /**
     * @brief 实现基类渲染函数
     */
    void render(QPainter* painter, 
               const QSize& viewport,
               const QGeoCoordinate& center, 
               double zoom,
               const TileForCoord::TileAlgorithm& algorithm) override;

signals:
    /**
     * @brief 船舶被点击时发出
     * @param vesselId 船舶ID
     * @param info 船舶信息
     */
    void vesselClicked(const QString &vesselId, const VesselDisplayInfo &info);

private:
    /**
     * @brief 绘制单个船舶
     * @param painter 绘图设备
     * @param screenPos 屏幕坐标
     * @param info 船舶信息
     */
    void drawVessel(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info);

    /**
     * @brief 绘制船舶工具提示
     * @param painter 绘图设备
     * @param screenPos 屏幕坐标
     * @param info 船舶信息
     */
    void drawTooltip(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info);

    /**
     * @brief 检查点是否在船舶图标范围内
     * @param point 测试点
     * @param vesselPos 船舶位置
     * @param hitRadius 碰撞半径
     * @return 是否在范围内
     */
    bool isPointNearVessel(const QPointF &point, const QPointF &vesselPos, double hitRadius) const;

    QMap<QString, VesselDisplayInfo> m_vessels;  // 船舶集合，key为vesselId
    int m_iconSize = 20;                         // 船舶图标大小
    bool m_showNames = true;                     // 是否显示船舶名称
    double m_hitRadius = 15.0;                   // 鼠标碰撞检测半径
};

#endif // SSMAP_VESSELS_LAYER_H