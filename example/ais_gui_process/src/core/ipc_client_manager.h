#ifndef IPC_CLIENT_MANAGER_H
#define IPC_CLIENT_MANAGER_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdint>

#include "core/tcp_ipc_client.h"
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
    void sendCustomCommand(const QString &command, const QVariantMap &params = QVariantMap());

    // 获取解析器管理器
    AISParserManager* getParserManager() const;

signals:
    void connectionStateChanged(bool connected);
    void messageReceived(const QString &message);           // 格式化后的消息
    void rawMessageReceived(const QString &message);        // 原始数据消息
    void serviceStateChanged(bool running);
    void serviceConfigReceived(const QVariantMap &config);
    void errorOccurred(const QString &errorMessage);
    void aisMessageReceived(const QString &rawData, const QString &processedData);
    void rawAisMessageReceived(const QString &rawData);     // 原始AIS数据

private slots:
    void onReconnectTimeout();

private:
    void initializeClient();
    QVariantMap parseMessage(const QString &message);
    void sendProtocolCommand(const ais::protocol::CommandMessage &cmd);
    void startReconnectTimer();
    void stopReconnectTimer();
    uint32_t generateSequence();
    
    // 消息处理辅助方法
    void handleAISMessage(const ais::protocol::CommandMessage &cmd);
    void handleConfigUpdate(const ais::protocol::CommandMessage &cmd);

    ais::AISClient *ipcClient;
    AISParserManager *parserManager;
    
    QString m_serverAddress;
    quint16 m_serverPort;
    bool m_connected;
    bool m_autoReconnect;
    
    QTimer *reconnectTimer;
    int reconnectAttempts;
};

#endif // IPC_CLIENT_MANAGER_H