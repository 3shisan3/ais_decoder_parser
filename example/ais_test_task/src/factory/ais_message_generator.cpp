#include "ais_message_generator.h"
#include <QDateTime>
#include <QtMath>
#include <cmath>

AISMessageGenerator::AISMessageGenerator()
{
}

// 辅助函数：添加二进制位
void AISMessageGenerator::appendBits(std::vector<bool>& bits, uint32_t value, size_t bitCount)
{
    for (int i = static_cast<int>(bitCount) - 1; i >= 0; i--) {
        bits.push_back((value >> i) & 1);
    }
}

// 辅助函数：添加有符号整数
void AISMessageGenerator::appendSignedBits(std::vector<bool>& bits, int32_t value, size_t bitCount)
{
    if (value < 0) {
        // 计算补码
        uint32_t mask = (1UL << bitCount) - 1;
        uint32_t absValue = static_cast<uint32_t>(std::abs(value));
        uint32_t complement = (~absValue + 1) & mask;
        appendBits(bits, complement, bitCount);
    } else {
        appendBits(bits, static_cast<uint32_t>(value), bitCount);
    }
}

// 辅助函数：添加布尔值
void AISMessageGenerator::appendBool(std::vector<bool>& bits, bool value)
{
    bits.push_back(value);
}

// 辅助函数：添加字符串（6-bit ASCII编码）
void AISMessageGenerator::appendString(std::vector<bool>& bits, const QString& text, size_t maxBits)
{
    int maxChars = maxBits / 6;
    QString paddedText = text.left(maxChars);
    
    // 填充到指定长度
    while (paddedText.length() < maxChars) {
        paddedText.append('@'); // @ 表示空格或填充
    }
    
    for (int i = 0; i < paddedText.length(); i++) {
        QChar c = paddedText[i];
        int charValue = 0;
        
        // AIS 6-bit ASCII编码规则
        if (c >= '@' && c <= '_') {
            charValue = c.unicode() - 64; // '@'=0, 'A'=1, ..., '_'=31
        } else if (c >= ' ' && c <= '?') {
            charValue = c.unicode(); // 保持原值
        } else {
            charValue = 0; // 无效字符用空格代替
        }
        
        charValue &= 0x3F; // 只取6位
        appendBits(bits, charValue, 6);
    }
}

// 辅助函数：二进制编码为6-bit ASCII
std::string AISMessageGenerator::encodeTo6bitAscii(const std::vector<bool>& bits)
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
                value |= (1 << (5 - j));
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

// 辅助函数：计算NMEA校验和
QString AISMessageGenerator::calculateNmeaChecksum(const QString& data)
{
    uint8_t checksum = 0;
    QByteArray bytes = data.toUtf8();
    
    for (int i = 0; i < bytes.length(); i++) {
        char c = bytes[i];
        if (c == '$') continue;
        if (c == '*') break;
        checksum ^= static_cast<uint8_t>(c);
    }
    
    return QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper();
}

// 辅助函数：构建NMEA语句
std::string AISMessageGenerator::buildNmeaSentence(const std::string& payload, int fillBits)
{
    QString nmeaBody = QString("AIVDM,1,1,,A,%1,%2")
        .arg(QString::fromStdString(payload))
        .arg(fillBits);
    
    QString checksum = calculateNmeaChecksum(nmeaBody);
    QString nmeaMessage = QString("!%1*%2\r\n").arg(nmeaBody).arg(checksum);
    
    return nmeaMessage.toStdString();
}

