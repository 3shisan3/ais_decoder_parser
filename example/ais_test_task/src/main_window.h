#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QUdpSocket>
#include <QGeoCoordinate>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QDockWidget>
#include <QToolBar>
#include <QUuid>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>

#include "map/multi_mapview.h"
#include "map/layers/vessels_layer.h"  // 新增：船舶图层
#include "udp-tcp-communicate/communicate_api.h"

// AIS船舶信息结构
struct AisVesselInfo {
    QString vesselId;
    QString vesselName;
    QColor vesselColor;
    int mmsi;
    int imo;
    QString callSign;
    int shipType;
    int length;
    int width;
    double draft;
    double speed; // 节
    double heading; // 度
    QGeoCoordinate position; // 位置
};

// AIS任务结构
struct AisGenerationTask {
    QString taskId;
    QString taskName;
    AisVesselInfo vesselInfo;
    QVector<QGeoCoordinate> route;
    int updateInterval; // 毫秒
    bool isActive;
    QDateTime startTime;
    int currentPointIndex;
    double progressAlongSegment;
};

// 主窗口类
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRoutePlanned(const QVector<QGeoCoordinate> &route);
    void onStartAisGeneration();
    void onStopAisGeneration();
    void onDeleteTask();
    void onTaskSelectionChanged();
    void onSendIntervalChanged(int interval);
    void onGenerateAisData();
    void onSendAisData();
    void onUdpSettingsChanged();
    void onToggleTaskDock();
    void onClearLog();

private:
    void setupUi();
    void setupConnections();
    void createAisTask(const QString &taskName, const QVector<QGeoCoordinate> &route);
    QString generateRandomMmsi();
    QString generateRandomCallsign();
    AisVesselInfo generateRandomVesselInfo();
    std::string generateAisMessage(const AisVesselInfo &vessel, const QGeoCoordinate &position);
    void logMessage(const QString &message);
    
    // 新增：更新地图上的船舶显示
    void updateVesselDisplay(const AisGenerationTask &task);

    // UI组件
    SsMultiMapView *m_mapView;
    QDockWidget *m_taskDock;
    QDockWidget *m_logDock;
    QListWidget *m_taskList;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_deleteButton;
    QSpinBox *m_intervalSpinBox;
    QLabel *m_statusLabel;
    QTextEdit *m_logTextEdit;
    QLineEdit *m_udpHostEdit;
    QSpinBox *m_udpPortSpinBox;
    QPushButton *m_clearLogButton;
    QPushButton *m_toggleTaskDockButton;

    // 数据管理
    QList<AisGenerationTask> m_aisTasks;
    QTimer *m_generationTimer;
    QTimer *m_sendingTimer;
    
    // 网络设置
    QString m_udpHost;
    int m_udpPort;
    int m_sendInterval;
    
    // 新增：船舶图层
    VesselsLayer *m_vesselsLayer;
};

#endif // MAIN_WINDOW_H