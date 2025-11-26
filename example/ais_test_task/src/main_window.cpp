#include "main_window.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDateTime>
#include <QTime>
#include <QRandomGenerator>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QComboBox>
#include <QToolBar>
#include <QTextEdit>
#include <QMenuBar>
#include <QStatusBar>
#include <QCheckBox>
#include <cmath>
#include <algorithm>

/**
 * @brief 主窗口构造函数
 * 初始化所有组件和连接
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_udpHost("127.0.0.1")
    , m_udpPort(10110)
    , m_sendInterval(1000)
{
    // 初始化communicate API
    int ret = communicate::Initialize();
    if (ret != 0) {
        QMessageBox::warning(this, "警告", "通信模块初始化失败");
    }
    
    setupUi();
    setupConnections();
    
    // 初始化船舶图层
    m_vesselsLayer = new VesselsLayer(this);
    m_mapView->addLayer(m_vesselsLayer);
    m_vesselsLayer->setVisible(true);
    m_vesselsLayer->setShowVesselNames(true);
    m_vesselsLayer->setVesselIconSize(20);
    m_vesselsLayer->setPerformanceLevel(1); // 启用基础优化
    m_vesselsLayer->setMaxVessels(150); // 限制最大显示数量
    
    // 优化定时器设置 - 降低更新频率减少卡顿
    m_generationTimer = new QTimer(this);
    m_generationTimer->setInterval(1000); // 改为1000ms，降低更新频率
    connect(m_generationTimer, &QTimer::timeout, this, &MainWindow::onGenerateAisData);
    
    m_sendingTimer = new QTimer(this);
    m_sendingTimer->setInterval(m_sendInterval);
    connect(m_sendingTimer, &QTimer::timeout, this, &MainWindow::onSendAisData);
    
    m_generationTimer->start();
    m_sendingTimer->start();
    
    logMessage("AIS数据生成与发送系统已启动 - 优化版本");
    logMessage(QString("UDP发送目标: %1:%2").arg(m_udpHost).arg(m_udpPort));
    logMessage("系统更新频率: 1000ms（优化了性能减少卡顿）");
    logMessage("消息类型: 创建时固定，发送时不再变动");
}

MainWindow::~MainWindow()
{
    m_generationTimer->stop();
    m_sendingTimer->stop();
    communicate::Destroy();
}

/**
 * @brief 设置UI界面
 * 创建所有界面组件和布局
 */