// 生成Type 1消息：A类位置报告
std::string AISMessageGenerator::generateType1Message(const AISVesselData& vesselData)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits)
    appendBits(bits, 1, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vesselData.mmsi, 30);
    
    // 4. 导航状态 (4 bits)
    appendBits(bits, vesselData.navigationStatus, 4);
    
    // 5. 转向率 (8 bits)
    appendSignedBits(bits, vesselData.rateOfTurn, 8);
    
    // 6. 对地速度 (10 bits) - 节*10
    uint32_t sog = static_cast<uint32_t>(vesselData.speed * 10.0);
    if (sog > 1022) sog = 1023;
    appendBits(bits, sog, 10);
    
    // 7. 位置精度 (1 bit) - 1: 高精度
    appendBool(bits, true);
    
    // 8. 经度 (28 bits) - 度*600000
    int32_t lonValue = static_cast<int32_t>(vesselData.position.longitude() * 600000.0);
    appendSignedBits(bits, lonValue, 28);
    
    // 9. 纬度 (27 bits) - 度*600000
    int32_t latValue = static_cast<int32_t>(vesselData.position.latitude() * 600000.0);
    appendSignedBits(bits, latValue, 27);
    
    // 10. 对地航向 (12 bits) - 度*10
    uint32_t cog = static_cast<uint32_t>(vesselData.courseOverGround * 10.0);
    if (cog >= 3600) cog = 3600;
    appendBits(bits, cog, 12);
    
    // 11. 真航向 (9 bits)
    uint32_t heading = static_cast<uint32_t>(vesselData.heading);
    if (heading > 359) heading = 511;
    appendBits(bits, heading, 9);
    
    // 12. 时间戳 (6 bits)
    QDateTime utc = QDateTime::currentDateTimeUtc();
    appendBits(bits, utc.time().second(), 6);
    
    // 13. 特殊操纵指示 (2 bits)
    appendBits(bits, 0, 2);
    
    // 14. 备用位 (3 bits)
    appendBits(bits, 0, 3);
    
    // 15. RAIM标志 (1 bit)
    appendBool(bits, false);
    
    // 16. 通信状态 (19 bits)
    appendBits(bits, 0, 19);
    
    // 编码并构建NMEA语句
    std::string payload = encodeTo6bitAscii(bits);
    int fillBits = (6 - (bits.size() % 6)) % 6;
    
    return buildNmeaSentence(payload, fillBits);
}

// 生成Type 5消息：静态和航程数据
std::string AISMessageGenerator::generateType5Message(const AISVesselData& vesselData)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits)
    appendBits(bits, 5, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vesselData.mmsi, 30);
    
    // 4. AIS版本 (2 bits)
    appendBits(bits, 0, 2);
    
    // 5. IMO编号 (30 bits)
    appendBits(bits, vesselData.imo, 30);
    
    // 6. 呼号 (42 bits = 7字符)
    appendString(bits, vesselData.callSign, 42);
    
    // 7. 船名 (120 bits = 20字符)
    appendString(bits, vesselData.vesselName, 120);
    
    // 8. 船舶类型 (8 bits)
    appendBits(bits, vesselData.shipType, 8);
    
    // 9. 船舶尺寸 (30 bits)
    int dimensionToBow = vesselData.length / 2;
    int dimensionToStern = vesselData.length - dimensionToBow;
    int dimensionToPort = vesselData.width / 2;
    int dimensionToStarboard = vesselData.width - dimensionToPort;
    
    appendBits(bits, dimensionToBow, 9);
    appendBits(bits, dimensionToStern, 9);
    appendBits(bits, dimensionToPort, 6);
    appendBits(bits, dimensionToStarboard, 6);
    
    // 10. 定位设备类型 (4 bits) - 1: GPS
    appendBits(bits, 1, 4);
    
    // 11. ETA (20 bits)
    appendBits(bits, vesselData.etaMonth, 4);
    appendBits(bits, vesselData.etaDay, 5);
    appendBits(bits, vesselData.etaHour, 5);
    appendBits(bits, vesselData.etaMinute, 6);
    
    // 12. 吃水深度 (8 bits) - 米*10
    uint32_t draught = static_cast<uint32_t>(vesselData.draft * 10.0);
    appendBits(bits, draught, 8);
    
    // 13. 目的地 (120 bits = 20字符)
    appendString(bits, vesselData.destination, 120);
    
    // 14. DTE标志 (1 bit)
    appendBool(bits, true);
    
    // 15. 备用位 (1 bit)
    appendBool(bits, false);
    
    std::string payload = encodeTo6bitAscii(bits);
    int fillBits = (6 - (bits.size() % 6)) % 6;
    
    return buildNmeaSentence(payload, fillBits);
}

