#ifndef AIS_PARSER_MANAGER_H
#define AIS_PARSER_MANAGER_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QString>

#include <memory>
#include <vector>
#include <string>

#include "ais_parser.h"
#include "message.h"     

class AISParserManager : public QObject
{
    Q_OBJECT

public:
    explicit AISParserManager(QObject *parent = nullptr);
    ~AISParserManager();

    // 初始化解析器配置
    void initializeParser(const ais::AISParseCfg &config);

    // 解析单个NMEA字符串
    QVariantMap parseNMEAString(const QString &nmeaString);

    // 批量解析NMEA字符串
    QVariantList parseNMEABatch(const QStringList &nmeaStrings);

    // 从文件解析NMEA数据
    QVariantList parseNMEAFile(const QString &filePath);

    // 获取解析器状态
    QString getParserStatus() const;

signals:
    void parseCompleted(const QVariantMap &result);
    void batchParseCompleted(const QVariantList &results);
    void parseError(const QString &errorMessage);

private:
    std::unique_ptr<ais::AISParser> m_parser;
    ais::AISParseCfg m_config;
};

#endif // AIS_PARSER_MANAGER_H