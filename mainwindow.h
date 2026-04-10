#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <memory>

#include "src/core/MqttClient.h"
#include "src/bridge/MqttBridge.h"
#include "src/ui/ConfigPanel.h"
#include "src/ui/DeviceView.h"

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

private:
    Ui::MainWindow* ui;

    // Owned objects — order matters: bridge must be destroyed before client
    std::unique_ptr<MqttBridge> mqttBridge_;
    std::unique_ptr<MqttClient> mqttClient_;

    QStackedWidget* stack_;
    ConfigPanel*    configPanel_;   // owned by Qt parent chain
    DeviceView*     deviceView_;    // owned by Qt parent chain

    QPushButton* configBtn_;
    QPushButton* deviceBtn_;
};
