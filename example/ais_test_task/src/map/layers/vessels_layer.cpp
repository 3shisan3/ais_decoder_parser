#include "vessels_layer.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QDateTime>
#include <QElapsedTimer>

VesselsLayer::VesselsLayer(QObject *parent)
    : BaseLayer(parent)
    , m_iconCache(100) // 缓存100个图标
{
    setPerformanceLevel(1);
}

VesselsLayer::~VesselsLayer()
{
    m_iconCache.clear();
}

void VesselsLayer::updateVessel(const QString &vesselId, const VesselDisplayInfo &info)
{
    QMutexLocker locker(&m_vesselsMutex);
    
    // 性能优化：限制最大船舶数量
    if (m_vessels.size() >= m_maxVessels && !m_vessels.contains(vesselId)) {
        // 移除最旧的船舶
        QString oldestId;
        QDateTime oldestTime = QDateTime::currentDateTime();
        
        for (auto it = m_vessels.constBegin(); it != m_vessels.constEnd(); ++it) {
            if (it.value().lastUpdateTime < oldestTime) {
                oldestTime = it.value().lastUpdateTime;
                oldestId = it.key();
            }
        }
        
        if (!oldestId.isEmpty()) {
            m_vessels.remove(oldestId);
            m_iconCache.remove(oldestId); // 同时移除缓存
        }
    }
    
    VesselDisplayInfo newInfo = info;
    newInfo.lastUpdateTime = QDateTime::currentDateTime();
    newInfo.needsRedraw = true;
    
    m_vessels[vesselId] = newInfo;
    m_needsFullRedraw = true;
    
    emit updateRequested();
}

void VesselsLayer::removeVessel(const QString &vesselId)
{
    QMutexLocker locker(&m_vesselsMutex);
    if (m_vessels.remove(vesselId) > 0) {
        m_iconCache.remove(vesselId);
        m_needsFullRedraw = true;
        emit updateRequested();
    }
}

void VesselsLayer::clearVessels()
{
    QMutexLocker locker(&m_vesselsMutex);
    if (!m_vessels.isEmpty()) {
        m_vessels.clear();
        m_iconCache.clear();
        m_visibleVessels.clear();
        m_needsFullRedraw = true;
        emit updateRequested();
    }
}

const VesselDisplayInfo* VesselsLayer::getVesselAtPosition(
    const QPointF &screenPos,
    const QSize &viewport,
    const QGeoCoordinate &center,
    double zoom,
    const TileForCoord::TileAlgorithm &algorithm) const
{
    if (!isVisible() || m_vessels.isEmpty()) {
        return nullptr;
    }

    QMutexLocker locker(&m_vesselsMutex);

    // 计算视口中心偏移
    QPointF centerPixel = algorithm.latLongToPixelXY(
        center.longitude(), center.latitude(), qFloor(zoom));
    QPointF viewportCenter(viewport.width() / 2.0, viewport.height() / 2.0);
    QPointF offset = viewportCenter - centerPixel;

    // 遍历所有船舶，查找最近的
    for (auto it = m_vessels.constBegin(); it != m_vessels.constEnd(); ++it) {
        const VesselDisplayInfo &info = it.value();
        
        if (!info.position.isValid()) {
            continue;
        }

        // 计算船舶在屏幕上的位置
        QPointF vesselPixel = algorithm.latLongToPixelXY(
            info.position.longitude(), info.position.latitude(), qFloor(zoom));
        QPointF vesselScreenPos = vesselPixel + offset;

        // 检测是否在碰撞范围内
        if (isPointNearVessel(screenPos, vesselScreenPos, m_hitRadius)) {
            return &info;
        }
    }

    return nullptr;
}

void VesselsLayer::setPerformanceLevel(int level)
{
    m_performanceLevel = qBound(0, level, 2);
    
    switch (m_performanceLevel) {
    case 0: // 禁用优化
        m_maxVessels = 500;
        break;
    case 1: // 基础优化
        m_maxVessels = 200;
        break;
    case 2: // 激进优化
        m_maxVessels = 100;
        break;
    }
    
    m_needsFullRedraw = true;
}

