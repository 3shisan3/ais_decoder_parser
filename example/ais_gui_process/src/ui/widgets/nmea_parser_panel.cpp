#include "nmea_parser_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>

NMEAParserPanel::NMEAParserPanel(QWidget *parent)
    : QWidget(parent)
    , m_parserManager(nullptr)
    , m_totalParsed(0)
    , m_successCount(0)
    , m_errorCount(0)
{
    createUI();
}

NMEAParserPanel::~NMEAParserPanel()
{
}

void NMEAParserPanel::setParserManager(AISParserManager *manager)
{
    m_parserManager = manager;
    if (m_parserManager) {
        connect(m_parserManager, &AISParserManager::parseCompleted,
                this, &NMEAParserPanel::onParseCompleted);
        connect(m_parserManager, &AISParserManager::batchParseCompleted,
                this, &NMEAParserPanel::onBatchParseCompleted);
        connect(m_parserManager, &AISParserManager::parseError,
                this, &NMEAParserPanel::onParseError);
    }
}

void NMEAParserPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 输入区域
    QLabel *inputLabel = new QLabel("NMEA输入:", this);
    m_inputTextEdit = new QTextEdit(this);
    m_inputTextEdit->setPlaceholderText("请输入NMEA语句（每行一条）或拖放文件到此区域...");
    m_inputTextEdit->setAcceptDrops(true);
    m_inputTextEdit->setMinimumHeight(100);
    
    // 控制按钮区域
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_parseButton = new QPushButton("解析", this);
    m_loadFileButton = new QPushButton("从文件加载", this);
    m_clearButton = new QPushButton("清空", this);
    m_exportButton = new QPushButton("导出结果", this);
    m_displayFormatCombo = new QComboBox(this);
    m_displayFormatCombo->addItems({"树状视图", "JSON格式", "文本摘要"});
    
    controlLayout->addWidget(m_parseButton);
    controlLayout->addWidget(m_loadFileButton);
    controlLayout->addWidget(m_clearButton);
    controlLayout->addWidget(m_exportButton);
    controlLayout->addWidget(new QLabel("显示格式:", this));
    controlLayout->addWidget(m_displayFormatCombo);
    controlLayout->addStretch();
    
    // // 统计信息
    // QHBoxLayout *statsLayout = new QHBoxLayout();
    // m_statsLabel = new QLabel("总计: 0, 成功: 0, 失败: 0", this);
    // statsLayout->addWidget(m_statsLabel);
    // statsLayout->addStretch();
    
    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 100);
    
    // 输出区域 - 使用分割器
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    
    // 树状结果显示
    m_resultTree = new QTreeWidget(this);
    m_resultTree->setHeaderLabels({"字段", "值", "描述"});
    m_resultTree->setColumnWidth(0, 150);
    m_resultTree->setColumnWidth(1, 200);
    m_resultTree->header()->setSectionResizeMode(QHeaderView::Interactive);
    
    // 文本结果显示
    m_outputTextEdit = new QTextEdit(this);
    m_outputTextEdit->setReadOnly(true);
    m_outputTextEdit->setFontFamily("Courier New");
    
    splitter->addWidget(m_resultTree);
    splitter->addWidget(m_outputTextEdit);
    splitter->setSizes({300, 200});
    
    mainLayout->addWidget(inputLabel);
    mainLayout->addWidget(m_inputTextEdit);
    mainLayout->addLayout(controlLayout);
    // mainLayout->addLayout(statsLayout);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(splitter);
    
    setLayout(mainLayout);
    
    // 连接信号
    connect(m_parseButton, &QPushButton::clicked, this, &NMEAParserPanel::onParseInput);
    connect(m_loadFileButton, &QPushButton::clicked, this, &NMEAParserPanel::onLoadFromFile);
    connect(m_clearButton, &QPushButton::clicked, this, &NMEAParserPanel::onClearAll);
    connect(m_exportButton, &QPushButton::clicked, this, &NMEAParserPanel::onExportResults);
    connect(m_displayFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                m_resultTree->setVisible(index == 0);
                m_outputTextEdit->setVisible(index > 0);
            });
}

void NMEAParserPanel::onParseInput()
{
    if (!m_parserManager) {
        QMessageBox::warning(this, "错误", "解析器未初始化");
        return;
    }
    
    QString nmeaText = m_inputTextEdit->toPlainText().trimmed();
    if (nmeaText.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入NMEA语句");
        return;
    }
    
    QStringList messages = nmeaText.split('\n', Qt::SkipEmptyParts);
    if (messages.size() == 1) {
        // 单条解析
        m_parserManager->parseNMEAString(messages.first());
    } else {
        // 批量解析
        m_progressBar->setVisible(true);
        m_progressBar->setValue(0);
        m_parserManager->parseNMEABatch(messages);
    }
}