void MainWindow::setupUi()
{
    // 设置主窗口
    setWindowTitle("AIS数据生成与发送系统");
    setGeometry(100, 100, 1600, 900);
    
    // 创建地图视图
    m_mapView = new SsMultiMapView(this);
    setCentralWidget(m_mapView);
    m_mapView->setTileUrlTemplate("https://wprd01.is.autonavi.com/appmaptile?&style=6&lang=zh_cn&scl=1&ltype=0&x={x}&y={y}&z={z}");
    m_mapView->setCenter(QGeoCoordinate(31.2304, 121.4737)); // 上海
    m_mapView->setZoomLevel(13);
    
    // 创建任务停靠窗口
    m_taskDock = new QDockWidget("AIS任务管理", this);
    m_taskDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_taskDock);
    
    // 创建任务列表控件
    QWidget *taskWidget = new QWidget();
    QVBoxLayout *taskLayout = new QVBoxLayout();
    
    // UDP设置组
    QGroupBox *udpGroup = new QGroupBox("UDP设置");
    QFormLayout *udpLayout = new QFormLayout();
    
    m_udpHostEdit = new QLineEdit(m_udpHost);
    m_udpPortSpinBox = new QSpinBox();
    m_udpPortSpinBox->setRange(1, 65535);
    m_udpPortSpinBox->setValue(m_udpPort);
    
    udpLayout->addRow("目标IP:", m_udpHostEdit);
    udpLayout->addRow("目标端口:", m_udpPortSpinBox);
    udpGroup->setLayout(udpLayout);
    taskLayout->addWidget(udpGroup);
    
    // 消息类型选择组
    QGroupBox *messageTypeGroup = new QGroupBox("消息类型选择");
    QFormLayout *messageTypeLayout = new QFormLayout();
    
    m_messageTypeComboBox = new QComboBox();
    // 添加所有27种AIS消息类型
    m_messageTypeComboBox->addItem("Type 1 - A类位置报告", static_cast<int>(AISMessageType::POSITION_REPORT_CLASS_A));
    m_messageTypeComboBox->addItem("Type 2 - A类分配时隙位置报告", static_cast<int>(AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED));
    m_messageTypeComboBox->addItem("Type 3 - A类轮询响应位置报告", static_cast<int>(AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE));
    m_messageTypeComboBox->addItem("Type 4 - 基站报告", static_cast<int>(AISMessageType::BASE_STATION_REPORT));
    m_messageTypeComboBox->addItem("Type 5 - 静态和航程数据", static_cast<int>(AISMessageType::STATIC_VOYAGE_DATA));
    m_messageTypeComboBox->addItem("Type 6 - 寻址二进制消息", static_cast<int>(AISMessageType::BINARY_ADDRESSED_MESSAGE));
    m_messageTypeComboBox->addItem("Type 7 - 二进制确认", static_cast<int>(AISMessageType::BINARY_ACKNOWLEDGE));
    m_messageTypeComboBox->addItem("Type 8 - 广播二进制消息", static_cast<int>(AISMessageType::BINARY_BROADCAST_MESSAGE));
    m_messageTypeComboBox->addItem("Type 9 - SAR飞机位置报告", static_cast<int>(AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT));
    m_messageTypeComboBox->addItem("Type 10 - UTC/日期查询", static_cast<int>(AISMessageType::UTC_DATE_INQUIRY));
    m_messageTypeComboBox->addItem("Type 11 - UTC/日期响应", static_cast<int>(AISMessageType::UTC_DATE_RESPONSE));
    m_messageTypeComboBox->addItem("Type 12 - 寻址安全消息", static_cast<int>(AISMessageType::ADDRESSED_SAFETY_MESSAGE));
    m_messageTypeComboBox->addItem("Type 13 - 安全确认", static_cast<int>(AISMessageType::SAFETY_ACKNOWLEDGE));
    m_messageTypeComboBox->addItem("Type 14 - 广播安全消息", static_cast<int>(AISMessageType::SAFETY_RELATED_BROADCAST));
    m_messageTypeComboBox->addItem("Type 15 - 询问", static_cast<int>(AISMessageType::INTERROGATION));
    m_messageTypeComboBox->addItem("Type 16 - 分配模式命令", static_cast<int>(AISMessageType::ASSIGNMENT_MODE_COMMAND));
    m_messageTypeComboBox->addItem("Type 17 - DGNSS二进制广播", static_cast<int>(AISMessageType::DGNSS_BINARY_BROADCAST));
    m_messageTypeComboBox->addItem("Type 18 - B类位置报告", static_cast<int>(AISMessageType::STANDARD_CLASS_B_CS_POSITION));
    m_messageTypeComboBox->addItem("Type 19 - 扩展B类位置报告", static_cast<int>(AISMessageType::EXTENDED_CLASS_B_CS_POSITION));
    m_messageTypeComboBox->addItem("Type 20 - 数据链路管理", static_cast<int>(AISMessageType::DATA_LINK_MANAGEMENT));
    m_messageTypeComboBox->addItem("Type 21 - 助航设备报告", static_cast<int>(AISMessageType::AID_TO_NAVIGATION_REPORT));
    m_messageTypeComboBox->addItem("Type 22 - 信道管理", static_cast<int>(AISMessageType::CHANNEL_MANAGEMENT));
    m_messageTypeComboBox->addItem("Type 23 - 组分配命令", static_cast<int>(AISMessageType::GROUP_ASSIGNMENT_COMMAND));
    m_messageTypeComboBox->addItem("Type 24 - 静态数据报告", static_cast<int>(AISMessageType::STATIC_DATA_REPORT));
    m_messageTypeComboBox->addItem("Type 25 - 单时隙二进制消息", static_cast<int>(AISMessageType::SINGLE_SLOT_BINARY_MESSAGE));
    m_messageTypeComboBox->addItem("Type 26 - 多时隙二进制消息", static_cast<int>(AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE));
    m_messageTypeComboBox->addItem("Type 27 - 长距离位置报告", static_cast<int>(AISMessageType::POSITION_REPORT_LONG_RANGE));
    
    messageTypeLayout->addRow("消息类型:", m_messageTypeComboBox);
    
    QLabel *noteLabel = new QLabel("注意：船舶创建时确定消息类型，发送时保持不变");
    noteLabel->setStyleSheet("color: gray; font-style: italic;");
    noteLabel->setWordWrap(true);
    messageTypeLayout->addRow(noteLabel);
    
    messageTypeGroup->setLayout(messageTypeLayout);
    taskLayout->addWidget(messageTypeGroup);
    
    // 任务列表
    QGroupBox *taskGroup = new QGroupBox("任务列表");
    QVBoxLayout *taskGroupLayout = new QVBoxLayout();
    m_taskList = new QListWidget();
    taskGroupLayout->addWidget(m_taskList);
    taskGroup->setLayout(taskGroupLayout);
    taskLayout->addWidget(taskGroup);
    
    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startButton = new QPushButton("启动");
    m_stopButton = new QPushButton("停止");
    m_deleteButton = new QPushButton("删除");
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_deleteButton);
    taskLayout->addLayout(buttonLayout);
    
    // 随机生成按钮
    m_randomGenerateButton = new QPushButton("随机生成船舶");
    m_randomGenerateButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
    m_randomGenerateButton->setToolTip("在当前地图范围内随机生成指定数量的船舶");
    taskLayout->addWidget(m_randomGenerateButton);
    
    // 发送间隔设置
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    intervalLayout->addWidget(new QLabel("发送间隔(ms):"));
    m_intervalSpinBox = new QSpinBox();
    m_intervalSpinBox->setRange(100, 10000);
    m_intervalSpinBox->setValue(m_sendInterval);
    m_intervalSpinBox->setSingleStep(100);
    intervalLayout->addWidget(m_intervalSpinBox);
    taskLayout->addLayout(intervalLayout);
    
    // 状态显示
    m_statusLabel = new QLabel("就绪");
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #e3f2fd; border-radius: 3px; }");
    taskLayout->addWidget(m_statusLabel);
    
    taskLayout->addStretch();
    taskWidget->setLayout(taskLayout);
    m_taskDock->setWidget(taskWidget);
    
    // 创建日志停靠窗口
    m_logDock = new QDockWidget("发送日志", this);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
    
    QWidget *logWidget = new QWidget();
    QVBoxLayout *logLayout = new QVBoxLayout();
    
    QHBoxLayout *logHeaderLayout = new QHBoxLayout();
    logHeaderLayout->addWidget(new QLabel("发送数据内容:"));
    m_clearLogButton = new QPushButton("清空日志");
    logHeaderLayout->addStretch();
    logHeaderLayout->addWidget(m_clearLogButton);
    logLayout->addLayout(logHeaderLayout);
    
    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(200);
    logLayout->addWidget(m_logTextEdit);
    
    logWidget->setLayout(logLayout);
    m_logDock->setWidget(logWidget);
    
    // 添加菜单栏
    QMenuBar *menuBar = this->menuBar();
    QMenu *viewMenu = menuBar->addMenu("视图");
    
    QAction *toggleTaskDockAction = viewMenu->addAction("显示/隐藏任务面板");
    QAction *toggleLogDockAction = viewMenu->addAction("显示/隐藏日志面板");
    
    connect(toggleTaskDockAction, &QAction::triggered, [this]() {
        m_taskDock->setVisible(!m_taskDock->isVisible());
    });
    
    connect(toggleLogDockAction, &QAction::triggered, [this]() {
        m_logDock->setVisible(!m_logDock->isVisible());
    });
    
    // 添加工具栏
    QToolBar *toolBar = addToolBar("操作");
    m_toggleTaskDockButton = new QPushButton("任务面板");
    toolBar->addWidget(m_toggleTaskDockButton);
    
    QAction *routeAction = toolBar->addAction("规划路线");
    connect(routeAction, &QAction::triggered, [this]() {
        m_mapView->startRoutePlanning();
    });
    
    // 添加状态栏
    statusBar()->showMessage("系统就绪");
}

