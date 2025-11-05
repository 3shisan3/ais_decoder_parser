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
#include <QNetworkDatagram>
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
#include <cmath>
#include <algorithm>

// 辅助函数：将整数转换为二进制位（最高位在前）
void appendBits(std::vector<bool> &bits, uint32_t value, size_t bitCount)
{
    for (int i = static_cast<int>(bitCount) - 1; i >= 0; i--) {
        bits.push_back((value >> i) & 1);
    }
}

// 辅助函数：添加有符号整数（使用二进制补码）
void appendSignedBits(std::vector<bool> &bits, int32_t value, size_t bitCount)
{
    if (value < 0) {
        // 计算补码：取绝对值，取反，加1
        uint32_t mask = (1UL << bitCount) - 1;
        uint32_t absValue = static_cast<uint32_t>(std::abs(value));
        uint32_t complement = (~absValue + 1) & mask;
        appendBits(bits, complement, bitCount);
    } else {
        appendBits(bits, static_cast<uint32_t>(value), bitCount);
    }
}

// 辅助函数：添加布尔值
void appendBool(std::vector<bool> &bits, bool value)
{
    bits.push_back(value);
}

// 辅助函数：添加字符串（6-bit ASCII编码）
void appendString(std::vector<bool> &bits, const QString &text, size_t maxBits)
{
    QString paddedText = text.left(maxBits / 6);
    while (paddedText.length() < maxBits / 6) {
        paddedText.append('@'); // @ 表示空格或填充
    }
    
    for (int i = 0; i < paddedText.length(); i++) {
        QChar c = paddedText[i];
        int charValue = 0;
        
        // AIS 6-bit ASCII编码规则
        if (c >= '@' && c <= '_') {
            charValue = c.unicode() - 64; // '@'=0, 'A'=1, ..., '_'=31
        } else if (c >= ' ' && c <= '?') {
            charValue = c.unicode(); // 保持原值（部分特殊字符）
        } else {
            charValue = 0; // 无效字符用空格代替
        }
        
        // 只取6位
        charValue &= 0x3F;
        appendBits(bits, charValue, 6);
    }
}

// 辅助函数：将二进制数据编码为6-bit ASCII（符合AIS标准）
std::string encodeTo6bitAscii(const std::vector<bool> &bits)
{
    std::string result;
    size_t totalBits = bits.size();
    
    // 填充到6的倍数
    size_t padding = (6 - (totalBits % 6)) % 6;
    std::vector<bool> paddedBits = bits;
    for (size_t i = 0; i < padding; i++) {
        paddedBits.push_back(false);
    }
    
    // 每6位转换为一个字符
    for (size_t i = 0; i < paddedBits.size(); i += 6) {
        uint8_t value = 0;
        for (int j = 0; j < 6; j++) {
            if (paddedBits[i + j]) {
                value |= (1 << (5 - j)); // 高位在前
            }
        }
        
        // AIS 6-bit ASCII到可打印字符的转换
        if (value < 40) {
            value += 48; // 0-39 -> 48-87 ('0'-'W')
        } else {
            value += 56; // 40-63 -> 96-119 ('`'-'w')
        }
        
        result += static_cast<char>(value);
    }
    
    return result;
}

// 辅助函数：计算NMEA校验和（正确的实现）
QString calculateNmeaChecksum(const QString &data)
{
    uint8_t checksum = 0;
    QByteArray bytes = data.toUtf8();
    
    // 计算从'$'后第一个字符到'*'前所有字符的异或值
    for (int i = 0; i < bytes.length(); i++) {
        char c = bytes[i];
        if (c == '$') continue; // 跳过起始符
        if (c == '*') break;    // 遇到校验和起始符停止
        checksum ^= static_cast<uint8_t>(c);
    }
    
    return QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper();
}

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
    
    // 初始化定时器
    m_generationTimer = new QTimer(this);
    m_generationTimer->setInterval(1000);
    connect(m_generationTimer, &QTimer::timeout, this, &MainWindow::onGenerateAisData);
    
    m_sendingTimer = new QTimer(this);
    m_sendingTimer->setInterval(m_sendInterval);
    connect(m_sendingTimer, &QTimer::timeout, this, &MainWindow::onSendAisData);
    
    m_generationTimer->start();
    m_sendingTimer->start();
    
    logMessage("AIS数据生成与发送系统已启动");
    logMessage(QString("UDP发送目标: %1:%2").arg(m_udpHost).arg(m_udpPort));
}

