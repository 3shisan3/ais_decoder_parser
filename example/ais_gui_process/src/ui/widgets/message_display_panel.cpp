#include "message_display_panel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>

MessageDisplayPanel::MessageDisplayPanel(QWidget *parent)
    : QWidget(parent)
{
    createUI();
}

void MessageDisplayPanel::createUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 控制按钮栏
    QHBoxLayout *controlLayout = new QHBoxLayout();
    
    clearButton = new QPushButton("清空消息", this);
    saveButton = new QPushButton("保存到文件", this);
    filterComboBox = new QComboBox(this);
    
    filterComboBox->addItem("显示所有消息");
    filterComboBox->addItem("仅显示AIS消息");
    filterComboBox->addItem("仅显示状态消息");
    filterComboBox->addItem("仅显示错误消息");
    
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(saveButton);
    controlLayout->addWidget(filterComboBox);
    controlLayout->addStretch();
    
    // 消息显示区域
    messageTextEdit = new QTextEdit(this);
    messageTextEdit->setReadOnly(true);
    messageTextEdit->setFontFamily("Courier New");
    messageTextEdit->setFontPointSize(10);
    
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(messageTextEdit);
    
    setLayout(mainLayout);
    
    connect(clearButton, &QPushButton::clicked, this, &MessageDisplayPanel::onClearMessages);
    connect(saveButton, &QPushButton::clicked, this, &MessageDisplayPanel::onSaveToFile);
    connect(filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MessageDisplayPanel::onFilterChanged);
}

void MessageDisplayPanel::addMessage(const QString &message)
{
    messages.append(message);
    messageTextEdit->append(message);
    
    // 自动滚动到底部
    QTextCursor cursor = messageTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    messageTextEdit->setTextCursor(cursor);
}

void MessageDisplayPanel::clearMessages()
{
    messages.clear();
    messageTextEdit->clear();
}

void MessageDisplayPanel::onClearMessages()
{
    clearMessages();
}

void MessageDisplayPanel::onFilterChanged(int index)
{
    // 简化实现：实际应用中可以根据消息类型进行过滤
    messageTextEdit->clear();
    for (const QString &message : messages) {
        // 这里可以根据过滤条件决定是否显示消息
        messageTextEdit->append(message);
    }
}

void MessageDisplayPanel::onSaveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存消息", 
                                                  QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log",
                                                  "日志文件 (*.log);;文本文件 (*.txt);;所有文件 (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        for (const QString &message : messages) {
            stream << message << "\n";
        }
        file.close();
    }
}