/**
 * @brief 建立信号槽连接
 */
void MainWindow::setupConnections()
{
    // 地图信号
    connect(m_mapView, &SsMultiMapView::routePlanningFinished, 
            this, &MainWindow::onRoutePlanned);
    
    // 按钮信号
    connect(m_startButton, &QPushButton::clicked, 
            this, &MainWindow::onStartAisGeneration);
    connect(m_stopButton, &QPushButton::clicked, 
            this, &MainWindow::onStopAisGeneration);
    connect(m_deleteButton, &QPushButton::clicked, 
            this, &MainWindow::onDeleteTask);
    connect(m_clearLogButton, &QPushButton::clicked,
            this, &MainWindow::onClearLog);
    connect(m_toggleTaskDockButton, &QPushButton::clicked,
            this, &MainWindow::onToggleTaskDock);
    connect(m_randomGenerateButton, &QPushButton::clicked,
            this, &MainWindow::onRandomGenerate);
    
    // 列表选择变化
    connect(m_taskList, &QListWidget::itemSelectionChanged,
            this, &MainWindow::onTaskSelectionChanged);
    
    // 间隔设置变化
    connect(m_intervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSendIntervalChanged);
    
    // UDP设置变化
    connect(m_udpHostEdit, &QLineEdit::textChanged,
            this, &MainWindow::onUdpSettingsChanged);
    connect(m_udpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onUdpSettingsChanged);
}

/**
 * @brief 路线规划完成处理
 * @param route 规划的路线
 */
