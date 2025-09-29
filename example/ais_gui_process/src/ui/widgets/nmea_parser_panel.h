#ifndef NMEA_PARSER_PANEL_H
#define NMEA_PARSER_PANEL_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTreeWidget>
#include <QFileDialog>
#include <QProgressBar>

#include "core/ais_parser_manager.h"

class NMEAParserPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NMEAParserPanel(QWidget *parent = nullptr);
    ~NMEAParserPanel();

    void setParserManager(AISParserManager *manager);

public slots:
    void onParseInput();
    void onLoadFromFile();
    void onClearAll();
    void onExportResults();
    void onParseCompleted(const QVariantMap &result);
    void onBatchParseCompleted(const QVariantList &results);
    void onParseError(const QString &errorMessage);

private:
    void createUI();
    void displayParseResult(const QVariantMap &result);
    void refreshDisplay(); // 刷新显示函数
    void updateStatistics();

    AISParserManager *m_parserManager;
    
    // UI组件
    QTextEdit *m_inputTextEdit;
    QTextEdit *m_outputTextEdit;
    QTreeWidget *m_resultTree;
    QPushButton *m_parseButton;
    QPushButton *m_loadFileButton;
    QPushButton *m_clearButton;
    QPushButton *m_exportButton;
    QComboBox *m_displayFormatCombo;
    QProgressBar *m_progressBar;
    
    // 统计信息
    int m_totalParsed;
    int m_successCount;
    int m_errorCount;
    
    QVariantList m_currentResults;
};

#endif // NMEA_PARSER_PANEL_H