MainWindow::~MainWindow()
{
    m_generationTimer->stop();
    m_sendingTimer->stop();

    communicate::Destroy();
}

void MainWindow::setupUi()
{
    // 设置主窗口
    setWindowTitle("AIS数据生成与发送系统");
    setGeometry(100, 100, 1400, 900);
    
    // 创建地图视图
    m_mapView = new SsMultiMapView(this);
    setCentralWidget(m_mapView);
    m_mapView->setTileUrlTemplate("https://wprd01.is.autonavi.com/appmaptile?&style=6&lang=zh_cn&scl=1&ltype=0&x={x}&y={y}&z={z}");
    m_mapView->setCenter(QGeoCoordinate(31.2304, 121.4737)); // 上海
    m_mapView->setZoomLevel(18);
    
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
    
    // 任务列表
    QGroupBox *taskGroup = new QGroupBox("任务列表");
    QVBoxLayout *taskGroupLayout = new QVBoxLayout();
    m_taskList = new QListWidget();
    taskGroupLayout->addWidget(m_taskList);
    taskGroup->setLayout(taskGroupLayout);
    taskLayout->addWidget(taskGroup);
    
    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startButton = new QPushButton("开始");
    m_stopButton = new QPushButton("停止");
    m_deleteButton = new QPushButton("删除");
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_deleteButton);
    taskLayout->addLayout(buttonLayout);
    
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
}

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

void MainWindow::onRoutePlanned(const QVector<QGeoCoordinate> &route)
{
    if (route.size() < 2) {
        QMessageBox::warning(this, "警告", "路线至少需要两个点");
        return;
    }
    
    // 弹出对话框获取任务名称
    QDialog dialog(this);
    dialog.setWindowTitle("创建AIS任务");
    dialog.setFixedWidth(300);
    
    QFormLayout form(&dialog);
    QLineEdit taskNameEdit;
    form.addRow("任务名称:", &taskNameEdit);
    
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString taskName = taskNameEdit.text().trimmed();
        if (taskName.isEmpty()) {
            taskName = QString("任务_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        }
        createAisTask(taskName, route);
    }
}

void MainWindow::createAisTask(const QString &taskName, const QVector<QGeoCoordinate> &route)
{
    AisGenerationTask task;
    task.taskId = QUuid::createUuid().toString();
    task.taskName = taskName;
    task.vesselInfo = generateRandomVesselInfo();
    task.route = route;
    task.updateInterval = 1000;
    task.isActive = false;
    task.startTime = QDateTime::currentDateTime();
    task.currentPointIndex = 0;
    task.progressAlongSegment = 0.0;
    
    m_aisTasks.append(task);
    
    // 更新列表
    QListWidgetItem *item = new QListWidgetItem(taskName);
    item->setData(Qt::UserRole, task.taskId);
    m_taskList->addItem(item);
    
    m_statusLabel->setText(QString("已创建任务: %1").arg(taskName));
    logMessage(QString("创建任务: %1 (MMSI: %2)").arg(taskName).arg(task.vesselInfo.mmsi));
}

AisVesselInfo MainWindow::generateRandomVesselInfo()
{
    AisVesselInfo vessel;
    
    static const QStringList vesselNames = {
        "Ocean Star", "Sea Explorer", "Marine Voyager", "Blue Whale",
        "Pacific King", "Atlantic Queen", "Cargo Master", "Container Express"
    };
    
    static const QList<QColor> vesselColors = {
        Qt::blue, Qt::red, Qt::green, Qt::darkYellow,
        Qt::darkBlue, Qt::darkRed, Qt::darkGreen, Qt::magenta
    };
    
    vessel.vesselName = vesselNames[QRandomGenerator::global()->bounded(vesselNames.size())];
    vessel.vesselColor = vesselColors[QRandomGenerator::global()->bounded(vesselColors.size())];
    vessel.vesselId = QString("VESSEL_%1").arg(QRandomGenerator::global()->bounded(10000, 99999));
    
    vessel.mmsi = generateRandomMmsi().toInt();
    vessel.imo = QRandomGenerator::global()->bounded(9000000, 9999999);
    vessel.callSign = generateRandomCallsign();
    vessel.shipType = QRandomGenerator::global()->bounded(1, 99);
    vessel.length = QRandomGenerator::global()->bounded(50, 300);
    vessel.width = QRandomGenerator::global()->bounded(10, 50);
    vessel.draft = QRandomGenerator::global()->bounded(5, 20) / 10.0;
    vessel.speed = QRandomGenerator::global()->bounded(5, 30);
    vessel.heading = QRandomGenerator::global()->bounded(0, 360);
    
    return vessel;
}

QString MainWindow::generateRandomMmsi()
{
    // MMSI: 9位数字，前3位是MID（海事识别数字）
    static const QStringList midCodes = {"232", "233", "234", "235", "236"};
    QString mid = midCodes[QRandomGenerator::global()->bounded(midCodes.size())];
    QString number = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
    return mid + number;
}

QString MainWindow::generateRandomCallsign()
{
    static const QString letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString callsign;
    for (int i = 0; i < 6; ++i) {
        if (i < 2) {
            callsign.append(letters[QRandomGenerator::global()->bounded(letters.size())]);
        } else {
            callsign.append(QString::number(QRandomGenerator::global()->bounded(0, 9)));
        }
    }
    return callsign;
}

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
            m_statusLabel->setText(QString("已启动任务: %1").arg(task.taskName));
            logMessage(QString("启动任务: %1").arg(task.taskName));
            break;
        }
    }
}

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
            break;
        }
    }
}

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
            m_aisTasks.removeAt(i);
            delete m_taskList->takeItem(m_taskList->row(selectedItems.first()));
            m_statusLabel->setText("已删除任务");
            logMessage(QString("删除任务: %1").arg(taskName));
            break;
        }
    }
}