void MainWindow::onRoutePlanned(const QVector<QGeoCoordinate> &route)
{
    if (route.size() < 2) {
        QMessageBox::warning(this, "警告", "路线至少需要两个点");
        return;
    }
    
    // 弹出对话框获取任务名称和消息类型
    QDialog dialog(this);
    dialog.setWindowTitle("创建AIS任务");
    dialog.setFixedWidth(400);
    
    QFormLayout form(&dialog);
    QLineEdit taskNameEdit;
    form.addRow("任务名称:", &taskNameEdit);
    
    QComboBox messageTypeComboBox;
    // 添加常用消息类型
    messageTypeComboBox.addItem("Type 1 - A类位置报告", static_cast<int>(ais::AISMessageType::POSITION_REPORT_CLASS_A));
    messageTypeComboBox.addItem("Type 4 - 基站报告", static_cast<int>(ais::AISMessageType::BASE_STATION_REPORT));
    messageTypeComboBox.addItem("Type 5 - 静态和航程数据", static_cast<int>(ais::AISMessageType::STATIC_VOYAGE_DATA));
    messageTypeComboBox.addItem("Type 18 - B类位置报告", static_cast<int>(ais::AISMessageType::STANDARD_CLASS_B_CS_POSITION));
    messageTypeComboBox.addItem("Type 21 - 助航设备报告", static_cast<int>(ais::AISMessageType::AID_TO_NAVIGATION_REPORT));
    messageTypeComboBox.addItem("Type 24 - 静态数据报告", static_cast<int>(ais::AISMessageType::STATIC_DATA_REPORT));
    form.addRow("消息类型:", &messageTypeComboBox);
    
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString taskName = taskNameEdit.text().trimmed();
        if (taskName.isEmpty()) {
            taskName = QString("任务_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        }
        
        AISMessageType messageType = static_cast<AISMessageType>(messageTypeComboBox.currentData().toInt());
        createAisTask(taskName, route, messageType);
    }
}

/**
 * @brief 创建AIS任务
 * @param taskName 任务名称
 * @param route 航线路径
 * @param messageType 消息类型
 */
void MainWindow::createAisTask(const QString &taskName, const QVector<QGeoCoordinate> &route, AISMessageType messageType)
{
    AisGenerationTask task;
    task.taskId = QUuid::createUuid().toString();
    task.taskName = taskName;
    task.vesselInfo = generateRandomVesselInfo(messageType);
    task.route = route;
    task.updateInterval = 1000; // 与定时器同步
    task.isActive = false;
    task.startTime = QDateTime::currentDateTime();
    task.currentPointIndex = 0;
    task.progressAlongSegment = 0.0;
    task.messageCounter = 0;
    
    // 设置初始位置为航线起点
    task.vesselInfo.position = route.first();
    
    m_aisTasks.append(task);
    
    // 更新列表
    QString listText = QString("%1 [%2]").arg(taskName).arg(getMessageTypeName(messageType));
    QListWidgetItem *item = new QListWidgetItem(listText);
    item->setData(Qt::UserRole, task.taskId);
    m_taskList->addItem(item);
    
    m_statusLabel->setText(QString("已创建任务: %1").arg(taskName));
    logMessage(QString("创建任务: %1 (MMSI: %2, 消息类型: %3)")
        .arg(taskName)
        .arg(task.vesselInfo.mmsi)
        .arg(getMessageTypeName(messageType)));
    
    // 立即更新显示
    updateVesselDisplay(task);
}

/**
 * @brief 生成随机船舶信息
 * @param messageType 消息类型
 * @return 船舶信息结构
 */
AisVesselInfo MainWindow::generateRandomVesselInfo(AISMessageType messageType)
{
    AisVesselInfo vessel;
    
    static const QStringList vesselNames = {
        "Ocean Star", "Sea Explorer", "Marine Voyager", "Blue Whale",
        "Pacific King", "Atlantic Queen", "Cargo Master", "Container Express",
        "Harmony", "Discovery", "Navigator", "Endeavour",
        "Navigation Buoy A", "Channel Marker B", "Light House C" // 助航设备名称
    };
    
    static const QList<QColor> vesselColors = {
        Qt::blue, Qt::red, Qt::green, Qt::darkYellow,
        Qt::darkBlue, Qt::darkRed, Qt::darkGreen, Qt::magenta,
        QColor(255, 140, 0), QColor(75, 0, 130)
    };
    
    static const QStringList destinations = {
        "SHANGHAI", "NINGBO", "QINGDAO", "TIANJIN",
        "HONG KONG", "SINGAPORE", "TOKYO", "BUSAN"
    };
    
    vessel.vesselName = vesselNames[QRandomGenerator::global()->bounded(vesselNames.size())];
    vessel.vesselColor = vesselColors[QRandomGenerator::global()->bounded(vesselColors.size())];
    vessel.vesselId = QString("VESSEL_%1").arg(QRandomGenerator::global()->bounded(10000, 99999));
    
    vessel.mmsi = generateRandomMmsi().toInt();
    vessel.imo = QRandomGenerator::global()->bounded(9000000, 9999999);
    vessel.callSign = generateRandomCallsign();
    
    // 根据消息类型设置不同的船舶类型
    switch (messageType) {
        case AISMessageType::AID_TO_NAVIGATION_REPORT:
            vessel.shipType = 1; // 助航设备
            vessel.speed = 0; // 固定设备
            break;
        case AISMessageType::BASE_STATION_REPORT:
            vessel.shipType = 0; // 基站
            vessel.speed = 0;
            break;
        default:
            vessel.shipType = QRandomGenerator::global()->bounded(60, 90); // 货船类型
            vessel.speed = QRandomGenerator::global()->bounded(5, 25);
            break;
    }
    
    vessel.length = QRandomGenerator::global()->bounded(50, 300);
    vessel.width = QRandomGenerator::global()->bounded(10, 50);
    vessel.draft = QRandomGenerator::global()->bounded(5, 20) / 10.0;
    vessel.heading = QRandomGenerator::global()->bounded(0, 360);
    vessel.courseOverGround = vessel.heading;
    vessel.navigationStatus = 0; // 在航
    vessel.rateOfTurn = 0;
    vessel.destination = destinations[QRandomGenerator::global()->bounded(destinations.size())];
    vessel.messageType = messageType; // 固定消息类型
    
    // 生成随机ETA
    QDateTime eta = QDateTime::currentDateTime().addDays(QRandomGenerator::global()->bounded(1, 7));
    vessel.etaMonth = eta.date().month();
    vessel.etaDay = eta.date().day();
    vessel.etaHour = eta.time().hour();
    vessel.etaMinute = eta.time().minute();
    
    return vessel;
}

/**
 * @brief 生成随机MMSI号
 * @return MMSI字符串
 */
QString MainWindow::generateRandomMmsi()
{
    // MMSI: 9位数字，前3位是MID（海事识别数字）
    static const QStringList midCodes = {"412", "413", "414", "440", "441"}; // 中国MID
    QString mid = midCodes[QRandomGenerator::global()->bounded(midCodes.size())];
    QString number = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
    return mid + number;
}

/**
 * @brief 生成随机呼号
 * @return 呼号字符串
 */
QString MainWindow::generateRandomCallsign()
{
    static const QString letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString callsign;
    
    // 前2-3个字母（国家代码）
    for (int i = 0; i < 2; i++) {
        callsign.append(letters[QRandomGenerator::global()->bounded(letters.size())]);
    }
    
    // 后4个字符（字母或数字）
    for (int i = 0; i < 4; i++) {
        if (QRandomGenerator::global()->bounded(2) == 0) {
            callsign.append(letters[QRandomGenerator::global()->bounded(letters.size())]);
        } else {
            callsign.append(QString::number(QRandomGenerator::global()->bounded(0, 10)));
        }
    }
    
    return callsign;
}

/**
 * @brief 启动AIS数据生成
 */
void MainWindow::onStartAisGeneration()
{
    QList<QListWidgetItem*> selectedItems = m_taskList->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一个任务");
        return;
    }
    
    QString taskId = selectedItems.first()->data(Qt::UserRole).toString();
    for (AisGenerationTask &task : m_aisTasks) {
        if (task.taskId == taskId) {
            task.isActive = true;
            task.startTime = QDateTime::currentDateTime();
            task.messageCounter = 0;
            m_statusLabel->setText(QString("已启动任务: %1").arg(task.taskName));
            logMessage(QString("启动任务: %1").arg(task.taskName));
            statusBar()->showMessage(QString("任务 %1 已启动").arg(task.taskName), 3000);
            break;
        }
    }
}

