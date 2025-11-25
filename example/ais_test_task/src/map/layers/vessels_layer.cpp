#include "vessels_layer.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

VesselsLayer::VesselsLayer(QObject *parent)
    : BaseLayer(parent)
{
}

void VesselsLayer::updateVessel(const QString &vesselId, const VesselDisplayInfo &info)
{
    m_vessels[vesselId] = info;
    emit updateRequested();  // 请求重绘
}

void VesselsLayer::removeVessel(const QString &vesselId)
{
    if (m_vessels.remove(vesselId) > 0) {
        emit updateRequested();
    }
}

void VesselsLayer::clearVessels()
{
    if (!m_vessels.isEmpty()) {
        m_vessels.clear();
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
    if (!isVisible()) {
        return nullptr;
    }

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

void VesselsLayer::render(QPainter *painter,
                          const QSize &viewport,
                          const QGeoCoordinate &center,
                          double zoom,
                          const TileForCoord::TileAlgorithm &algorithm)
{
    if (m_vessels.isEmpty() || !isVisible()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 计算视口中心偏移
    QPointF centerPixel = algorithm.latLongToPixelXY(
        center.longitude(), center.latitude(), qFloor(zoom));
    QPointF viewportCenter(viewport.width() / 2.0, viewport.height() / 2.0);
    QPointF offset = viewportCenter - centerPixel;

    // 绘制所有船舶
    for (auto it = m_vessels.constBegin(); it != m_vessels.constEnd(); ++it) {
        const VesselDisplayInfo &info = it.value();
        
        if (!info.position.isValid()) {
            continue;
        }

        // 计算船舶在屏幕上的位置
        QPointF vesselPixel = algorithm.latLongToPixelXY(
            info.position.longitude(), info.position.latitude(), qFloor(zoom));
        QPointF vesselScreenPos = vesselPixel + offset;

        // 检查是否在可见范围内（加上边界容差）
        QRectF visibleRect(-100, -100, viewport.width() + 200, viewport.height() + 200);
        if (!visibleRect.contains(vesselScreenPos)) {
            continue;
        }

        // 绘制船舶
        drawVessel(painter, vesselScreenPos, info);
    }

    painter->restore();
    emit renderingComplete();
}

void VesselsLayer::drawVessel(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info)
{
    painter->save();

    // 移动到船舶位置
    painter->translate(screenPos);
    
    // 旋转到航向角度
    painter->rotate(info.heading);

    // 绘制船舶三角形图标
    QPolygonF vesselShape;
    double halfSize = m_iconSize / 2.0;
    vesselShape << QPointF(0, -halfSize * 1.5)           // 船头
                << QPointF(halfSize, halfSize * 0.75)    // 右侧
                << QPointF(-halfSize, halfSize * 0.75);  // 左侧

    // 填充颜色
    painter->setBrush(info.color);
    painter->setPen(QPen(Qt::white, 2));
    painter->drawPolygon(vesselShape);

    // 绘制外边框（增强可见性）
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(vesselShape);

    painter->restore();

    // 绘制船舶名称（如果启用）
    if (m_showNames && !info.vesselName.isEmpty()) {
        painter->save();
        
        QFont font = painter->font();
        font.setPointSize(9);
        font.setBold(true);
        painter->setFont(font);

        // 计算文本位置（在船舶图标下方）
        QPointF textPos = screenPos + QPointF(0, m_iconSize);
        
        // 绘制文本背景（提高可读性）
        QString displayText = info.vesselName;
        QFontMetrics fm(font);
        QRectF textRect = fm.boundingRect(displayText);
        textRect.moveCenter(QPointF(textPos.x(), textPos.y() + textRect.height() / 2));
        textRect.adjust(-3, -1, 3, 1);
        
        painter->setBrush(QColor(255, 255, 255, 200));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(textRect, 3, 3);

        // 绘制文本
        painter->setPen(Qt::black);
        painter->drawText(textRect, Qt::AlignCenter, displayText);

        painter->restore();
    }

    // 绘制速度指示线（可选）
    if (info.speed > 0.5) {  // 速度大于0.5节时显示
        painter->save();
        painter->translate(screenPos);
        painter->rotate(info.heading);
        
        // 根据速度调整线长
        double lineLength = m_iconSize * (1.0 + info.speed / 10.0);
        lineLength = qMin(lineLength, m_iconSize * 3.0);  // 最大3倍图标大小
        
        QPen speedLinePen(info.color, 2, Qt::DashLine);
        painter->setPen(speedLinePen);
        painter->drawLine(QPointF(0, -m_iconSize * 0.75), 
                         QPointF(0, -lineLength));
        
        painter->restore();
    }
}

void VesselsLayer::drawTooltip(QPainter *painter, const QPointF &screenPos, const VesselDisplayInfo &info)
{
    painter->save();

    // 构建提示文本
    QStringList tooltipLines;
    tooltipLines << QString("MMSI: %1").arg(info.mmsi);
    tooltipLines << QString("Name: %1").arg(info.vesselName);
    tooltipLines << QString("Position: %1, %2")
                    .arg(info.position.latitude(), 0, 'f', 6)
                    .arg(info.position.longitude(), 0, 'f', 6);
    tooltipLines << QString("Heading: %1°").arg(info.heading, 0, 'f', 1);
    tooltipLines << QString("Speed: %1 kts").arg(info.speed, 0, 'f', 1);

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
        totalHeight += rect.height() + 2;  // 行间距2像素
    }

    // 添加内边距
    int padding = 8;
    QRectF tooltipRect(0, 0, maxWidth + padding * 2, totalHeight + padding * 2);
    
    // 计算提示框位置（在船舶右侧）
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