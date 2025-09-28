#include "ipc_client_manager.h"
#include <QHostAddress>

IPCClientManager::IPCClientManager(QObject *parent)
    : QObject(parent)
    , socket(nullptr)
    , autoReconnect(false)
{
    socket = new QTcpSocket(this);
    
    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(3000);
    reconnectTimer->setSingleShot(true);
    
    connect(socket, &QTcpSocket::connected, this, &IPCClientManager::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &IPCClientManager::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &IPCClientManager::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &IPCClientManager::onErrorOccurred);
    connect(reconnectTimer, &QTimer::timeout, this, &IPCClientManager::onReconnectTimeout);
}

IPCClientManager::~IPCClientManager()
{
    disconnectFromServer();
}

bool IPCClientManager::connectToServer(const QString &host, quint16 port)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        disconnectFromServer();
    }
    
    serverHost = host;
    serverPort = port;
    
    socket->connectToHost(host, port);
    return true;
}

void IPCClientManager::disconnectFromServer()
{
    reconnectTimer->stop();
    socket->disconnectFromHost();
}

bool IPCClientManager::isConnected() const
{
    return socket->state() == QAbstractSocket::ConnectedState;
}

void IPCClientManager::getServiceStatus()
{
    QJsonObject command;
    command["type"] = static_cast<int>(ais::protocol::CommandType::GET_STATUS);
    command["sequence"] = QDateTime::currentSecsSinceEpoch();
    sendCommand(command);
}

void IPCClientManager::startService()
{
    QJsonObject command;
    command["type"] = static_cast<int>(ais::protocol::CommandType::START_SERVICE);
    command["sequence"] = QDateTime::currentSecsSinceEpoch();
    sendCommand(command);
}

void IPCClientManager::stopService()
{
    QJsonObject command;
    command["type"] = static_cast<int>(ais::protocol::CommandType::STOP_SERVICE);
    command["sequence"] = QDateTime::currentSecsSinceEpoch();
    sendCommand(command);
}

void IPCClientManager::updateConfig(const QJsonObject &config)
{
    QJsonObject command;
    command["type"] = static_cast<int>(ais::protocol::CommandType::CONFIG_UPDATE);
    command["sequence"] = QDateTime::currentSecsSinceEpoch();
    command["data"] = QJsonDocument(config).toJson(QJsonDocument::Compact);
    sendCommand(command);
}

void IPCClientManager::onConnected()
{
    autoReconnect = true;
    emit connected();
}

void IPCClientManager::onDisconnected()
{
    emit disconnected();
    
    if (autoReconnect) {
        reconnectTimer->start();
    }
}

void IPCClientManager::onReadyRead()
{
    while (socket->canReadLine()) {
        QByteArray message = socket->readLine().trimmed();
        if (!message.isEmpty()) {
            processMessage(message);
        }
    }
}

void IPCClientManager::onErrorOccurred(QAbstractSocket::SocketError error)
{
    emit errorOccurred(socket->errorString());
    
    if (error != QAbstractSocket::RemoteHostClosedError && autoReconnect) {
        reconnectTimer->start();
    }
}

void IPCClientManager::onReconnectTimeout()
{
    if (autoReconnect) {
        socket->connectToHost(serverHost, serverPort);
    }
}

void IPCClientManager::processMessage(const QByteArray &message)
{
    try {
        QJsonDocument doc = QJsonDocument::fromJson(message);
        if (doc.isNull()) {
            emit messageReceived(QString::fromUtf8(message));
            return;
        }
        
        QJsonObject json = doc.object();
        QString type = json.contains("type") ? json["type"].toString() : "";
        
        if (json.contains("status")) {
            // 这是响应消息
            int status = json["status"].toInt();
            QJsonObject data = QJsonDocument::fromJson(json["data"].toString().toUtf8()).object();
            
            switch (static_cast<ais::protocol::ResponseStatus>(status)) {
            case ais::protocol::ResponseStatus::SUCCESS:
                if (data.contains("is_running")) {
                    handleServiceStatus(data);
                } else if (data.contains("config")) {
                    handleConfigResponse(data);
                }
                break;
            default:
                emit errorOccurred("命令执行失败");
                break;
            }
        } else {
            // 这是命令消息（服务端主动推送）
            emit messageReceived(QString::fromUtf8(message));
        }
    } catch (...) {
        emit messageReceived(QString::fromUtf8(message));
    }
}

void IPCClientManager::sendCommand(const QJsonObject &command)
{
    if (!isConnected()) {
        emit errorOccurred("未连接到服务");
        return;
    }
    
    QJsonDocument doc(command);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void IPCClientManager::handleServiceStatus(const QJsonObject &data)
{
    bool running = data["is_running"].toBool();
    emit serviceStateChanged(running);
}

void IPCClientManager::handleConfigResponse(const QJsonObject &data)
{
    emit configUpdated(data);
}