void VesselsLayer::render(QPainter *painter,
                                  const QSize &viewport,
                                  const QGeoCoordinate &center,
                                  double zoom,
                                  const TileForCoord::TileAlgorithm &algorithm)
{
    if (m_vessels.isEmpty() || !isVisible()) {
        return;
    }

    QMutexLocker locker(&m_vesselsMutex);

    QElapsedTimer timer;
    if (m_performanceLevel > 0) {
        timer.start();
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, m_performanceLevel < 2);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, m_performanceLevel < 2);

    // 计算视口中心偏移
    QPointF centerPixel = algorithm.latLongToPixelXY(
        center.longitude(), center.latitude(), qFloor(zoom));
    QPointF viewportCenter(viewport.width() / 2.0, viewport.height() / 2.0);
    QPointF offset = viewportCenter - centerPixel;

    // 性能优化：可见区域计算
    QRectF visibleRect(-50, -50, viewport.width() + 100, viewport.height() + 100);
    QSet<QString> currentVisibleVessels;

    int vesselsDrawn = 0;
    int vesselsSkipped = 0;

    // 绘制所有船舶
    for (auto it = m_vessels.begin(); it != m_vessels.end(); ++it) {
        VesselDisplayInfo &info = it.value();
        
        if (!info.position.isValid()) {
            continue;
        }

        // 计算船舶在屏幕上的位置
        QPointF vesselPixel = algorithm.latLongToPixelXY(
            info.position.longitude(), info.position.latitude(), qFloor(zoom));
        QPointF vesselScreenPos = vesselPixel + offset;

        // 性能优化：可见性检查
        if (!visibleRect.contains(vesselScreenPos)) {
            vesselsSkipped++;
            continue;
        }

        currentVisibleVessels.insert(it.key());

        // 性能优化：细节级别控制
        bool drawDetails = shouldDrawDetails(zoom) && m_showNames;
        
        // 检查是否需要重绘
        bool needsRedraw = m_needsFullRedraw || info.needsRedraw || 
                          !m_visibleVessels.contains(it.key());

        // 绘制船舶
        drawVessel(painter, vesselScreenPos, info);
        
        // 绘制名称（根据性能级别决定）
        if (drawDetails && m_performanceLevel < 2) {
            // 名称绘制逻辑...
        }

        info.needsRedraw = false;
        vesselsDrawn++;
    }

    m_visibleVessels = currentVisibleVessels;
    m_needsFullRedraw = false;

    painter->restore();

    // 性能监控
    if (m_performanceLevel > 0 && timer.elapsed() > 16) { // 超过16ms警告
        qDebug() << "VesselsLayer render time:" << timer.elapsed() << "ms, drawn:" 
                 << vesselsDrawn << "skipped:" << vesselsSkipped;
    }

    emit renderingComplete();
}

void VesselsLayer::drawVessel(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info)
{
    painter->save();

    // 移动到船舶位置
    painter->translate(screenPos);
    
    // 旋转到航向角度（移动的船舶才需要旋转）
    if (info.speed > 0.5) {
        painter->rotate(info.heading);
    }

    // 获取设备颜色和形状
    QColor color = getDeviceColor(info.messageType);
    QPolygonF vesselShape;
    getDeviceShape(info.messageType, vesselShape);

    // 填充颜色
    painter->setBrush(color);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawPolygon(vesselShape);

    // 绘制外边框（增强可见性）
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(vesselShape);

    painter->restore();

    // 性能优化：只在需要时绘制速度指示线
    if (info.speed > 0.5 && m_performanceLevel < 2) {
        painter->save();
        painter->translate(screenPos);
        painter->rotate(info.heading);
        
        // 根据速度调整线长
        double lineLength = m_iconSize * (1.0 + info.speed / 10.0);
        lineLength = qMin(lineLength, m_iconSize * 3.0);
        
        QPen speedLinePen(color, 2, Qt::DashLine);
        painter->setPen(speedLinePen);
        painter->drawLine(QPointF(0, -m_iconSize * 0.75), 
                         QPointF(0, -lineLength));
        
        painter->restore();
    }
}

void VesselsLayer::drawTooltip(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info)
{
    if (m_performanceLevel == 2) return; // 激进优化模式下不显示tooltip

    painter->save();

    // 构建提示文本
    QStringList tooltipLines;
    tooltipLines << QString("MMSI: %1").arg(info.mmsi);
    tooltipLines << QString("Name: %1").arg(info.vesselName);
    tooltipLines << QString("Type: %1").arg(static_cast<int>(info.messageType));
    tooltipLines << QString("Position: %1, %2")
                    .arg(info.position.latitude(), 0, 'f', 6)
                    .arg(info.position.longitude(), 0, 'f', 6);
    
    if (info.speed > 0) {
        tooltipLines << QString("Heading: %1°").arg(info.heading, 0, 'f', 1);
        tooltipLines << QString("Speed: %1 kts").arg(info.speed, 0, 'f', 1);
    }

    // 设置字体
    QFont font = painter->font();
    font.setPointSize(10);
    painter->setFont(font);

    // 计算提示框大小
    QFontMetrics fm(font);
    int maxWidth = 0;
    int totalHeight = 0;
    QList<QRectF> lineRects;
    
    for (const QString &line : tooltipLines) {
        QRectF rect = fm.boundingRect(line);
        lineRects.append(rect);
        maxWidth = qMax(maxWidth, (int)rect.width());
        totalHeight += rect.height() + 2;
    }

    // 添加内边距
    int padding = 8;
    QRectF tooltipRect(0, 0, maxWidth + padding * 2, totalHeight + padding * 2);
    
    // 计算提示框位置
    QPointF tooltipPos = screenPos + QPointF(m_iconSize + 10, -tooltipRect.height() / 2);
    tooltipRect.moveTo(tooltipPos);

    // 绘制提示框背景
    painter->setBrush(QColor(255, 255, 220, 230));
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRoundedRect(tooltipRect, 5, 5);

    // 绘制文本
    painter->setPen(Qt::black);
    qreal yPos = tooltipRect.top() + padding;
    for (int i = 0; i < tooltipLines.size(); ++i) {
        QRectF textRect(tooltipRect.left() + padding, yPos, 
                       maxWidth, lineRects[i].height());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, tooltipLines[i]);
        yPos += lineRects[i].height() + 2;
    }

    painter->restore();
}

