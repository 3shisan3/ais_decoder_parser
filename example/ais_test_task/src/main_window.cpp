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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_udpHost("127.0.0.1")
    , m_udpPort(10110)
    , m_sendInterval(1000)
{
    setupUi();
    setupConnections();
    
    // 初始化定时器
    m_generationTimer = new QTimer(this);
    m_generationTimer->setInterval(1000);
    connect(m_generationTimer, &QTimer::timeout, this, &MainWindow::onGenerateAisData);
    
    m_sendingTimer = new QTimer(this);
    m_sendingTimer->setInterval(m_sendInterval);
    connect(m_sendingTimer, &QTimer::timeout, this, &MainWindow::onSendAisData);
    
    // 初始化UDP socket
    m_udpSocket = new QUdpSocket(this);
    
    m_generationTimer->start();
    m_sendingTimer->start();
    
    logMessage("AIS数据生成与发送系统已启动");
    logMessage(QString("UDP发送目标: %1:%2").arg(m_udpHost).arg(m_udpPort));
}

MainWindow::~MainWindow()
{
    m_generationTimer->stop();
    m_sendingTimer->stop();
    m_udpSocket->close();
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

void MainWindow::onSendAisData()
{
    int sentCount = 0;
    QDateTime currentTime = QDateTime::currentDateTime();
    
    for (const AisGenerationTask &task : m_aisTasks) {
        if (!task.isActive || !task.vesselInfo.position.isValid()) {
            continue;
        }
        
        std::string aisData = generateAisMessage(task.vesselInfo, task.vesselInfo.position);
        if (!aisData.empty()) {
            // 发送UDP数据
            qint64 bytesSent = m_udpSocket->writeDatagram(
                aisData.c_str(), 
                static_cast<qint64>(aisData.size()),
                QHostAddress(m_udpHost), 
                m_udpPort
            );
            
            if (bytesSent > 0) {
                // 记录发送的AIS数据内容
                QString logmsg = QString("[%1] 发送AIS数据到 %2:%3\n")
                    .arg(currentTime.toString("hh:mm:ss.zzz"))
                    .arg(m_udpHost)
                    .arg(m_udpPort);
                
                logmsg += QString("  船舶: %1 (MMSI: %2)\n")
                    .arg(task.vesselInfo.vesselName)
                    .arg(task.vesselInfo.mmsi);
                
                logmsg += QString("  位置: 纬度 %1, 经度 %2\n")
                    .arg(task.vesselInfo.position.latitude(), 0, 'f', 6)
                    .arg(task.vesselInfo.position.longitude(), 0, 'f', 6);
                
                logmsg += QString("  航向: %1°, 航速: %2节\n")
                    .arg(task.vesselInfo.heading, 0, 'f', 1)
                    .arg(task.vesselInfo.speed, 0, 'f', 1);
                
                logmsg += QString("  数据: %1\n")
                    .arg(QString::fromStdString(aisData));
                
                logMessage(logmsg);
                sentCount++;
            } else {
                logMessage(QString("[%1] UDP发送失败到 %2:%3")
                    .arg(currentTime.toString("hh:mm:ss.zzz"))
                    .arg(m_udpHost)
                    .arg(m_udpPort));
            }
        }
    }
    
    if (sentCount > 0) {
        m_statusLabel->setText(QString("已发送 %1 条AIS数据").arg(sentCount));
    }
}

// QByteArray MainWindow::generateAisMessage(const AisVesselInfo &vessel, const QGeoCoordinate &position)
// {
//     // 简化的AIS消息生成（模拟AIS消息类型1：位置报告）
//     QString aisMessage = QString("!AIVDM,1,1,,A,%1,%2,%3,%4,%5,%6,%7,%8")
//         .arg(vessel.mmsi)
//         .arg(position.latitude(), 0, 'f', 6)
//         .arg(position.longitude(), 0, 'f', 6)
//         .arg(vessel.heading, 0, 'f', 1)
//         .arg(vessel.speed, 0, 'f', 1)
//         .arg(vessel.heading, 0, 'f', 1)
//         .arg(QDateTime::currentDateTimeUtc().toString("yyMMddhhmm"))
//         .arg(vessel.vesselName);
    
//     return aisMessage.toUtf8();
// }

void MainWindow::logMessage(const QString &message)
{
    m_logTextEdit->append(message);
    // 自动滚动到底部
    QTextCursor cursor = m_logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logTextEdit->setTextCursor(cursor);
}



// 辅助函数：添加无符号32位整数到比特流（修正比特顺序）
void appendUInt32(std::vector<bool> &bits, uint32_t value, size_t length)
{
    for (int i = static_cast<int>(length) - 1; i >= 0; i--) {
        bits.push_back((value >> i) & 1);
    }
}

// 辅助函数：添加有符号32位整数到比特流（使用二进制补码）
void appendInt32(std::vector<bool> &bits, int32_t value, size_t length)
{
    if (value < 0) {
        // 处理负数：计算补码
        uint32_t mask = (1UL << length) - 1;
        uint32_t positiveValue = static_cast<uint32_t>(-value);
        uint32_t twosComplement = (~positiveValue + 1) & mask;
        appendUInt32(bits, twosComplement, length);
    } else {
        appendUInt32(bits, static_cast<uint32_t>(value), length);
    }
}

// 辅助函数：添加布尔值到比特流
void appendBool(std::vector<bool> &bits, bool value)
{
    bits.push_back(value);
}

// 辅助函数：将二进制数据转换为6-bit ASCII
std::string binaryTo6bitAscii(const std::vector<bool> &bits)
{
    std::string result;
    size_t bitCount = bits.size();
    
    // 确保比特数是6的倍数，不足的补0
    size_t padding = (6 - (bitCount % 6)) % 6;
    std::vector<bool> paddedBits = bits;
    for (size_t i = 0; i < padding; i++) {
        paddedBits.push_back(false);
    }
    
    // 每6位转换为一个字符
    for (size_t i = 0; i < paddedBits.size(); i += 6) {
        uint8_t value = 0;
        for (int j = 0; j < 6; j++) {
            if (paddedBits[i + j]) {
                value |= (1 << (5 - j));
            }
        }
        
        // 6-bit to ASCII转换 (AIS使用特殊的6-bit ASCII编码)
        value += 48; // AIS 6-bit ASCII从48开始 ('0')
        if (value > 87) value += 8; // 调整到正确的字符范围
        result += static_cast<char>(value);
    }
    
    return result;
}

// 辅助函数：计算NMEA校验和
uint8_t calculateChecksum(const QString &data)
{
    uint8_t checksum = 0;
    QByteArray bytes = data.toUtf8();
    
    // NMEA校验和从'$'后第一个字符开始，到'*'前结束
    // 但这里我们计算整个数据部分的校验和
    for (char c : bytes) {
        checksum ^= static_cast<uint8_t>(c);
    }
    
    return checksum;
}

std::string MainWindow::generateAisMessage(const AisVesselInfo &vessel, const QGeoCoordinate &position)
{
    // 创建BitBuffer来构建AIS消息的二进制数据
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits) - 类型1 (位置报告)
    appendUInt32(bits, 1, 6);
    
    // 2. 重复指示符 (2 bits) - 默认0（不重复）
    appendUInt32(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendUInt32(bits, vessel.mmsi, 30);
    
    // 4. 导航状态 (4 bits) - 默认0（在航）
    appendUInt32(bits, 0, 4);
    
    // 5. 转向率 (8 bits) - 默认0（不转向），0x7F表示不可用
    appendInt32(bits, 0, 8);
    
    // 6. 对地速度 (10 bits) - 节转换为AIS格式，1023表示不可用
    uint32_t sog = static_cast<uint32_t>(std::round(vessel.speed * 10.0));
    sog = std::min(sog, static_cast<uint32_t>(1023)); // 最大102.3节
    appendUInt32(bits, sog, 10);
    
    // 7. 位置精度 (1 bit) - 假设使用GPS（高精度）
    appendBool(bits, true);
    
    // 8. 经度 (28 bits) - 转换为AIS格式（1/10000分）
    double longitude = position.longitude();
    // 确保经度在有效范围内
    if (longitude > 180.0) longitude = 180.0;
    if (longitude < -180.0) longitude = -180.0;
    int32_t lon = static_cast<int32_t>(std::round(longitude * 600000.0));
    appendInt32(bits, lon, 28);
    
    // 9. 纬度 (27 bits) - 转换为AIS格式（1/10000分）
    double latitude = position.latitude();
    // 确保纬度在有效范围内
    if (latitude > 90.0) latitude = 90.0;
    if (latitude < -90.0) latitude = -90.0;
    int32_t lat = static_cast<int32_t>(std::round(latitude * 600000.0));
    appendInt32(bits, lat, 27);
    
    // 10. 对地航向 (12 bits)
    uint32_t cog = static_cast<uint32_t>(std::round(vessel.heading * 10.0));
    cog = std::min(cog, static_cast<uint32_t>(3599));
    appendUInt32(bits, cog, 12);
    
    // 11. 真航向 (9 bits) - 511表示不可用
    uint32_t trueHeading = static_cast<uint32_t>(std::round(vessel.heading));
    trueHeading = std::min(trueHeading, static_cast<uint32_t>(359));
    appendUInt32(bits, trueHeading, 9);
    
    // 12. 时间戳 (6 bits) - 当前UTC时间的秒部分，60表示不可用
    QDateTime now = QDateTime::currentDateTimeUtc();
    appendUInt32(bits, now.time().second(), 6);
    
    // 13. 特殊操纵指示 (2 bits) - 默认0（无特殊操纵）
    appendUInt32(bits, 0, 2);
    
    // 14. 备用位 (3 bits) - 填充0
    appendUInt32(bits, 0, 3);
    
    // 15. RAIM标志 (1 bit) - 默认不使用RAIM
    appendBool(bits, false);
    
    // 16. 通信状态 (19 bits) - SOTDMA通信状态，这里使用简单默认值
    // 同步状态 + 通信状态
    appendUInt32(bits, 0, 2); // 同步状态：UTC直接
    appendUInt32(bits, 0, 3); // 通信状态：默认
    appendUInt32(bits, 0, 14); // 填充
    
    // 将二进制数据转换为6-bit ASCII
    std::string payload = binaryTo6bitAscii(bits);
    
    // 构建完整的NMEA消息 - 使用标准格式
    QString dataPart = QString("AIVDM,1,1,,A,%1,0").arg(QString::fromStdString(payload));
    uint8_t checksum = calculateChecksum(dataPart);
    
    QString nmeaMessage = QString("$%1*%2\r\n")
        .arg(dataPart)
        .arg(checksum, 2, 16, QLatin1Char('0')).toUpper();
    
    return nmeaMessage.toStdString();
}