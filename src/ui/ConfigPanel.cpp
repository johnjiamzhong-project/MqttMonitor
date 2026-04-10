#include "ConfigPanel.h"
#include "ui_ConfigPanel.h"

#include <QFont>
#include <QDateTime>

ConfigPanel::ConfigPanel(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigPanel)
{
    ui->setupUi(this);

    // Splitter appearance
    ui->splitter->setHandleWidth(8);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->formContainer->setFixedWidth(300);

    // Message view
    ui->messageEdit->setFont(QFont("Consolas", 9));
    ui->messageEdit->setMaximumBlockCount(1000);  // ~500 messages x 2 lines

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

    const QString broker   = ui->brokerEdit->text().trimmed();
    const QString clientId = ui->clientIdEdit->text().trimmed();
    const QString topic    = ui->topicEdit->text().trimmed();

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

void ConfigPanel::onConnected()
{
    ui->statusLabel->clear();
    ui->connectBtn->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);
    ui->brokerEdit->setEnabled(false);
    ui->clientIdEdit->setEnabled(false);
}

void ConfigPanel::onDisconnected()
{
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    ui->brokerEdit->setEnabled(true);
    ui->clientIdEdit->setEnabled(true);
}

void ConfigPanel::appendMessage(const QString& topic, const QString& payload)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->messageEdit->appendPlainText(
        QString("[%1]  topic:[%2]\n%3").arg(ts, topic, payload.simplified()));
}
