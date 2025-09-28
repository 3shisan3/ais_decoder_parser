#ifndef MESSAGE_DISPLAY_PANEL_H
#define MESSAGE_DISPLAY_PANEL_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>

class MessageDisplayPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MessageDisplayPanel(QWidget *parent = nullptr);
    
    void addMessage(const QString &message);
    void clearMessages();

public slots:
    void onClearMessages();
    void onFilterChanged(int index);
    void onSaveToFile();

private:
    void createUI();
    
    QTextEdit *messageTextEdit;
    QPushButton *clearButton;
    QPushButton *saveButton;
    QComboBox *filterComboBox;
    
    QStringList messages;
};

#endif // MESSAGE_DISPLAY_PANEL_H