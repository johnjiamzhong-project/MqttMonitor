#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#ifdef Q_OS_WIN
#  include <windows.h>
#  include <dwmapi.h>
#  ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#    define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#  endif
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("MqttMonitor");

#ifdef Q_OS_WIN
    // 通知 DWM 对本窗口使用暗色标题栏（Win10 18985+ / Win11）
    HWND hwnd = reinterpret_cast<HWND>(winId());
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
#endif

    // --- Nav bar ---
    configBtn_ = new QPushButton("配置");
    deviceBtn_ = new QPushButton("设备");

    configBtn_->setCheckable(true);
    deviceBtn_->setCheckable(true);
    configBtn_->setChecked(true);   // default page

    // 互斥分组，保证同一时刻只有一个按钮处于 checked 状态
    auto* navGroup = new QButtonGroup(this);
    navGroup->setExclusive(true);
    navGroup->addButton(configBtn_);
    navGroup->addButton(deviceBtn_);

    auto* navBar    = new QWidget(this);
    navBar->setObjectName("navBar");   // QSS selector: QWidget#navBar
    auto* navLayout = new QVBoxLayout(navBar);
    navLayout->setContentsMargins(6, 10, 6, 10);
    navLayout->setSpacing(4);
    navLayout->addWidget(configBtn_);
    navLayout->addWidget(deviceBtn_);
    navLayout->addStretch();
    navBar->setFixedWidth(76);

    // --- Pages ---
    configPanel_ = new ConfigPanel;
    deviceView_  = new DeviceView;

    stack_ = new QStackedWidget(this);
    stack_->addWidget(configPanel_);   // index 0
    stack_->addWidget(deviceView_);    // index 1

    // --- Main layout ---
    auto* container = new QWidget(this);
    auto* hLayout   = new QHBoxLayout(container);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
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
    connect(mqttBridge_.get(), &MqttBridge::messageReceived,
            deviceView_, &DeviceView::updateDevice);
    connect(deviceView_, &DeviceView::publishRequested,
            this, &MainWindow::onPublishRequested);

    configPanel_->onConnected();
    ui->statusbar->showMessage("Connected: " + brokerUrl);
}

void MainWindow::onPublishRequested(const QString& topic, const QString& payload, int qos)
{
    if (mqttClient_ && mqttClient_->isConnected())
        mqttClient_->publish(topic.toStdString(), payload.toStdString(), qos);
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
