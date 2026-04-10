#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- Nav bar ---
    configBtn_ = new QPushButton("配置");
    deviceBtn_ = new QPushButton("设备");

    auto* navBar    = new QWidget(this);
    auto* navLayout = new QVBoxLayout(navBar);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->addWidget(configBtn_);
    navLayout->addWidget(deviceBtn_);
    navLayout->addStretch();
    navBar->setFixedWidth(72);

    // --- Pages ---
    configPanel_ = new ConfigPanel;
    deviceView_  = new DeviceView;

    stack_ = new QStackedWidget(this);
    stack_->addWidget(configPanel_);   // index 0
    stack_->addWidget(deviceView_);    // index 1

    // --- Main layout ---
    auto* container = new QWidget(this);
    auto* hLayout   = new QHBoxLayout(container);
    hLayout->setContentsMargins(6, 6, 14, 6);
    hLayout->setSpacing(8);
    hLayout->addWidget(navBar);
    hLayout->addWidget(stack_);
    setCentralWidget(container);

    // --- Nav connections ---
    connect(configBtn_, &QPushButton::clicked, [this]{ stack_->setCurrentIndex(0); });
    connect(deviceBtn_, &QPushButton::clicked, [this]{ stack_->setCurrentIndex(1); });

    // --- ConfigPanel connections ---
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
            configPanel_, &ConfigPanel::appendMessage);

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
