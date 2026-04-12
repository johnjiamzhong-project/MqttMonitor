#include "ConfigPanel.h"
#include "ui_ConfigPanel.h"

#include <QFont>
#include <QDateTime>
#include <QInputDialog>
#include <QMessageBox>

ConfigPanel::ConfigPanel(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigPanel), store_(new ConfigStore(this))
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

    connect(ui->connectBtn,      &QPushButton::clicked, this, &ConfigPanel::onConnectClicked);
    connect(ui->disconnectBtn,   &QPushButton::clicked, this, &ConfigPanel::disconnectRequested);
    connect(ui->saveProfileBtn,  &QPushButton::clicked, this, &ConfigPanel::onSaveProfileClicked);
    connect(ui->deleteProfileBtn,&QPushButton::clicked, this, &ConfigPanel::onDeleteProfileClicked);
    connect(ui->profileCombo,    &QComboBox::currentTextChanged,
            this, &ConfigPanel::onProfileSelected);

    // Load saved profiles and restore last used
    store_->load();
    populateProfileCombo();
    const QString last = store_->lastProfile();
    if (!last.isEmpty())
        loadProfile(last);
}

ConfigPanel::~ConfigPanel()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// Profile helpers
// ---------------------------------------------------------------------------

void ConfigPanel::populateProfileCombo()
{
    QSignalBlocker blocker(ui->profileCombo);  // suppress currentTextChanged during rebuild
    ui->profileCombo->clear();
    ui->profileCombo->addItem("-- 新建配置 --");
    for (const QString& name : store_->profileNames())
        ui->profileCombo->addItem(name);
}

void ConfigPanel::loadProfile(const QString& name)
{
    const MqttProfile p = store_->findByName(name);
    if (p.name.isEmpty())
        return;

    ui->brokerEdit->setText(p.broker);
    ui->clientIdEdit->setText(p.clientId);
    ui->usernameEdit->setText(p.username);
    ui->passwordEdit->setText(p.password);
    ui->topicEdit->setText(p.topic);

    // Sync combo selection without re-triggering the slot
    QSignalBlocker blocker(ui->profileCombo);
    const int idx = ui->profileCombo->findText(name);
    if (idx >= 0)
        ui->profileCombo->setCurrentIndex(idx);

    ui->deleteProfileBtn->setEnabled(true);
}

// ---------------------------------------------------------------------------
// Slots — profile management
// ---------------------------------------------------------------------------

void ConfigPanel::onProfileSelected(const QString& name)
{
    const bool isNew = (name == "-- 新建配置 --");
    ui->deleteProfileBtn->setEnabled(!isNew);

    if (!isNew) {
        loadProfile(name);
        store_->setLastProfile(name);
        store_->save();
    }
}

void ConfigPanel::onSaveProfileClicked()
{
    QString name = ui->profileCombo->currentText();
    const bool isNew = (name == "-- 新建配置 --");

    if (isNew) {
        bool ok = false;
        name = QInputDialog::getText(this, "保存配置", "配置名称：",
                                     QLineEdit::Normal, QString(), &ok);
        if (!ok || name.trimmed().isEmpty())
            return;
        name = name.trimmed();
    }

    MqttProfile p;
    p.name     = name;
    p.broker   = ui->brokerEdit->text().trimmed();
    p.clientId = ui->clientIdEdit->text().trimmed();
    p.username = ui->usernameEdit->text();
    p.password = ui->passwordEdit->text();
    p.topic    = ui->topicEdit->text().trimmed();

    store_->addOrUpdate(p);
    store_->setLastProfile(name);
    store_->save();

    populateProfileCombo();

    QSignalBlocker blocker(ui->profileCombo);
    const int idx = ui->profileCombo->findText(name);
    if (idx >= 0)
        ui->profileCombo->setCurrentIndex(idx);
    ui->deleteProfileBtn->setEnabled(true);
}

void ConfigPanel::onDeleteProfileClicked()
{
    const QString name = ui->profileCombo->currentText();
    if (name == "-- 新建配置 --")
        return;

    const auto reply = QMessageBox::question(
        this, "删除配置",
        QString("确认删除配置 \"%1\" ？").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    store_->remove(name);
    store_->save();
    populateProfileCombo();

    QSignalBlocker blocker(ui->profileCombo);
    ui->profileCombo->setCurrentIndex(0);
    ui->deleteProfileBtn->setEnabled(false);
}

// ---------------------------------------------------------------------------
// Slots — connection
// ---------------------------------------------------------------------------

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
    ui->profileCombo->setEnabled(false);
    ui->deleteProfileBtn->setEnabled(false);
}

void ConfigPanel::onDisconnected()
{
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);
    ui->brokerEdit->setEnabled(true);
    ui->clientIdEdit->setEnabled(true);
    ui->profileCombo->setEnabled(true);
    // Restore delete button if a real profile is selected
    const bool hasProfile = (ui->profileCombo->currentText() != "-- 新建配置 --");
    ui->deleteProfileBtn->setEnabled(hasProfile);
}

void ConfigPanel::appendMessage(const QString& topic, const QString& payload)
{
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->messageEdit->appendPlainText(
        QString("[%1]  topic:[%2]\n%3").arg(ts, topic, payload.simplified()));
}