void MainWindow::onTaskSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems = m_taskList->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();
    
    m_startButton->setEnabled(hasSelection);
    m_stopButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onSendIntervalChanged(int interval)
{
    m_sendInterval = interval;
    m_sendingTimer->setInterval(m_sendInterval);
    logMessage(QString("发送间隔调整为: %1ms").arg(m_sendInterval));
}

void MainWindow::onUdpSettingsChanged()
{
    m_udpHost = m_udpHostEdit->text().trimmed();
    m_udpPort = m_udpPortSpinBox->value();
    logMessage(QString("UDP目标更新为: %1:%2").arg(m_udpHost).arg(m_udpPort));
}

void MainWindow::onToggleTaskDock()
{
    m_taskDock->setVisible(!m_taskDock->isVisible());
}

void MainWindow::onClearLog()
{
    m_logTextEdit->clear();
}

void MainWindow::onGenerateAisData()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    
    for (AisGenerationTask &task : m_aisTasks) {
        if (!task.isActive || task.route.size() < 2) {
            continue;
        }
        
        // 计算船舶位置
        double timeElapsed = task.startTime.msecsTo(currentTime) / 1000.0; // 秒
        double totalDistance = 0.0;
        
        // 计算总距离
        for (int i = 1; i < task.route.size(); ++i) {
            totalDistance += task.route[i-1].distanceTo(task.route[i]);
        }
        
        // 计算当前应完成的比例
        double completion = fmod(timeElapsed * task.vesselInfo.speed * 0.5144, totalDistance) / totalDistance;
        
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
        }
    }
}