/**
 * @brief 停止AIS数据生成
 */
void MainWindow::onStopAisGeneration()
{
    QList<QListWidgetItem*> selectedItems = m_taskList->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    
    QString taskId = selectedItems.first()->data(Qt::UserRole).toString();
    for (AisGenerationTask &task : m_aisTasks) {
        if (task.taskId == taskId) {
            task.isActive = false;
            m_statusLabel->setText(QString("已停止任务: %1").arg(task.taskName));
            logMessage(QString("停止任务: %1").arg(task.taskName));
            statusBar()->showMessage(QString("任务 %1 已停止").arg(task.taskName), 3000);
            break;
        }
    }
}

/**
 * @brief 删除AIS任务
 */
void MainWindow::onDeleteTask()
{
    QList<QListWidgetItem*> selectedItems = m_taskList->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    
    QString taskId = selectedItems.first()->data(Qt::UserRole).toString();
    for (int i = 0; i < m_aisTasks.size(); ++i) {
        if (m_aisTasks[i].taskId == taskId) {
            QString taskName = m_aisTasks[i].taskName;
            QString vesselId = m_aisTasks[i].vesselInfo.vesselId;
            
            // 从地图移除船舶
            m_vesselsLayer->removeVessel(vesselId);
            
            m_aisTasks.removeAt(i);
            delete m_taskList->takeItem(m_taskList->row(selectedItems.first()));
            m_statusLabel->setText("已删除任务");
            logMessage(QString("删除任务: %1").arg(taskName));
            statusBar()->showMessage(QString("任务 %1 已删除").arg(taskName), 3000);
            break;
        }
    }
}

/**
 * @brief 任务选择变化处理
 */
void MainWindow::onTaskSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = m_taskList->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();
    
    m_startButton->setEnabled(hasSelection);
    m_stopButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

/**
 * @brief 发送间隔变化处理
 * @param interval 新的间隔值（毫秒）
 */
void MainWindow::onSendIntervalChanged(int interval)
{
    m_sendInterval = interval;
    m_sendingTimer->setInterval(m_sendInterval);
    logMessage(QString("发送间隔调整为: %1ms").arg(m_sendInterval));
}

/**
 * @brief UDP设置变化处理
 */
void MainWindow::onUdpSettingsChanged()
{
    m_udpHost = m_udpHostEdit->text().trimmed();
    m_udpPort = m_udpPortSpinBox->value();
    logMessage(QString("UDP目标更新为: %1:%2").arg(m_udpHost).arg(m_udpPort));
}

/**
 * @brief 切换任务面板显示
 */
void MainWindow::onToggleTaskDock()
{
    m_taskDock->setVisible(!m_taskDock->isVisible());
}

/**
 * @brief 清空日志
 */
void MainWindow::onClearLog()
{
    m_logTextEdit->clear();
    logMessage("日志已清空");
}

/**
 * @brief 生成AIS数据（定时调用）
 * 计算船舶位置并更新地图显示
 */
void MainWindow::onGenerateAisData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    
    for (AisGenerationTask &task : m_aisTasks) {
        if (!task.isActive || task.route.size() < 2) {
            continue;
        }
        
        // 优化：跳过不需要移动的设备
        if (task.vesselInfo.messageType == AISMessageType::AID_TO_NAVIGATION_REPORT ||
            task.vesselInfo.messageType == AISMessageType::BASE_STATION_REPORT) {
            // 助航设备和基站保持固定位置
            continue;
        }
        
        // 计算船舶位置
        double timeElapsed = task.startTime.msecsTo(currentTime) / 1000.0; // 秒
        double totalDistance = 0.0;
        
        // 计算总距离
        for (int i = 1; i < task.route.size(); ++i) {
            totalDistance += task.route[i-1].distanceTo(task.route[i]);
        }
        
        if (totalDistance <= 0) continue;
        
        // 计算当前应完成的比例（循环航行）
        double distanceTraveled = fmod(timeElapsed * task.vesselInfo.speed * 0.5144, totalDistance);
        double completion = distanceTraveled / totalDistance;
        
        // 找到当前所在的线段
        double accumulatedDistance = 0.0;
        for (int i = 1; i < task.route.size(); ++i) {
            double segmentDistance = task.route[i-1].distanceTo(task.route[i]);
            double segmentStart = accumulatedDistance / totalDistance;
            double segmentEnd = (accumulatedDistance + segmentDistance) / totalDistance;
            
            if (completion >= segmentStart && completion <= segmentEnd) {
                task.currentPointIndex = i - 1;
                task.progressAlongSegment = (completion - segmentStart) / (segmentEnd - segmentStart);
                break;
            }
            accumulatedDistance += segmentDistance;
        }
        
        // 计算当前位置
        if (task.currentPointIndex < task.route.size() - 1) {
            QGeoCoordinate start = task.route[task.currentPointIndex];
            QGeoCoordinate end = task.route[task.currentPointIndex + 1];
            
            double lat = start.latitude() + (end.latitude() - start.latitude()) * task.progressAlongSegment;
            double lon = start.longitude() + (end.longitude() - start.longitude()) * task.progressAlongSegment;
            
            task.vesselInfo.position = QGeoCoordinate(lat, lon);
            
            // 计算航向
            double dx = end.longitude() - start.longitude();
            double dy = end.latitude() - start.latitude();
            task.vesselInfo.heading = atan2(dx, dy) * 180.0 / M_PI;
            if (task.vesselInfo.heading < 0) {
                task.vesselInfo.heading += 360.0;
            }
            task.vesselInfo.courseOverGround = task.vesselInfo.heading;
            
            // 立即更新地图显示
            updateVesselDisplay(task);
        }
    }
}

