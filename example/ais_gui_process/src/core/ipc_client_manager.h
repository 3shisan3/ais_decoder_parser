#ifndef IPC_CLIENT_MANAGER_H
#define IPC_CLIENT_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "ais_protocol.h"

class IPCClientManager : public QObject
{
    Q_OBJECT

public:
    explicit IPCClientManager(QObject *parent = nullptr);
    ~IPCClientManager();

    bool connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    // 服务控制命令
    void getServiceStatus();
    void startService();
    void stopService();
    void updateConfig(const QJsonObject &config);

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);
    void serviceStateChanged(bool running);
    void configUpdated(const QJsonObject &config);
    void errorOccurred(const QString &error);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReconnectTimeout();

private:
    void processMessage(const QByteArray &message);
    void sendCommand(const QJsonObject &command);
    void handleServiceStatus(const QJsonObject &data);
    void handleConfigResponse(const QJsonObject &data);

    QTcpSocket *socket;
    QTimer *reconnectTimer;
    QString serverHost;
    quint16 serverPort;
    bool autoReconnect;
};

#endif // IPC_CLIENT_MANAGER_H