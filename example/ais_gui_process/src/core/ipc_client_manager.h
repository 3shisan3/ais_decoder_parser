#ifndef IPC_CLIENT_MANAGER_H
#define IPC_CLIENT_MANAGER_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdint>
#include <memory>

#include "tcp_client_session.h"
#include "core/ais_parser_manager.h"

class IPCClientManager : public QObject
{
    Q_OBJECT

public:
    explicit IPCClientManager(QObject *parent = nullptr);
    ~IPCClientManager();

    // 连接管理
    bool connectToServer();
    bool connectToServer(const QString &address, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    // 服务器配置
    void setServerAddress(const QString &address);
    void setServerPort(quint16 port);
    QString serverAddress() const;
    quint16 serverPort() const;

    // 服务控制命令 - 使用protocol定义的消息类型
    void sendStartCommand();
    void sendStopCommand();
    void getServiceStatus();
    void getServiceConfig();
    void updateServiceConfig(const QVariantMap &config);
    void getShipCount();
    void getMessageStats();
    void toggleServiceLogs();
    void sendHeartbeat();
    void sendCustomCommand(const QString &commandType, const QVariantMap &params = QVariantMap());

    // 获取解析器管理器
    AISParserManager* getParserManager() const;

    // 连接配置
    void setConnectTimeout(int timeoutMs);
    void enableAutoReconnect(bool enable, uint32_t intervalMs = 3000);

signals:
    void connectionStateChanged(bool connected);
    void messageReceived(const QString &message);           // 格式化后的消息
    void rawMessageReceived(const QString &message);        // 原始数据消息
    void serviceStateChanged(bool running);
    void serviceConfigReceived(const QVariantMap &config);
    void errorOccurred(const QString &errorMessage);
    void aisMessageReceived(const QString &rawData, const QString &processedData);
    void rawAisMessageReceived(const QString &rawData);     // 原始AIS数据
    void shipCountReceived(int count);                      // 船舶数量
    void messageStatsReceived(const QVariantMap &stats);    // 消息统计

private slots:
    void onReconnectTimeout();

private:
    void initializeClient();
    void cleanupClient();
    QVariantMap parseMessage(const QString &message);
    void sendProtocolCommand(const ais::protocol::CommandMessage &cmd);
    void startReconnectTimer();
    void stopReconnectTimer();
    uint32_t generateSequence();
    
    // 消息处理辅助方法
    void handleServerMessage(const ais::protocol::CommandMessage &cmd);
    void handleServerResponse(const ais::protocol::ResponseMessage &response);
    void handleConnectionError(const std::string &error);
    void handleAISMessage(const ais::protocol::CommandMessage &cmd);
    void handleConfigUpdate(const ais::protocol::CommandMessage &cmd);
    void handleSuccessResponse(const ais::protocol::ResponseMessage &response);
    void handleErrorResponse(const ais::protocol::ResponseMessage &response);

    std::unique_ptr<ais::TCPClientSession> clientSession_;
    AISParserManager *parserManager_;
    
    QString serverAddress_;
    quint16 serverPort_;
    bool connected_;
    bool autoReconnect_;
    
    QTimer *reconnectTimer_;
    int reconnectAttempts_;
};

#endif // IPC_CLIENT_MANAGER_H