// 生成Type 18消息：标准B类位置报告
std::string AISMessageGenerator::generateType18Message(const AISVesselData& vesselData)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits)
    appendBits(bits, 18, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vesselData.mmsi, 30);
    
    // 4. 备用位 (8 bits)
    appendBits(bits, 0, 8);
    
    // 5. 对地速度 (10 bits)
    uint32_t sog = static_cast<uint32_t>(vesselData.speed * 10.0);
    if (sog > 1022) sog = 1023;
    appendBits(bits, sog, 10);
    
    // 6. 位置精度 (1 bit)
    appendBool(bits, true);
    
    // 7. 经度 (28 bits)
    int32_t lonValue = static_cast<int32_t>(vesselData.position.longitude() * 600000.0);
    appendSignedBits(bits, lonValue, 28);
    
    // 8. 纬度 (27 bits)
    int32_t latValue = static_cast<int32_t>(vesselData.position.latitude() * 600000.0);
    appendSignedBits(bits, latValue, 27);
    
    // 9. 对地航向 (12 bits)
    uint32_t cog = static_cast<uint32_t>(vesselData.courseOverGround * 10.0);
    if (cog >= 3600) cog = 3600;
    appendBits(bits, cog, 12);
    
    // 10. 真航向 (9 bits)
    uint32_t heading = static_cast<uint32_t>(vesselData.heading);
    if (heading > 359) heading = 511;
    appendBits(bits, heading, 9);
    
    // 11. 时间戳 (6 bits)
    QDateTime utc = QDateTime::currentDateTimeUtc();
    appendBits(bits, utc.time().second(), 6);
    
    // 12. 备用位 (2 bits)
    appendBits(bits, 0, 2);
    
    // 13. CS单元标志 (1 bit)
    appendBool(bits, false);
    
    // 14. 显示标志 (1 bit)
    appendBool(bits, true);
    
    // 15. DSC标志 (1 bit)
    appendBool(bits, true);
    
    // 16. 频带标志 (1 bit)
    appendBool(bits, false);
    
    // 17. 消息22标志 (1 bit)
    appendBool(bits, false);
    
    // 18. 分配模式标志 (1 bit)
    appendBool(bits, false);
    
    // 19. RAIM标志 (1 bit)
    appendBool(bits, false);
    
    // 20. 通信状态 (19 bits)
    appendBits(bits, 0, 19);
    
    std::string payload = encodeTo6bitAscii(bits);
    int fillBits = (6 - (bits.size() % 6)) % 6;
    
    return buildNmeaSentence(payload, fillBits);
}

