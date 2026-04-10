#include "ConfigPanel.h"
#include "ui_ConfigPanel.h"

ConfigPanel::ConfigPanel(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigPanel)
{
    ui->setupUi(this);
    connect(ui->connectBtn,    &QPushButton::clicked, this, &ConfigPanel::onConnectClicked);
    connect(ui->disconnectBtn, &QPushButton::clicked, this, &ConfigPanel::disconnectRequested);
}

ConfigPanel::~ConfigPanel()
{
    delete ui;
}

void ConfigPanel::onConnectClicked()
{
    ui->statusLabel->clear();

    const QString broker = ui->brokerEdit->text().trimmed();
    const QString clientId = ui->clientIdEdit->text().trimmed();
    const QString topic = ui->topicEdit->text().trimmed();

    if (broker.isEmpty()) {
        ui->statusLabel->setText("Broker URL cannot be empty.");
        return;
    }
    if (clientId.isEmpty()) {
        ui->statusLabel->setText("Client ID cannot be empty.");
        return;
    }
    if (topic.isEmpty()) {
        ui->statusLabel->setText("Topic cannot be empty.");
        return;
    }

    emit connectRequested(
        broker,
        clientId,
        ui->usernameEdit->text(),
        ui->passwordEdit->text(),
        topic
    );
}

// Called by MainWindow after MqttClient::connect() succeeds
void ConfigPanel::onConnected()
{
    ui->statusLabel->clear();
    ui->connectBtn->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);
    ui->brokerEdit->setEnabled(false);
    ui->clientIdEdit->setEnabled(false);
}

// Called by MainWindow after MqttClient::disconnect() or on connection lost
void ConfigPanel::onDisconnected()
{
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    ui->brokerEdit->setEnabled(true);
    ui->clientIdEdit->setEnabled(true);
}