void NMEAParserPanel::onLoadFromFile()
{
    if (!m_parserManager) {
        QMessageBox::warning(this, "错误", "解析器未初始化");
        return;
    }
    
    QString fileName = QFileDialog::getOpenFileName(this, "打开NMEA文件", 
                                                   "", 
                                                   "NMEA文件 (*.nmea *.txt *.log);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        m_progressBar->setVisible(true);
        m_progressBar->setValue(0);
        
        // 在后台线程中处理文件
        QTimer::singleShot(0, this, [this, fileName]() {
            m_parserManager->parseNMEAFile(fileName);
        });
    }
}

void NMEAParserPanel::onClearAll()
{
    m_inputTextEdit->clear();
    m_outputTextEdit->clear();
    m_resultTree->clear();
    m_currentResults.clear();
    m_totalParsed = m_successCount = m_errorCount = 0;
}

void NMEAParserPanel::onExportResults()
{
    if (m_currentResults.isEmpty()) {
        QMessageBox::information(this, "信息", "没有可导出的结果");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出解析结果", 
                                                   "ais_parse_results.json",
                                                   "JSON文件 (*.json);;文本文件 (*.txt);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray jsonArray;
            for (const QVariant &result : m_currentResults) {
                jsonArray.append(QJsonValue::fromVariant(result));
            }
            
            QJsonDocument doc(jsonArray);
            file.write(doc.toJson());
            file.close();
            
            QMessageBox::information(this, "成功", QString("已导出 %1 条结果").arg(m_currentResults.size()));
        }
    }
}

void NMEAParserPanel::onParseCompleted(const QVariantMap &result)
{
    displayParseResult(result);
    m_currentResults.append(result);
    updateStatistics();
}

void NMEAParserPanel::onBatchParseCompleted(const QVariantList &results)
{
    m_progressBar->setVisible(false);
    m_currentResults = results;
    
    // 显示第一条结果的摘要
    if (!results.isEmpty()) {
        displayParseResult(results.first().toMap());
    }
    
    updateStatistics();
    QMessageBox::information(this, "完成", 
        QString("批量解析完成\n成功: %1\n失败: %2").arg(m_successCount).arg(m_errorCount));
}

void NMEAParserPanel::onParseError(const QString &errorMessage)
{
    m_progressBar->setVisible(false);
    QMessageBox::warning(this, "解析错误", errorMessage);
}

void NMEAParserPanel::displayParseResult(const QVariantMap &result)
{
    if (m_displayFormatCombo->currentIndex() == 0) {
        // 树状视图显示
        m_resultTree->clear();
        
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(m_resultTree);
        rootItem->setText(0, "解析结果");
        rootItem->setText(1, result["success"].toBool() ? "成功" : "失败");
        
        for (auto it = result.constBegin(); it != result.constEnd(); ++it) {
            QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
            item->setText(0, it.key());
            item->setText(1, it.value().toString());
            
            // 添加字段描述
            if (it.key() == "type") {
                item->setText(2, "AIS消息类型");
            } else if (it.key() == "mmsi") {
                item->setText(2, "船舶MMSI号码");
            } else if (it.key() == "rawNMEA") {
                item->setText(2, "原始NMEA语句");
            }
        }
        m_resultTree->expandItem(rootItem);
    } else if (m_displayFormatCombo->currentIndex() == 1) {
        // JSON格式显示
        QJsonDocument doc = QJsonDocument::fromVariant(result);
        m_outputTextEdit->setText(doc.toJson(QJsonDocument::Indented));
    } else {
        // 文本摘要显示
        QString summary = QString("解析结果: %1\n")
            .arg(result["success"].toBool() ? "成功" : "失败");
        
        if (result["success"].toBool()) {
            summary += QString("消息类型: %1\nMMSI: %2\n时间: %3")
                .arg(result["type"].toString())
                .arg(result["mmsi"].toString())
                .arg(result["timestamp"].toString());
        } else {
            summary += QString("错误: %1").arg(result["error"].toString());
        }
        
        m_outputTextEdit->setText(summary);
    }
}

void NMEAParserPanel::updateStatistics()
{
    m_successCount = 0;
    m_errorCount = 0;
    
    for (const QVariant &result : m_currentResults) {
        if (result.toMap()["success"].toBool()) {
            m_successCount++;
        } else {
            m_errorCount++;
        }
    }
    
    m_totalParsed = m_currentResults.size();
    setWindowTitle(QString("NMEA解析器 - 总计: %1, 成功: %2, 失败: %3")
                  .arg(m_totalParsed).arg(m_successCount).arg(m_errorCount));
}