#ifndef SERVICE_CONTROL_PANEL_H
#define SERVICE_CONTROL_PANEL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

#include "ipc_client_manager.h"

class ServiceControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceControlPanel(IPCClientManager *ipcManager, QWidget *parent = nullptr);

private slots:
    void onServiceStateChanged(bool running);
    void onConnectionStateChanged(bool connected);
    void onStartService();
    void onStopService();
    void onRefreshStatus();

private:
    void createUI();
    void updateUI();

    IPCClientManager *ipcManager;
    
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *refreshButton;
    QLabel *statusLabel;
    QLabel *connectionLabel;
    
    bool serviceRunning;
    bool connectedToService;
};

#endif // SERVICE_CONTROL_PANEL_H