// 生成AIS位置报告消息（类型1）
std::string MainWindow::generateAisMessage(const AisVesselInfo &vessel, const QGeoCoordinate &position)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits) - 类型1：位置报告
    appendBits(bits, 1, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vessel.mmsi, 30);
    
    // 4. 导航状态 (4 bits) - 0: 在航
    appendBits(bits, 0, 4);
    
    // 5. 转向率 (8 bits) - 0: 不转向，-128: 不可用
    int32_t rot = 0;
    appendSignedBits(bits, rot, 8);
    
    // 6. 对地速度 (10 bits) - 节*10，1023: 不可用
    uint32_t sog = static_cast<uint32_t>(vessel.speed * 10.0);
    if (sog > 1022) sog = 1023; // 1023表示不可用
    appendBits(bits, sog, 10);
    
    // 7. 位置精度 (1 bit) - 1: 高精度(GPS)
    appendBool(bits, true);
    
    // 8. 经度 (28 bits) - 度*600000，181°: 不可用
    double longitude = position.longitude();
    int32_t lonValue;
    if (longitude < -180.0 || longitude > 180.0) {
        lonValue = 181 * 600000; // 不可用
    } else {
        lonValue = static_cast<int32_t>(longitude * 600000.0);
    }
    appendSignedBits(bits, lonValue, 28);
    
    // 9. 纬度 (27 bits) - 度*600000，91°: 不可用
    double latitude = position.latitude();
    int32_t latValue;
    if (latitude < -90.0 || latitude > 90.0) {
        latValue = 91 * 600000; // 不可用
    } else {
        latValue = static_cast<int32_t>(latitude * 600000.0);
    }
    appendSignedBits(bits, latValue, 27);
    
    // 10. 对地航向 (12 bits) - 度*10，3600: 不可用
    uint32_t cog = static_cast<uint32_t>(vessel.heading * 10.0);
    if (cog >= 3600) cog = 3600; // 不可用
    appendBits(bits, cog, 12);
    
    // 11. 真航向 (9 bits) - 度，511: 不可用
    uint32_t heading = static_cast<uint32_t>(vessel.heading);
    if (heading > 359) heading = 511; // 不可用
    appendBits(bits, heading, 9);
    
    // 12. 时间戳 (6 bits) - UTC秒，60: 不可用
    QDateTime utc = QDateTime::currentDateTimeUtc();
    appendBits(bits, utc.time().second(), 6);
    
    // 13. 特殊操纵指示 (2 bits) - 0: 无特殊操纵
    appendBits(bits, 0, 2);
    
    // 14. 备用位 (3 bits)
    appendBits(bits, 0, 3);
    
    // 15. RAIM标志 (1 bit) - 0: 未使用RAIM
    appendBool(bits, false);
    
    // 16. 通信状态 (19 bits) - SOTDMA状态
    // 同步状态(2) + 通信状态(3) + 时隙偏移(14)
    appendBits(bits, 0, 2);  // UTC同步
    appendBits(bits, 0, 3); // 通信状态
    appendBits(bits, 0, 14); // 时隙偏移
    
    // 编码为6-bit ASCII
    std::string payload = encodeTo6bitAscii(bits);
    
    // 构建NMEA语句
    QString nmeaBody = QString("AIVDM,1,1,,A,%1,0").arg(QString::fromStdString(payload));
    QString checksum = calculateNmeaChecksum(nmeaBody);
    
    // 完整NMEA语句：!AIVDM,...*CS<CR><LF>
    QString nmeaMessage = QString("!%1*%2\r\n").arg(nmeaBody).arg(checksum);
    
    return nmeaMessage.toStdString();
}

