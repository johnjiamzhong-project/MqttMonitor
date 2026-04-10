#pragma once

#include <QMainWindow>
#include <memory>

#include "src/core/MqttClient.h"
#include "src/bridge/MqttBridge.h"
#include "src/ui/ConfigPanel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectRequested(const QString& brokerUrl,
                            const QString& clientId,
                            const QString& username,
                            const QString& password,
                            const QString& topic);
    void onDisconnectRequested();
    void onMessageReceived(const QString& topic, const QString& payload);

private:
    Ui::MainWindow* ui;

    // Owned objects — order matters: bridge must be destroyed before client
    std::unique_ptr<MqttBridge>  mqttBridge_;
    std::unique_ptr<MqttClient>  mqttClient_;

    ConfigPanel* configPanel_;  // owned by Qt parent chain
};
