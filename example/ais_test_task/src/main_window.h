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
#include <QCheckBox>
#include <QSet>
#include <QComboBox>

#include "map/multi_mapview.h"
#include "map/layers/vessels_layer.h"
#include "factory/ais_message_generator.h"
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
    double courseOverGround; // 对地航向（度）
    QGeoCoordinate position; // 位置
    int navigationStatus;
    int rateOfTurn;
    QString destination;
    int etaMonth;
    int etaDay;
    int etaHour;
    int etaMinute;
    
    // 固定消息类型（创建时确定）
    AISMessageType messageType;
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
    int messageCounter;
};

/**
 * @brief 主窗口类
 * 提供AIS数据生成、发送和地图显示功能
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 路线规划相关
    void onRoutePlanned(const QVector<QGeoCoordinate> &route);
    
    // 任务控制相关
    void onStartAisGeneration();
    void onStopAisGeneration();
    void onDeleteTask();
    void onTaskSelectionChanged();
    
    // 配置相关
    void onSendIntervalChanged(int interval);
    void onUdpSettingsChanged();
    
    // 数据生成和发送
    void onGenerateAisData();
    void onSendAisData();
    
    // UI控制
    void onToggleTaskDock();
    void onClearLog();
    
    // 随机生成功能
    void onRandomGenerate();

private:
    /**
     * @brief 初始化UI界面
     */
    void setupUi();
    
    /**
     * @brief 建立信号槽连接
     */
    void setupConnections();
    
    /**
     * @brief 创建AIS任务
     * @param taskName 任务名称
     * @param route 航线路径
     * @param messageType 消息类型
     */
    void createAisTask(const QString &taskName, const QVector<QGeoCoordinate> &route, AISMessageType messageType);
    
    /**
     * @brief 生成随机MMSI号
     * @return MMSI字符串
     */
    QString generateRandomMmsi();
    
    /**
     * @brief 生成随机呼号
     * @return 呼号字符串
     */
    QString generateRandomCallsign();
    
    /**
     * @brief 生成随机船舶信息
     * @param messageType 消息类型
     * @return 船舶信息结构
     */
    AisVesselInfo generateRandomVesselInfo(AISMessageType messageType);
    
    /**
     * @brief 根据船舶信息和消息类型生成AIS消息
     * @param vesselData 船舶数据
     * @param messageType 消息类型
     * @return NMEA格式的AIS消息
     */
    std::string generateAisMessage(const AISVesselData& vesselData, AISMessageType messageType);
    
    /**
     * @brief 记录日志消息
     * @param message 日志内容
     */
    void logMessage(const QString &message);
    
    /**
     * @brief 更新地图上的船舶显示
     * @param task AIS任务
     */
    void updateVesselDisplay(const AisGenerationTask &task);
    
    /**
     * @brief 将AisVesselInfo转换为AISVesselData
     * @param vesselInfo 船舶信息
     * @return AIS船舶数据
     */
    AISVesselData convertToAISVesselData(const AisVesselInfo& vesselInfo);
    
    /**
     * @brief 在地图可见范围内随机生成位置
     * @return 随机坐标
     */
    QGeoCoordinate generateRandomPositionInView();
    
    /**
     * @brief 生成随机航线
     * @param startPos 起始位置
     * @param numPoints 航线点数量
     * @return 航线路径
     */
    QVector<QGeoCoordinate> generateRandomRoute(const QGeoCoordinate& startPos, int numPoints);
    
    /**
     * @brief 获取消息类型名称
     * @param type 消息类型
     * @return 类型名称
     */
    QString getMessageTypeName(AISMessageType type) const;

    // UI组件
    SsMultiMapView *m_mapView;
    QDockWidget *m_taskDock;
    QDockWidget *m_logDock;
    QListWidget *m_taskList;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_deleteButton;
    QPushButton *m_randomGenerateButton;
    QSpinBox *m_intervalSpinBox;
    QLabel *m_statusLabel;
    QTextEdit *m_logTextEdit;
    QLineEdit *m_udpHostEdit;
    QSpinBox *m_udpPortSpinBox;
    QPushButton *m_clearLogButton;
    QPushButton *m_toggleTaskDockButton;
    
    // 消息类型选择
    QComboBox *m_messageTypeComboBox;

    // 数据管理
    QList<AisGenerationTask> m_aisTasks;
    QTimer *m_generationTimer;
    QTimer *m_sendingTimer;
    
    // 网络设置
    QString m_udpHost;
    int m_udpPort;
    int m_sendInterval;
    
    // AIS消息生成器
    AISMessageGenerator m_aisGenerator;
    
    // 船舶图层
    VesselsLayer *m_vesselsLayer;
};

#endif // MAIN_WINDOW_H