/**
 * @brief 更新地图上的船舶显示
 * @param task AIS任务
 */
void MainWindow::updateVesselDisplay(const AisGenerationTask &task)
{
    if (!task.vesselInfo.position.isValid()) {
        return;
    }
    
    // 构建船舶显示信息
    VesselDisplayInfo displayInfo;
    displayInfo.mmsi = task.vesselInfo.mmsi;
    displayInfo.vesselName = task.vesselInfo.vesselName;
    displayInfo.position = task.vesselInfo.position;
    displayInfo.heading = task.vesselInfo.heading;
    displayInfo.speed = task.vesselInfo.speed;
    displayInfo.color = task.vesselInfo.vesselColor;
    displayInfo.vesselId = task.vesselInfo.vesselId;
    
    // 根据设备类型设置不同的图标颜色
    switch (task.vesselInfo.messageType) {
        case AISMessageType::AID_TO_NAVIGATION_REPORT:
            displayInfo.color = Qt::yellow; // 助航设备用黄色
            break;
        case AISMessageType::BASE_STATION_REPORT:
            displayInfo.color = Qt::gray; // 基站用灰色
            break;
        default:
            // 船舶保持原有颜色
            break;
    }
    
    // 更新船舶图层
    m_vesselsLayer->updateVessel(task.vesselInfo.vesselId, displayInfo);
}

/**
 * @brief 发送AIS数据（定时调用）
 * 根据固定的消息类型发送数据
 */
void MainWindow::onSendAisData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    int sentCount = 0;
    
    for (AisGenerationTask &task : m_aisTasks) {
        if (!task.isActive || !task.vesselInfo.position.isValid()) {
            continue;
        }
        
        // 转换船舶信息
        AISVesselData vesselData = convertToAISVesselData(task.vesselInfo);
        
        // 使用固定的消息类型生成AIS消息
        std::string aisData = generateAisMessage(vesselData, task.vesselInfo.messageType);
        
        if (!aisData.empty()) {
            // 使用communicate API发送数据
            int ret = communicate::SendGeneralMessage(
                m_udpHost.toStdString().c_str(),
                m_udpPort,
                aisData.data(),
                aisData.size()
            );
            
            if (ret == 0) {
                QString logmsg = QString("[%1] 发送AIS数据到 %2:%3\n")
                    .arg(currentTime.toString("hh:mm:ss.zzz"))
                    .arg(m_udpHost)
                    .arg(m_udpPort);
                
                logmsg += QString("  设备: %1 (MMSI: %2)\n")
                    .arg(task.vesselInfo.vesselName)
                    .arg(task.vesselInfo.mmsi);
                
                logmsg += QString("  消息类型: %1\n").arg(getMessageTypeName(task.vesselInfo.messageType));
                logmsg += QString("  位置: 纬度 %1, 经度 %2\n")
                    .arg(task.vesselInfo.position.latitude(), 0, 'f', 6)
                    .arg(task.vesselInfo.position.longitude(), 0, 'f', 6);
                
                if (task.vesselInfo.speed > 0) {
                    logmsg += QString("  航向: %1°, 航速: %2节\n")
                        .arg(task.vesselInfo.heading, 0, 'f', 1)
                        .arg(task.vesselInfo.speed, 0, 'f', 1);
                }
                
                logmsg += QString("  数据: %1").arg(QString::fromStdString(aisData).trimmed());
                
                logMessage(logmsg);
                sentCount++;
            } else {
                logMessage(QString("[%1] 发送失败到 %2:%3, 错误码: %4")
                    .arg(currentTime.toString("hh:mm:ss.zzz"))
                    .arg(m_udpHost)
                    .arg(m_udpPort)
                    .arg(ret));
            }
        }
        
        task.messageCounter++;
    }
    
    if (sentCount > 0) {
        m_statusLabel->setText(QString("已发送 %1 条AIS数据").arg(sentCount));
        statusBar()->showMessage(QString("发送 %1 条消息").arg(sentCount), 2000);
    }
}

/**
 * @brief 转换船舶信息格式
 * @param vesselInfo 船舶信息
 * @return AIS船舶数据
 */
AISVesselData MainWindow::convertToAISVesselData(const AisVesselInfo& vesselInfo)
{
    AISVesselData data;
    
    data.mmsi = vesselInfo.mmsi;
    data.imo = vesselInfo.imo;
    data.callSign = vesselInfo.callSign;
    data.vesselName = vesselInfo.vesselName;
    data.shipType = vesselInfo.shipType;
    data.length = vesselInfo.length;
    data.width = vesselInfo.width;
    data.draft = vesselInfo.draft;
    data.position = vesselInfo.position;
    data.speed = vesselInfo.speed;
    data.heading = vesselInfo.heading;
    data.courseOverGround = vesselInfo.courseOverGround;
    data.navigationStatus = vesselInfo.navigationStatus;
    data.rateOfTurn = vesselInfo.rateOfTurn;
    data.destination = vesselInfo.destination;
    data.etaMonth = vesselInfo.etaMonth;
    data.etaDay = vesselInfo.etaDay;
    data.etaHour = vesselInfo.etaHour;
    data.etaMinute = vesselInfo.etaMinute;
    
    return data;
}

