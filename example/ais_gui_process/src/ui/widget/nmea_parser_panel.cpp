#include "nmea_parser_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QHeaderView>

// 简化的AIS解析器模拟（实际应使用您提供的AISParser）
class AISParserSimulator
{
public:
    static QVariantMap parse(const QString &nmeaMessage)
    {
        QVariantMap result;
        
        if (nmeaMessage.startsWith("!AIVDM") || nmeaMessage.startsWith("!AIVDO")) {
            result["type"] = "AIS Message";
            result["valid"] = true;
            
            // 简化的AIS消息解析
            QStringList parts = nmeaMessage.split(',');
            if (parts.size() > 5) {
                result["sentence"] = parts[0];
                result["count"] = parts[1];
                result["number"] = parts[2];
                result["seq_id"] = parts[3];
                result["channel"] = parts[4];
                result["payload"] = parts[5];
                
                // 尝试解析MMSI
                if (parts.size() > 5 && !parts[5].isEmpty()) {
                    result["mmsi"] = "未知";
                    // 这里可以添加更复杂的解析逻辑
                }
            }
        } else {
            result["type"] = "Unknown";
            result["valid"] = false;
            result["error"] = "不支持的NMEA语句类型";
        }
        
        return result;
    }
};

NMEAParserPanel::NMEAParserPanel(QWidget *parent)
    : QWidget(parent)
{
    createUI();
}

void NMEAParserPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 输入区域
    QLabel *inputLabel = new QLabel("NMEA输入:", this);
    inputTextEdit = new QTextEdit(this);
    inputTextEdit->setPlaceholderText("请输入NMEA语句或拖放文件到此...");
    inputTextEdit->setAcceptDrops(true);
    
    // 控制按钮
    QHBoxLayout *controlLayout = new QHBoxLayout();
    parseButton = new QPushButton("解析", this);
    loadButton = new QPushButton("从文件加载", this);
    clearButton = new QPushButton("清空", this);
    exportButton = new QPushButton("导出结果", this);
    formatCombo = new QComboBox(this);
    formatCombo->addItems({"树状视图", "JSON格式", "文本格式"});
    
    controlLayout->addWidget(parseButton);
    controlLayout->addWidget(loadButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(exportButton);
    controlLayout->addWidget(new QLabel("显示格式:", this));
    controlLayout->addWidget(formatCombo);
    controlLayout->addStretch();
    
    // 输出区域 - 使用分割器
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    
    // 树状结果显示
    resultTree = new QTreeWidget(this);
    resultTree->setHeaderLabels({"字段", "值"});
    resultTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    // 文本结果显示
    outputTextEdit = new QTextEdit(this);
    outputTextEdit->setReadOnly(true);
    
    splitter->addWidget(resultTree);
    splitter->addWidget(outputTextEdit);
    splitter->setSizes({200, 100});
    
    mainLayout->addWidget(inputLabel);
    mainLayout->addWidget(inputTextEdit);
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(splitter);
    
    setLayout(mainLayout);
    
    // 连接信号
    connect(parseButton, &QPushButton::clicked, this, &NMEAParserPanel::onParseText);
    connect(loadButton, &QPushButton::clicked, this, &NMEAParserPanel::onLoadFromFile);
    connect(clearButton, &QPushButton::clicked, this, &NMEAParserPanel::onClearResults);
    connect(exportButton, &QPushButton::clicked, this, &NMEAParserPanel::onExportResults);
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                resultTree->setVisible(index == 0);
                outputTextEdit->setVisible(index != 0);
            });
}

void NMEAParserPanel::onParseText()
{
    QString nmeaText = inputTextEdit->toPlainText().trimmed();
    if (nmeaText.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入NMEA语句");
        return;
    }
    
    QStringList messages = nmeaText.split('\n', Qt::SkipEmptyParts);
    resultTree->clear();
    outputTextEdit->clear();
    
    for (const QString &message : messages) {
        if (!message.trimmed().isEmpty()) {
            parseNMEAMessage(message.trimmed());
        }
    }
}

void NMEAParserPanel::onLoadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开NMEA文件", 
                                                   "", 
                                                   "NMEA文件 (*.nmea *.txt);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            inputTextEdit->setText(stream.readAll());
            file.close();
            onParseText(); // 自动解析
        }
    }
}

void NMEAParserPanel::onClearResults()
{
    resultTree->clear();
    outputTextEdit->clear();
    inputTextEdit->clear();
}

void NMEAParserPanel::onExportResults()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出解析结果", 
                                                   "ais_parse_result.json",
                                                   "JSON文件 (*.json);;文本文件 (*.txt);;所有文件 (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            if (fileName.endsWith(".json")) {
                // 导出为JSON格式
                QTextStream stream(&file);
                stream << outputTextEdit->toPlainText();
            } else {
                // 导出为文本格式
                file.write(outputTextEdit->toPlainText().toUtf8());
            }
            file.close();
        }
    }
}

void NMEAParserPanel::parseNMEAMessage(const QString &nmeaMessage)
{
    QVariantMap result = AISParserSimulator::parse(nmeaMessage);
    displayParsedResult(result);
}

void NMEAParserPanel::displayParsedResult(const QVariantMap &result)
{
    // 树状显示
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(resultTree);
    rootItem->setText(0, "NMEA消息");
    rootItem->setText(1, result["valid"].toBool() ? "有效" : "无效");
    
    for (auto it = result.constBegin(); it != result.constEnd(); ++it) {
        QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
        item->setText(0, it.key());
        item->setText(1, it.value().toString());
    }
    resultTree->expandItem(rootItem);
    
    // 文本显示
    QString jsonText = outputTextEdit->toPlainText();
    if (!jsonText.isEmpty()) jsonText += "\n";
    
    QJsonDocument doc(QJsonObject::fromVariantMap(result));
    jsonText += doc.toJson(QJsonDocument::Indented);
    outputTextEdit->setText(jsonText);
}