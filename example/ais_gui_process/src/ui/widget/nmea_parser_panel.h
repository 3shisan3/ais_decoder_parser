#ifndef NMEA_PARSER_PANEL_H
#define NMEA_PARSER_PANEL_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTreeWidget>
#include <QFileDialog>

class NMEAParserPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NMEAParserPanel(QWidget *parent = nullptr);

public slots:
    void onParseText();
    void onLoadFromFile();
    void onClearResults();
    void onExportResults();

private:
    void createUI();
    void parseNMEAMessage(const QString &nmeaMessage);
    void displayParsedResult(const QVariantMap &result);
    
    QTextEdit *inputTextEdit;
    QTextEdit *outputTextEdit;
    QTreeWidget *resultTree;
    QPushButton *parseButton;
    QPushButton *loadButton;
    QPushButton *clearButton;
    QPushButton *exportButton;
    QComboBox *formatCombo;
};

#endif // NMEA_PARSER_PANEL_H