/**
 * @brief 生成AIS消息
 * @param vesselData 船舶数据
 * @param messageType 消息类型
 * @return NMEA格式的AIS消息
 */
std::string MainWindow::generateAisMessage(const AISVesselData& vesselData, AISMessageType messageType)
{
    // 使用AISMessageGenerator生成消息
    switch (messageType) {
        case AISMessageType::POSITION_REPORT_CLASS_A:
            return m_aisGenerator.generateType1Message(vesselData);
        case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED:
            return m_aisGenerator.generateType2Message(vesselData);
        case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE:
            return m_aisGenerator.generateType3Message(vesselData);
        case AISMessageType::BASE_STATION_REPORT:
            return m_aisGenerator.generateType4Message(vesselData);
        case AISMessageType::STATIC_VOYAGE_DATA:
            return m_aisGenerator.generateType5Message(vesselData);
        case AISMessageType::STANDARD_CLASS_B_CS_POSITION:
            return m_aisGenerator.generateType18Message(vesselData);
        case AISMessageType::EXTENDED_CLASS_B_CS_POSITION:
            return m_aisGenerator.generateType19Message(vesselData);
        case AISMessageType::AID_TO_NAVIGATION_REPORT:
            return m_aisGenerator.generateType21Message(vesselData);
        case AISMessageType::STATIC_DATA_REPORT:
            // Type 24分A/B两部分，这里随机选择
            return m_aisGenerator.generateType24Message(vesselData, 
                QRandomGenerator::global()->bounded(2));
        default:
            // 对于其他消息类型，默认使用Type 1
            return m_aisGenerator.generateType1Message(vesselData);
    }
}

/**
 * @brief 随机生成船舶功能
 * 在地图可见范围内生成指定数量的随机船舶
 */
void MainWindow::onRandomGenerate()
{
    // 创建对话框
    QDialog dialog(this);
    dialog.setWindowTitle("随机生成设备");
    dialog.setFixedWidth(400);
    
    QFormLayout form(&dialog);
    
    QSpinBox *countSpinBox = new QSpinBox();
    countSpinBox->setRange(1, 50); // 减少最大数量避免卡顿
    countSpinBox->setValue(5);
    countSpinBox->setSuffix(" 个");
    form.addRow("生成数量:", countSpinBox);
    
    QSpinBox *routePointsSpinBox = new QSpinBox();
    routePointsSpinBox->setRange(2, 8); // 减少最大航点数
    routePointsSpinBox->setValue(3);
    routePointsSpinBox->setSuffix(" 个航点");
    form.addRow("航线点数:", routePointsSpinBox);
    
    QComboBox *messageTypeComboBox = new QComboBox();
    // 添加常用消息类型
    messageTypeComboBox->addItem("Type 1 - A类位置报告", static_cast<int>(ais::AISMessageType::POSITION_REPORT_CLASS_A));
    messageTypeComboBox->addItem("Type 4 - 基站报告", static_cast<int>(ais::AISMessageType::BASE_STATION_REPORT));
    messageTypeComboBox->addItem("Type 5 - 静态和航程数据", static_cast<int>(ais::AISMessageType::STATIC_VOYAGE_DATA));
    messageTypeComboBox->addItem("Type 18 - B类位置报告", static_cast<int>(ais::AISMessageType::STANDARD_CLASS_B_CS_POSITION));
    messageTypeComboBox->addItem("Type 21 - 助航设备报告", static_cast<int>(ais::AISMessageType::AID_TO_NAVIGATION_REPORT));
    messageTypeComboBox->addItem("Type 24 - 静态数据报告", static_cast<int>(ais::AISMessageType::STATIC_DATA_REPORT));
    form.addRow("消息类型:", messageTypeComboBox);
    
    QCheckBox *autoStartCheckBox = new QCheckBox("生成后自动启动");
    autoStartCheckBox->setChecked(true);
    form.addRow("", autoStartCheckBox);
    
    QLabel *noteLabel = new QLabel("注意：将在当前地图可见范围内随机生成设备，性能优化版本");
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("color: gray; font-style: italic;");
    form.addRow(noteLabel);
    
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        int count = countSpinBox->value();
        int routePoints = routePointsSpinBox->value();

        AISMessageType messageType = static_cast<AISMessageType>(messageTypeComboBox->currentData().toInt());
        bool autoStart = autoStartCheckBox->isChecked();
        
        logMessage(QString("开始随机生成 %1 个设备，类型: %2").arg(count).arg(getMessageTypeName(messageType)));
        
        for (int i = 0; i < count; i++) {
            // 生成随机起点
            QGeoCoordinate startPos = generateRandomPositionInView();
            
            // 对于固定设备（基站、助航设备），不需要航线
            QVector<QGeoCoordinate> route;
            if (messageType == AISMessageType::BASE_STATION_REPORT || 
                messageType == AISMessageType::AID_TO_NAVIGATION_REPORT) {
                route = {startPos}; // 只有一个点
            } else {
                route = generateRandomRoute(startPos, routePoints);
            }
            
            // 创建任务
            QString taskName = QString("%1_%2").arg(getMessageTypeName(messageType)).arg(i + 1);
            createAisTask(taskName, route, messageType);
            
            // 如果自动启动，启动任务
            if (autoStart && !m_aisTasks.isEmpty()) {
                m_aisTasks.last().isActive = true;
                m_aisTasks.last().startTime = QDateTime::currentDateTime();
            }
        }
        
        logMessage(QString("成功生成 %1 个设备").arg(count));
        m_statusLabel->setText(QString("已生成 %1 个设备").arg(count));
        
        QMessageBox::information(this, "完成", 
            QString("成功生成 %1 个设备！\n类型: %2").arg(count).arg(getMessageTypeName(messageType)));
    }
}

