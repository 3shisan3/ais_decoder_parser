#include "ais_parser_manager.h"

#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <iostream>

AISParserManager::AISParserManager(QObject *parent)
    : QObject(parent)
{
    // 使用默认配置初始化解析器
    m_config.validateChecksum = false;
    m_config.enableMultipartReassembly = true;
    m_config.maxMultipartAge = 300;
    
    m_parser = std::make_unique<ais::AISParser>(m_config);
}

AISParserManager::~AISParserManager()
{
}

void AISParserManager::initializeParser(const ais::AISParseCfg &config)
{
    m_config = config;
    m_parser->setConfig(config);
    std::cout << "AIS parser initialized with new configuration" << std::endl;
}

QVariantMap AISParserManager::parseNMEAString(const QString &nmeaString)
{
    QVariantMap result;

    if (!m_parser) {
        result["success"] = false;
        result["error"] = "Parser not initialized";
        emit parseCompleted(result);
        return result;
    }
    
    try {
        std::string nmeaStdStr = nmeaString.toStdString();
        auto message = m_parser->parse(nmeaStdStr);
        
        if (message) {
            result["success"] = true;
            result["type"] = static_cast<int>(message->type);
            result["mmsi"] = static_cast<quint32>(message->mmsi);
            result["rawNMEA"] = QString::fromStdString(message->rawNMEA);
            result["timestamp"] = QString::fromStdString(message->timestamp);
            result["json"] = QString::fromStdString(message->toJson());
            result["csv"] = QString::fromStdString(message->toCsv());
            
            // 根据消息类型添加特定字段
            switch (message->type) {
            case ais::AISMessageType::POSITION_REPORT_CLASS_A:
                // 添加位置相关字段
                break;
            case ais::AISMessageType::STATIC_VOYAGE_DATA:
                // 添加静态数据字段
                break;
            default:
                break;
            }
        } else {
            result["success"] = false;
            result["error"] = "Failed to parse NMEA string";
        }
    } catch (const std::exception &e) {
        result["success"] = false;
        result["error"] = QString("Parse error: %1").arg(e.what());
        std::cerr << "NMEA parse error: " << e.what() << std::endl;
    }
    
    emit parseCompleted(result);
    return result;
}

QVariantList AISParserManager::parseNMEABatch(const QStringList &nmeaStrings)
{
    QVariantList results;
    int successCount = 0;
    int errorCount = 0;
    
    for (const QString &nmeaString : nmeaStrings) {
        if (nmeaString.trimmed().isEmpty()) continue;
        
        QVariantMap result = parseNMEAString(nmeaString);
        results.append(result);
        
        if (result["success"].toBool()) {
            successCount++;
        } else {
            errorCount++;
        }
    }
    
    std::cout << "Batch parse completed: " << successCount << " successful, " << errorCount << " failed" << std::endl;
    emit batchParseCompleted(results);
    return results;
}

QVariantList AISParserManager::parseNMEAFile(const QString &filePath)
{
    QVariantList results;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit parseError(QString("无法打开文件: %1").arg(filePath));
        return results;
    }
    
    QTextStream stream(&file);
    QStringList nmeaStrings;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (!line.isEmpty() && (line.startsWith("!AIVDM") || line.startsWith("!AIVDO"))) {
            nmeaStrings.append(line);
        }
    }
    
    file.close();
    
    if (nmeaStrings.isEmpty()) {
        emit parseError("文件中未找到有效的NMEA语句");
        return results;
    }
    
    std::cout << "Loaded " << nmeaStrings.size() << " NMEA sentences from file: " << filePath.toStdString() << std::endl;
    return parseNMEABatch(nmeaStrings);
}

QString AISParserManager::getParserStatus() const
{
    return QString("Parser configured - Checksum: %1, Multipart: %2, MaxAge: %3s")
        .arg(m_config.validateChecksum ? "enabled" : "disabled")
        .arg(m_config.enableMultipartReassembly ? "enabled" : "disabled")
        .arg(m_config.maxMultipartAge);
}