/* // 生成AIS静态数据消息（类型5）
std::string MainWindow::generateAisStaticMessage(const AisVesselInfo &vessel)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits) - 类型5：静态和航程数据
    appendBits(bits, 5, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vessel.mmsi, 30);
    
    // 4. AIS版本 (2 bits) - 0: 符合标准
    appendBits(bits, 0, 2);
    
    // 5. IMO编号 (30 bits)
    appendBits(bits, vessel.imo, 30);
    
    // 6. 呼号 (42 bits = 7字符)
    appendString(bits, vessel.callSign, 42);
    
    // 7. 船名 (120 bits = 20字符)
    appendString(bits, vessel.vesselName, 120);
    
    // 8. 船舶类型 (8 bits)
    appendBits(bits, vessel.shipType, 8);
    
    // 9. 船舶尺寸 (30 bits)
    // 计算船舶尺寸（基于长度和宽度）
    int dimensionToBow = vessel.length / 2;
    int dimensionToStern = vessel.length - dimensionToBow;
    int dimensionToPort = vessel.width / 2;
    int dimensionToStarboard = vessel.width - dimensionToPort;
    
    appendBits(bits, dimensionToBow, 9);   // 到船首距离
    appendBits(bits, dimensionToStern, 9);  // 到船尾距离
    appendBits(bits, dimensionToPort, 6);   // 到左舷距离
    appendBits(bits, dimensionToStarboard, 6); // 到右舷距离
    
    // 10. 定位设备类型 (4 bits) - 1: GPS
    appendBits(bits, 1, 4);
    
    // 11. ETA (20 bits) - 月(4)+日(5)+时(5)+分(6)
    QDateTime eta = QDateTime::currentDateTime().addDays(1);
    appendBits(bits, eta.date().month(), 4);
    appendBits(bits, eta.date().day(), 5);
    appendBits(bits, eta.time().hour(), 5);
    appendBits(bits, eta.time().minute(), 6);
    
    // 12. 吃水深度 (8 bits) - 米*10
    uint32_t draught = static_cast<uint32_t>(vessel.draft * 10.0);
    appendBits(bits, draught, 8);
    
    // 13. 目的地 (120 bits = 20字符)
    appendString(bits, "SHANGHAI", 120);
    
    // 14. DTE标志 (1 bit) - 1: 就绪
    appendBool(bits, true);
    
    // 15. 备用位 (1 bit)
    appendBool(bits, false);
    
    // 编码为6-bit ASCII
    std::string payload = encodeTo6bitAscii(bits);
    
    // 构建NMEA语句
    QString nmeaBody = QString("AIVDM,1,1,,A,%1,0").arg(QString::fromStdString(payload));
    QString checksum = calculateNmeaChecksum(nmeaBody);
    
    QString nmeaMessage = QString("!%1*%2\r\n").arg(nmeaBody).arg(checksum);
    
    return nmeaMessage.toStdString();
}

// 生成AIS B类设备位置报告（类型18）
std::string MainWindow::generateAisType18Message(const AisVesselInfo &vessel, const QGeoCoordinate &position)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits) - 类型18：标准B类设备位置报告
    appendBits(bits, 18, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vessel.mmsi, 30);
    
    // 4. 备用位 (8 bits)
    appendBits(bits, 0, 8);
    
    // 5. 对地速度 (10 bits)
    uint32_t sog = static_cast<uint32_t>(vessel.speed * 10.0);
    if (sog > 1022) sog = 1023;
    appendBits(bits, sog, 10);
    
    // 6. 位置精度 (1 bit)
    appendBool(bits, true);
    
    // 7. 经度 (28 bits)
    double longitude = position.longitude();
    int32_t lonValue = static_cast<int32_t>(longitude * 600000.0);
    appendSignedBits(bits, lonValue, 28);
    
    // 8. 纬度 (27 bits)
    double latitude = position.latitude();
    int32_t latValue = static_cast<int32_t>(latitude * 600000.0);
    appendSignedBits(bits, latValue, 27);
    
    // 9. 对地航向 (12 bits)
    uint32_t cog = static_cast<uint32_t>(vessel.heading * 10.0);
    if (cog >= 3600) cog = 3600;
    appendBits(bits, cog, 12);
    
    // 10. 真航向 (9 bits)
    uint32_t heading = static_cast<uint32_t>(vessel.heading);
    if (heading > 359) heading = 511;
    appendBits(bits, heading, 9);
    
    // 11. 时间戳 (6 bits)
    QDateTime utc = QDateTime::currentDateTimeUtc();
    appendBits(bits, utc.time().second(), 6);
    
    // 12. 备用位 (2 bits)
    appendBits(bits, 0, 2);
    
    // 13. CS单元标志 (2 bits) - 0: Class B CS
    appendBits(bits, 0, 2);
    
    // 14. 显示标志 (1 bit) - 1: 有显示
    appendBool(bits, true);
    
    // 15. DSC标志 (1 bit) - 1: 有DSC
    appendBool(bits, true);
    
    // 16. 频带标志 (1 bit) - 0: 使用默认频带
    appendBool(bits, false);
    
    // 17. 消息22标志 (1 bit) - 0: 不接受消息22
    appendBool(bits, false);
    
    // 18. 分配模式标志 (1 bit) - 0: 自主模式
    appendBool(bits, false);
    
    // 19. RAIM标志 (1 bit)
    appendBool(bits, false);
    
    // 20. 通信状态 (19 bits)
    appendBits(bits, 0, 19);
    
    // 编码为6-bit ASCII
    std::string payload = encodeTo6bitAscii(bits);
    
    // 构建NMEA语句
    QString nmeaBody = QString("AIVDM,1,1,,A,%1,0").arg(QString::fromStdString(payload));
    QString checksum = calculateNmeaChecksum(nmeaBody);
    
    QString nmeaMessage = QString("!%1*%2\r\n").arg(nmeaBody).arg(checksum);
    
    return nmeaMessage.toStdString();
}
*/

