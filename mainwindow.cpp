#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSplitter>
#include <QWidget>
#include <QHBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Layout ---
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    configPanel_ = new ConfigPanel(splitter);
    configPanel_->setFixedWidth(300);

    // Placeholder for DeviceGridView (Phase 2 next step)
    auto* rightPanel = new QWidget(splitter);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    // --- Signals ---
    connect(configPanel_, &ConfigPanel::connectRequested,
            this, &MainWindow::onConnectRequested);
    connect(configPanel_, &ConfigPanel::disconnectRequested,
            this, &MainWindow::onDisconnectRequested);
}

MainWindow::~MainWindow()
{
    // Destroy bridge before client (bridge holds a reference to client)
    mqttBridge_.reset();
    mqttClient_.reset();
    delete ui;
}

void MainWindow::onConnectRequested(const QString& brokerUrl,
                                    const QString& clientId,
                                    const QString& username,
                                    const QString& password,
                                    const QString& topic)
{
    // Re-create client and bridge on each new connection
    mqttBridge_.reset();
    mqttClient_ = std::make_unique<MqttClient>(brokerUrl.toStdString(),
                                               clientId.toStdString());

    const bool ok = mqttClient_->connect(username.toStdString(),
                                         password.toStdString());
    if (!ok) {
        ui->statusbar->showMessage("Connection failed: " + brokerUrl, 5000);
        mqttClient_.reset();
        return;
    }

    mqttClient_->subscribe(topic.toStdString());

    mqttBridge_ = std::make_unique<MqttBridge>(*mqttClient_);
    connect(mqttBridge_.get(), &MqttBridge::messageReceived,
            this, &MainWindow::onMessageReceived);

    configPanel_->onConnected();
    ui->statusbar->showMessage("Connected: " + brokerUrl);
}

void MainWindow::onDisconnectRequested()
{
    mqttBridge_.reset();
    if (mqttClient_) {
        mqttClient_->disconnect();
        mqttClient_.reset();
    }
    configPanel_->onDisconnected();
    ui->statusbar->showMessage("Disconnected", 3000);
}

void MainWindow::onMessageReceived(const QString& topic, const QString& payload)
{
    // Placeholder until DeviceGridView is ready
    qDebug() << "[MQTT]" << topic << "->" << payload;
}