// 生成Type 19消息：扩展B类位置报告
std::string AISMessageGenerator::generateType19Message(const AISVesselData& vesselData)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits)
    appendBits(bits, 19, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vesselData.mmsi, 30);
    
    // 4. 备用位 (8 bits)
    appendBits(bits, 0, 8);
    
    // 5. 对地速度 (10 bits)
    uint32_t sog = static_cast<uint32_t>(vesselData.speed * 10.0);
    if (sog > 1022) sog = 1023;
    appendBits(bits, sog, 10);
    
    // 6. 位置精度 (1 bit)
    appendBool(bits, true);
    
    // 7. 经度 (28 bits)
    int32_t lonValue = static_cast<int32_t>(vesselData.position.longitude() * 600000.0);
    appendSignedBits(bits, lonValue, 28);
    
    // 8. 纬度 (27 bits)
    int32_t latValue = static_cast<int32_t>(vesselData.position.latitude() * 600000.0);
    appendSignedBits(bits, latValue, 27);
    
    // 9. 对地航向 (12 bits)
    uint32_t cog = static_cast<uint32_t>(vesselData.courseOverGround * 10.0);
    if (cog >= 3600) cog = 3600;
    appendBits(bits, cog, 12);
    
    // 10. 真航向 (9 bits)
    uint32_t heading = static_cast<uint32_t>(vesselData.heading);
    if (heading > 359) heading = 511;
    appendBits(bits, heading, 9);
    
    // 11. 时间戳 (6 bits)
    QDateTime utc = QDateTime::currentDateTimeUtc();
    appendBits(bits, utc.time().second(), 6);
    
    // 12. 备用位 (4 bits)
    appendBits(bits, 0, 4);
    
    // 13. 船名 (120 bits = 20字符)
    appendString(bits, vesselData.vesselName, 120);
    
    // 14. 船舶类型 (8 bits)
    appendBits(bits, vesselData.shipType, 8);
    
    // 15. 船舶尺寸 (30 bits)
    int dimensionToBow = vesselData.length / 2;
    int dimensionToStern = vesselData.length - dimensionToBow;
    int dimensionToPort = vesselData.width / 2;
    int dimensionToStarboard = vesselData.width - dimensionToPort;
    
    appendBits(bits, dimensionToBow, 9);
    appendBits(bits, dimensionToStern, 9);
    appendBits(bits, dimensionToPort, 6);
    appendBits(bits, dimensionToStarboard, 6);
    
    // 16. 定位设备类型 (4 bits)
    appendBits(bits, 1, 4);
    
    // 17. RAIM标志 (1 bit)
    appendBool(bits, false);
    
    // 18. DTE标志 (1 bit)
    appendBool(bits, true);
    
    // 19. 分配模式标志 (1 bit)
    appendBool(bits, false);
    
    // 20. 备用位 (4 bits)
    appendBits(bits, 0, 4);
    
    std::string payload = encodeTo6bitAscii(bits);
    int fillBits = (6 - (bits.size() % 6)) % 6;
    
    return buildNmeaSentence(payload, fillBits);
}

// 生成Type 24消息：静态数据报告
std::string AISMessageGenerator::generateType24Message(const AISVesselData& vesselData, int partNumber)
{
    std::vector<bool> bits;
    
    // 1. 消息类型 (6 bits)
    appendBits(bits, 24, 6);
    
    // 2. 重复指示符 (2 bits)
    appendBits(bits, 0, 2);
    
    // 3. MMSI (30 bits)
    appendBits(bits, vesselData.mmsi, 30);
    
    // 4. 部分号 (2 bits)
    appendBits(bits, partNumber, 2);
    
    if (partNumber == 0) {
        // Part A: 船名
        // 5. 船名 (120 bits = 20字符)
        appendString(bits, vesselData.vesselName, 120);
        
        // 6. 备用位 (8 bits)
        appendBits(bits, 0, 8);
    } else {
        // Part B: 船舶类型和尺寸
        // 5. 船舶类型 (8 bits)
        appendBits(bits, vesselData.shipType, 8);
        
        // 6. 供应商ID (42 bits = 7字符)
        appendString(bits, "VENDOR1", 42);
        
        // 7. 呼号 (42 bits = 7字符)
        appendString(bits, vesselData.callSign, 42);
        
        // 8. 船舶尺寸 (30 bits)
        int dimensionToBow = vesselData.length / 2;
        int dimensionToStern = vesselData.length - dimensionToBow;
        int dimensionToPort = vesselData.width / 2;
        int dimensionToStarboard = vesselData.width - dimensionToPort;
        
        appendBits(bits, dimensionToBow, 9);
        appendBits(bits, dimensionToStern, 9);
        appendBits(bits, dimensionToPort, 6);
        appendBits(bits, dimensionToStarboard, 6);
        
        // 9. 备用位 (6 bits)
        appendBits(bits, 0, 6);
    }
    
    std::string payload = encodeTo6bitAscii(bits);
    int fillBits = (6 - (bits.size() % 6)) % 6;
    
    return buildNmeaSentence(payload, fillBits);
}