/**
 * @brief 在地图可见范围内生成随机位置
 * @return 随机坐标
 */
QGeoCoordinate MainWindow::generateRandomPositionInView()
{
    // 获取当前地图中心和缩放级别
    QGeoCoordinate center = m_mapView->currentCenter();
    double zoomLevel = m_mapView->zoomLevel();
    
    // 根据缩放级别计算合理的偏移范围（度）
    double range = 0.3 / qPow(2, zoomLevel - 10); // 减小范围避免过于分散
    
    // 生成随机偏移
    double latOffset = (QRandomGenerator::global()->generateDouble() - 0.5) * range;
    double lonOffset = (QRandomGenerator::global()->generateDouble() - 0.5) * range;
    
    return QGeoCoordinate(
        center.latitude() + latOffset,
        center.longitude() + lonOffset
    );
}

/**
 * @brief 生成随机航线
 * @param startPos 起始位置
 * @param numPoints 航点数量
 * @return 航线路径
 */
QVector<QGeoCoordinate> MainWindow::generateRandomRoute(const QGeoCoordinate& startPos, int numPoints)
{
    QVector<QGeoCoordinate> route;
    route.append(startPos);
    
    QGeoCoordinate currentPos = startPos;
    
    // 根据缩放级别确定航点间距
    double zoomLevel = m_mapView->zoomLevel();
    double stepRange = 0.2 / qPow(2, zoomLevel - 10); // 减小步长
    
    for (int i = 1; i < numPoints; i++) {
        // 生成下一个航点（相对于当前点的偏移）
        double latOffset = (QRandomGenerator::global()->generateDouble() - 0.5) * stepRange;
        double lonOffset = (QRandomGenerator::global()->generateDouble() - 0.5) * stepRange;
        
        QGeoCoordinate nextPos(
            currentPos.latitude() + latOffset,
            currentPos.longitude() + lonOffset
        );
        
        route.append(nextPos);
        currentPos = nextPos;
    }
    
    return route;
}

/**
 * @brief 获取消息类型名称
 * @param type 消息类型
 * @return 类型名称
 */
QString MainWindow::getMessageTypeName(AISMessageType type) const
{
    switch (type) {
        case AISMessageType::POSITION_REPORT_CLASS_A: return "Type 1";
        case AISMessageType::POSITION_REPORT_CLASS_A_ASSIGNED: return "Type 2";
        case AISMessageType::POSITION_REPORT_CLASS_A_RESPONSE: return "Type 3";
        case AISMessageType::BASE_STATION_REPORT: return "Type 4";
        case AISMessageType::STATIC_VOYAGE_DATA: return "Type 5";
        case AISMessageType::BINARY_ADDRESSED_MESSAGE: return "Type 6";
        case AISMessageType::BINARY_ACKNOWLEDGE: return "Type 7";
        case AISMessageType::BINARY_BROADCAST_MESSAGE: return "Type 8";
        case AISMessageType::STANDARD_SAR_AIRCRAFT_REPORT: return "Type 9";
        case AISMessageType::UTC_DATE_INQUIRY: return "Type 10";
        case AISMessageType::UTC_DATE_RESPONSE: return "Type 11";
        case AISMessageType::ADDRESSED_SAFETY_MESSAGE: return "Type 12";
        case AISMessageType::SAFETY_ACKNOWLEDGE: return "Type 13";
        case AISMessageType::SAFETY_RELATED_BROADCAST: return "Type 14";
        case AISMessageType::INTERROGATION: return "Type 15";
        case AISMessageType::ASSIGNMENT_MODE_COMMAND: return "Type 16";
        case AISMessageType::DGNSS_BINARY_BROADCAST: return "Type 17";
        case AISMessageType::STANDARD_CLASS_B_CS_POSITION: return "Type 18";
        case AISMessageType::EXTENDED_CLASS_B_CS_POSITION: return "Type 19";
        case AISMessageType::DATA_LINK_MANAGEMENT: return "Type 20";
        case AISMessageType::AID_TO_NAVIGATION_REPORT: return "Type 21";
        case AISMessageType::CHANNEL_MANAGEMENT: return "Type 22";
        case AISMessageType::GROUP_ASSIGNMENT_COMMAND: return "Type 23";
        case AISMessageType::STATIC_DATA_REPORT: return "Type 24";
        case AISMessageType::SINGLE_SLOT_BINARY_MESSAGE: return "Type 25";
        case AISMessageType::MULTIPLE_SLOT_BINARY_MESSAGE: return "Type 26";
        case AISMessageType::POSITION_REPORT_LONG_RANGE: return "Type 27";
        default: return "Unknown";
    }
}

/**
 * @brief 记录日志消息
 * @param message 日志内容
 */
void MainWindow::logMessage(const QString &message)
{
    m_logTextEdit->append(message);
    
    // 自动滚动到底部
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
    m_logTextEdit->ensureCursorVisible();
}