bool VesselsLayer::isPointNearVessel(const QPointF &point, const QPointF &vesselPos, double hitRadius) const
{
    double dx = point.x() - vesselPos.x();
    double dy = point.y() - vesselPos.y();
    double distance = qSqrt(dx * dx + dy * dy);
    return distance <= hitRadius;
}

bool VesselsLayer::isVesselVisible(const QPointF &screenPos, const QSize &viewport) const
{
    QRectF visibleArea(-100, -100, viewport.width() + 200, viewport.height() + 200);
    return visibleArea.contains(screenPos);
}

bool VesselsLayer::shouldDrawDetails(double zoom) const
{
    // 根据缩放级别决定是否显示细节
    if (m_performanceLevel == 2) return zoom > 10;
    if (m_performanceLevel == 1) return zoom > 8;
    return zoom > 5;
}

void VesselsLayer::cleanupStaleVessels()
{
    if (m_performanceLevel == 0) return;

    QDateTime cleanupTime = QDateTime::currentDateTime().addSecs(-300); // 5分钟前的数据
    
    QMutexLocker locker(&m_vesselsMutex);
    QList<QString> toRemove;
    
    for (auto it = m_vessels.begin(); it != m_vessels.end(); ++it) {
        if (it.value().lastUpdateTime < cleanupTime) {
            toRemove.append(it.key());
        }
    }
    
    for (const QString &vesselId : toRemove) {
        m_vessels.remove(vesselId);
        m_iconCache.remove(vesselId);
    }
    
    if (!toRemove.isEmpty()) {
        m_needsFullRedraw = true;
    }
}

QColor VesselsLayer::getDeviceColor(AISMessageType messageType) const
{
    switch (messageType) {
    case AISMessageType::AID_TO_NAVIGATION_REPORT:
        return QColor(255, 255, 0); // 黄色 - 助航设备
    case AISMessageType::BASE_STATION_REPORT:
        return QColor(128, 128, 128); // 灰色 - 基站
    case AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT:
        return QColor(255, 0, 255); // 紫色 - SAR飞机
    case AISMessageType::BINARY_BROADCAST_MESSAGE:
    case AISMessageType::BINARY_ADDRESSED_MESSAGE:
        return QColor(0, 255, 255); // 青色 - 二进制消息
    default:
        return QColor(0, 120, 255); // 蓝色 - 普通船舶
    }
}

void VesselsLayer::getDeviceShape(AISMessageType messageType, QPolygonF &shape) const
{
    double halfSize = m_iconSize / 2.0;
    shape.clear();

    switch (messageType) {
    case AISMessageType::AID_TO_NAVIGATION_REPORT:
        // 助航设备 - 菱形
        shape << QPointF(0, -halfSize)
              << QPointF(halfSize, 0)
              << QPointF(0, halfSize)
              << QPointF(-halfSize, 0);
        break;
        
    case AISMessageType::BASE_STATION_REPORT:
        // 基站 - 正方形
        shape << QPointF(-halfSize, -halfSize)
              << QPointF(halfSize, -halfSize)
              << QPointF(halfSize, halfSize)
              << QPointF(-halfSize, halfSize);
        break;
        
    case AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT:
        // SAR飞机 - 三角形
        shape << QPointF(0, -halfSize * 1.5)
              << QPointF(halfSize, halfSize * 0.75)
              << QPointF(-halfSize, halfSize * 0.75);
        break;
        
    default:
        // 普通船舶 - 船形
        shape << QPointF(0, -halfSize * 1.5)
              << QPointF(halfSize, halfSize * 0.75)
              << QPointF(-halfSize, halfSize * 0.75);
        break;
    }
}