void MainWindow::onSendAisData()
{
    static int messageCounter = 0;
    QDateTime currentTime = QDateTime::currentDateTime();
    int sentCount = 0;
    
    for (const AisGenerationTask &task : m_aisTasks) {
        if (!task.isActive || !task.vesselInfo.position.isValid()) {
            continue;
        }
        
        std::string aisData;
        QString messageType;
        
        // 根据计数器选择消息类型
        // if (messageCounter % 20 == 0) {
        //     // 每20条消息发送一次静态数据
        //     aisData = generateAisStaticMessage(task.vesselInfo);
        //     messageType = "类型5(静态数据)";
        // } else if (messageCounter % 5 == 0) {
        //     // 每5条消息发送一次B类位置报告
        //     aisData = generateAisType18Message(task.vesselInfo, task.vesselInfo.position);
        //     messageType = "类型18(B类位置)";
        // } else {
            // 其他时间发送A类位置报告
            aisData = generateAisMessage(task.vesselInfo, task.vesselInfo.position);
            messageType = "类型1(A类位置)";
        // }
        
        if (!aisData.empty()) {
            // 使用communicate API发送数据
            int ret = communicate::SendGeneralMessage(
                m_udpHost.toStdString().c_str(),
                m_udpPort,
                aisData.data(),
                aisData.size() + 1
            );
            
            if (ret == 0) {
                QString logmsg = QString("[%1] 发送AIS数据到 %2:%3\n")
                    .arg(currentTime.toString("hh:mm:ss.zzz"))
                    .arg(m_udpHost)
                    .arg(m_udpPort);
                
                logmsg += QString("  船舶: %1 (MMSI: %2)\n")
                    .arg(task.vesselInfo.vesselName)
                    .arg(task.vesselInfo.mmsi);
                
                logmsg += QString("  消息类型: %1\n").arg(messageType);
                logmsg += QString("  位置: 纬度 %1, 经度 %2\n")
                    .arg(task.vesselInfo.position.latitude(), 0, 'f', 6)
                    .arg(task.vesselInfo.position.longitude(), 0, 'f', 6);
                
                logmsg += QString("  航向: %1°, 航速: %2节\n")
                    .arg(task.vesselInfo.heading, 0, 'f', 1)
                    .arg(task.vesselInfo.speed, 0, 'f', 1);
                
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
    }
    
    messageCounter++;
    if (sentCount > 0) {
        m_statusLabel->setText(QString("已发送 %1 条AIS数据 (计数器: %2)").arg(sentCount).arg(messageCounter));
    }
}

void MainWindow::logMessage(const QString &message)
{
    m_logTextEdit->append(message);
    // 自动滚动到底部
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
    
    // // 限制日志行数，防止内存过度使用
    // if (m_logTextEdit->document()->lineCount() > 1000) {
    //     QTextCursor cursor(m_logTextEdit->document()->firstBlock());
    //     cursor.select(QTextCursor::BlockUnderCursor);
    //     cursor.removeSelectedText();
    // }
}