#include "DeviceView.h"
#include "DeviceCard.h"
#include "AddDeviceDialog.h"
#include "CardRuleDialog.h"
#include "../core/CardRuleStore.h"
#include "../core/CmdPresetStore.h"
#include "../core/DisconnectDetectionConfig.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QSpinBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

static constexpr int kColumns = 3;

DeviceView::DeviceView(QWidget* parent)
    : QWidget(parent), timeoutMs_(30000)
{
    // --- CardRuleStore: load persisted field-name mappings ---
    ruleStore_ = new CardRuleStore(this);
    ruleStore_->load();
    cardRule_ = ruleStore_->config();

    // --- CmdPresetStore: load command presets + last used values ---
    presetStore_ = new CmdPresetStore(this);
    presetStore_->load();

    // --- DisconnectDetectionConfig: load detection settings ---
    detectionConfig_ = new DisconnectDetectionConfig;
    detectionConfig_->load();
    timeoutMs_ = (qint64)detectionConfig_->timeoutSeconds() * 1000;

    // --- Toolbar ---
    auto* addBtn  = new QPushButton("添加设备", this);
    addBtn->setObjectName("addDeviceBtn");
    auto* ruleBtn = new QPushButton("卡片规则", this);
    ruleBtn->setObjectName("cardRuleBtn");
    diagLabel_ = new QLabel("检测: 待启动", this);
    diagLabel_->setStyleSheet("color: #a6adc8; font-size: 11px;");

    // Disconnect detection controls
    detectionCheckBox_ = new QCheckBox("启用断联检测", this);
    detectionCheckBox_->setChecked(detectionConfig_->isEnabled());
    timeoutSpinBox_ = new QSpinBox(this);
    timeoutSpinBox_->setMinimum(1);
    timeoutSpinBox_->setMaximum(300);
    timeoutSpinBox_->setValue(detectionConfig_->timeoutSeconds());
    timeoutSpinBox_->setSuffix(" 秒");
    timeoutSpinBox_->setFixedWidth(100);

    auto* toolbar = new QHBoxLayout;
    toolbar->setSpacing(12);
    toolbar->addWidget(addBtn);
    toolbar->addWidget(ruleBtn);
    toolbar->addWidget(detectionCheckBox_);
    toolbar->addWidget(timeoutSpinBox_);
    toolbar->addStretch();
    toolbar->addWidget(diagLabel_);
    connect(addBtn,  &QPushButton::clicked, this, &DeviceView::onAddDeviceClicked);
    connect(ruleBtn, &QPushButton::clicked, this, &DeviceView::onCardRuleClicked);
    connect(detectionCheckBox_, &QCheckBox::toggled, this, &DeviceView::onDetectionToggled);
    connect(timeoutSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DeviceView::onTimeoutChanged);

    // --- Scroll area ---
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget;
    grid_ = new QGridLayout(content);
    grid_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    grid_->setSpacing(12);
    grid_->setContentsMargins(12, 12, 12, 12);
    scroll->setWidget(content);

    // --- Command panel (hidden until selection) ---
    cmdPanel_ = new QWidget(this);
    auto* cmdLayout = new QVBoxLayout(cmdPanel_);
    cmdLayout->setContentsMargins(8, 6, 8, 8);
    cmdLayout->setSpacing(6);

    // Preset row: dropdown + save + delete
    presetCombo_ = new QComboBox(this);
    presetCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    savePresetBtn_ = new QPushButton("保存", this);
    savePresetBtn_->setFixedWidth(52);
    delPresetBtn_ = new QPushButton("删除", this);
    delPresetBtn_->setFixedWidth(52);
    auto* presetRow = new QHBoxLayout;
    presetRow->addWidget(new QLabel("预设:", this));
    presetRow->addWidget(presetCombo_, 1);
    presetRow->addWidget(savePresetBtn_);
    presetRow->addWidget(delPresetBtn_);
    cmdLayout->addLayout(presetRow);

    // Top row: selection info + clear button
    selectionLabel_ = new QLabel(this);
    auto* clearBtn  = new QPushButton("取消选择", this);
    clearBtn->setFixedWidth(80);
    auto* infoRow = new QHBoxLayout;
    infoRow->addWidget(selectionLabel_);
    infoRow->addStretch();
    infoRow->addWidget(clearBtn);
    cmdLayout->addLayout(infoRow);

    // Separator
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    cmdLayout->addWidget(sep);

    // Controls row: Topic + QoS + Send
    topicEdit_ = new QLineEdit(this);
    topicEdit_->setPlaceholderText("目标 Topic，如 devices/{device_id}/cmd");

    qosCombo_ = new QComboBox(this);
    qosCombo_->addItems({"QoS 0", "QoS 1", "QoS 2"});
    qosCombo_->setFixedWidth(72);

    auto* sendBtn = new QPushButton("发送", this);
    sendBtn->setFixedWidth(56);

    auto* ctrlRow = new QHBoxLayout;
    ctrlRow->addWidget(new QLabel("Topic:", this));
    ctrlRow->addWidget(topicEdit_, 1);
    ctrlRow->addWidget(qosCombo_);
    ctrlRow->addWidget(sendBtn);
    cmdLayout->addLayout(ctrlRow);

    // Payload
    payloadEdit_ = new QPlainTextEdit(this);
    payloadEdit_->setPlaceholderText("Payload（JSON 或纯文本）");
    payloadEdit_->setFixedHeight(72);
    payloadEdit_->setFont(QFont("Consolas", 9));
    cmdLayout->addWidget(payloadEdit_);

    cmdPanel_->hide();

    // Restore last used values
    topicEdit_->setText(presetStore_->lastTopic());
    payloadEdit_->setPlainText(presetStore_->lastPayload());
    qosCombo_->setCurrentIndex(qBound(0, presetStore_->lastQos(), 2));

    // Populate preset combo
    refreshPresetCombo();

    connect(clearBtn,      &QPushButton::clicked,        this, &DeviceView::onClearSelectionClicked);
    connect(sendBtn,       &QPushButton::clicked,        this, &DeviceView::onSendClicked);
    connect(savePresetBtn_,&QPushButton::clicked,        this, &DeviceView::onSavePresetClicked);
    connect(delPresetBtn_, &QPushButton::clicked,        this, &DeviceView::onDelPresetClicked);
    connect(presetCombo_,  QOverload<int>::of(&QComboBox::activated),
                                                         this, &DeviceView::onPresetSelected);
    connect(topicEdit_,    &QLineEdit::textChanged,      this, &DeviceView::onCmdFieldChanged);
    connect(payloadEdit_,  &QPlainTextEdit::textChanged, this, &DeviceView::onCmdFieldChanged);
    connect(qosCombo_,     QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                         this, &DeviceView::onCmdFieldChanged);

    // --- Disconnect detection timer ---
    // 定时触发设备断联检测
    disconnectCheckTimer_ = new QTimer(this);
    disconnectCheckTimer_->setInterval(1000);
    connect(disconnectCheckTimer_, &QTimer::timeout, this, &DeviceView::checkDisconnectedDevices);

    // --- Root layout ---
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addLayout(toolbar);
    root->addWidget(scroll, 1);
    root->addWidget(cmdPanel_);

    // Auto-start detection if enabled in config
    if (detectionConfig_->isEnabled()) {
        startDisconnectDetection();
    }
}

void DeviceView::updateDevice(const QString& /*topic*/, const QString& payload)
{
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (!doc.isObject())
        return;

    const QJsonObject obj = doc.object();
    const QString id = obj[cardRule_.fieldDeviceId].toString();
    if (id.isEmpty())
        return;

    if (!cards_.contains(id))
        addCard(id, obj[cardRule_.fieldName].toString());

    // 更新设备的最后消息时间，并处理断联恢复逻辑
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    lastMsgTime_ = now;
    lastMsgId_ = id;
    if (deviceStates_.contains(id)) {
        DeviceState& state = deviceStates_[id];
        const qint64 lastTime = state.lastMessageTime;
        state.lastMessageTime = now;

        // 如果设备已断联，新消息到达时立即恢复
        if (state.isDisconnected) {
            markDeviceConnected(id);
        }

        // 首次消息或距上次消息超过 20 秒，加入待检查队列
        if (lastTime == 0 || (now - lastTime) > 20000) {
            checkQueue_.insert(id);
        }
    }

    // Translate user-configured field names / status values to internal fixed keys
    QJsonObject normalised;
    if (obj.contains(cardRule_.fieldName))
        normalised["name"] = obj[cardRule_.fieldName];
    if (obj.contains(cardRule_.fieldData))
        normalised["data"] = obj[cardRule_.fieldData];
    if (obj.contains(cardRule_.fieldStatus)) {
        const QString raw = obj[cardRule_.fieldStatus].toString();
        if (raw == cardRule_.statusOnlineValue)
            normalised["status"] = QStringLiteral("online");
        else if (raw == cardRule_.statusOfflineValue) {
            normalised["status"] = QStringLiteral("offline");
            // 消息中主动报告 offline，立即标记为断联
            markDeviceDisconnected(id);
        } else
            normalised["status"] = raw;
    }

    cards_[id]->update(normalised);
}

void DeviceView::onCardRuleClicked()
{
    CardRuleDialog dlg(cardRule_, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    cardRule_ = dlg.result();
    ruleStore_->setConfig(cardRule_);
    ruleStore_->save();
    // Existing cards are not retroactively updated
}

void DeviceView::onAddDeviceClicked()
{
    AddDeviceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString id   = dlg.deviceId();
    const QString name = dlg.deviceName();

    if (id.isEmpty() || cards_.contains(id))
        return;

    addCard(id, name.isEmpty() ? id : name);
}

void DeviceView::removeDevice(const QString& deviceId)
{
    DeviceCard* card = cards_.take(deviceId);
    if (!card)
        return;

    selectedIds_.remove(deviceId);
    deviceStates_.remove(deviceId);
    checkQueue_.remove(deviceId);
    updateCommandPanel();

    grid_->removeWidget(card);
    card->deleteLater();
    rebuildGrid();
}

void DeviceView::onCardSelectionChanged(const QString& deviceId, bool selected)
{
    if (selected)
        selectedIds_.insert(deviceId);
    else
        selectedIds_.remove(deviceId);

    updateCommandPanel();
}

void DeviceView::onSendClicked()
{
    const QString topicTpl = topicEdit_->text().trimmed();
    const QString payload  = payloadEdit_->toPlainText();
    const int     qos      = qosCombo_->currentIndex();

    if (topicTpl.isEmpty() || payload.isEmpty())
        return;

    for (const QString& id : std::as_const(selectedIds_)) {
        QString topic = topicTpl;
        topic.replace("{device_id}", id);
        emit publishRequested(topic, payload, qos);
    }
}

void DeviceView::refreshPresetCombo()
{
    presetCombo_->blockSignals(true);
    presetCombo_->clear();
    presetCombo_->addItem("— 选择预设 —");
    for (const CmdPreset& p : presetStore_->presets())
        presetCombo_->addItem(p.name);
    presetCombo_->setCurrentIndex(0);
    presetCombo_->blockSignals(false);
}

void DeviceView::onPresetSelected(int index)
{
    if (index <= 0) return;
    const QList<CmdPreset> list = presetStore_->presets();
    if (index - 1 >= list.size()) return;

    const CmdPreset& p = list[index - 1];
    topicEdit_->setText(p.topic);
    payloadEdit_->setPlainText(p.payload);
    qosCombo_->setCurrentIndex(qBound(0, p.qos, 2));
}

void DeviceView::onSavePresetClicked()
{
    const QString topic   = topicEdit_->text().trimmed();
    const QString payload = payloadEdit_->toPlainText();
    if (topic.isEmpty()) return;

    presetStore_->addPreset(topic, payload, qosCombo_->currentIndex());
    refreshPresetCombo();

    // Select the newly added item (last in list)
    presetCombo_->setCurrentIndex(presetCombo_->count() - 1);
}

void DeviceView::onDelPresetClicked()
{
    const int idx = presetCombo_->currentIndex();
    if (idx <= 0) return;

    const QString name = presetCombo_->currentText();
    presetStore_->removePreset(name);
    refreshPresetCombo();
}

void DeviceView::onCmdFieldChanged()
{
    presetStore_->saveLastUsed(
        topicEdit_->text().trimmed(),
        payloadEdit_->toPlainText(),
        qosCombo_->currentIndex());
}

void DeviceView::onClearSelectionClicked()
{
    for (const QString& id : std::as_const(selectedIds_)) {
        if (DeviceCard* card = cards_.value(id))
            card->setSelected(false);
    }
    // setSelected emits selectionChanged which clears selectedIds_ one by one,
    // but to avoid iteration-while-modifying, clear directly after the loop.
    selectedIds_.clear();
    updateCommandPanel();
}

void DeviceView::addCard(const QString& id, const QString& name)
{
    auto* card = new DeviceCard(id, grid_->parentWidget());
    if (!name.isEmpty() && name != id) {
        QJsonObject obj;
        obj["name"] = name;
        card->update(obj);
    }
    cards_[id] = card;

    DeviceState state;
    state.deviceId = id;
    state.lastMessageTime = QDateTime::currentMSecsSinceEpoch();
    state.isDisconnected = false;
    deviceStates_[id] = state;
    // 新设备立即进入检查队列，以便超时检测生效
    checkQueue_.insert(id);

    connect(card, &DeviceCard::removeRequested,   this, &DeviceView::removeDevice);
    connect(card, &DeviceCard::selectionChanged,  this, &DeviceView::onCardSelectionChanged);

    int pos = cards_.size() - 1;
    grid_->addWidget(card, pos / kColumns, pos % kColumns);
}

void DeviceView::rebuildGrid()
{
    const QList<DeviceCard*> list = cards_.values();
    for (DeviceCard* c : list)
        grid_->removeWidget(c);

    int pos = 0;
    for (DeviceCard* c : list) {
        grid_->addWidget(c, pos / kColumns, pos % kColumns);
        ++pos;
    }
}

void DeviceView::updateCommandPanel()
{
    const int n = selectedIds_.size();
    if (n == 0) {
        cmdPanel_->hide();
        return;
    }
    selectionLabel_->setText(QString("已选 %1 台设备").arg(n));
    cmdPanel_->show();
}

// 检测设备是否已超时断联
void DeviceView::checkDisconnectedDevices()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // 收集诊断信息：所有待检查设备的 elapsed 秒数
    QStringList diagParts;
    QStringList toDisconnect;

    const QSet<QString> queueSnapshot = checkQueue_;
    for (const QString& deviceId : queueSnapshot) {
        if (!deviceStates_.contains(deviceId))
            continue;

        DeviceState& state = deviceStates_[deviceId];
        qint64 elapsed = now - state.lastMessageTime;
        diagParts << QString("%1=%2s").arg(deviceId).arg(elapsed / 1000);

        if (elapsed > timeoutMs_ && !state.isDisconnected) {
            toDisconnect << deviceId;
        }
    }

    for (const QString& id : toDisconnect)
        markDeviceDisconnected(id);

    if (diagLabel_) {
        const QString t = QDateTime::currentDateTime().toString("hh:mm:ss");
        const qint64 msgAge = lastMsgTime_ > 0 ? (now - lastMsgTime_) / 1000 : -1;
        const QString msgInfo = lastMsgId_.isEmpty()
            ? QString("无消息")
            : QString("最后消息[%1]%2s前").arg(lastMsgId_).arg(msgAge);
        diagLabel_->setText(QString("检查[%1] 队列:%2 %3 | %4")
            .arg(t)
            .arg(queueSnapshot.size())
            .arg(diagParts.join(" "))
            .arg(msgInfo));
    }
}

// 将设备标记为已断联
void DeviceView::markDeviceDisconnected(const QString& deviceId)
{
    if (!deviceStates_.contains(deviceId))
        return;

    DeviceState& state = deviceStates_[deviceId];
    state.isDisconnected = true;

    if (DeviceCard* card = cards_.value(deviceId)) {
        card->setDisconnected(true);
    }

    checkQueue_.remove(deviceId);
}

// 将设备标记为已连接
void DeviceView::markDeviceConnected(const QString& deviceId)
{
    if (!deviceStates_.contains(deviceId))
        return;

    DeviceState& state = deviceStates_[deviceId];
    state.isDisconnected = false;

    if (DeviceCard* card = cards_.value(deviceId)) {
        card->setDisconnected(false);
    }
    // 设备恢复后重新加入检查队列
    checkQueue_.insert(deviceId);
}

// 启动断联检测定时器
void DeviceView::startDisconnectDetection()
{
    if (disconnectCheckTimer_ && !disconnectCheckTimer_->isActive()) {
        disconnectCheckTimer_->start();
    }
}

// 停止断联检测定时器
void DeviceView::stopDisconnectDetection()
{
    if (disconnectCheckTimer_ && disconnectCheckTimer_->isActive()) {
        disconnectCheckTimer_->stop();
    }
}

// 标记所有设备为断联（MQTT连接丢失时调用）
void DeviceView::markAllDevicesDisconnected()
{
    const QList<QString> ids = cards_.keys();
    for (const QString& deviceId : ids) {
        markDeviceDisconnected(deviceId);
    }
}

void DeviceView::onDetectionToggled(bool enabled)
{
    detectionConfig_->setEnabled(enabled);
    detectionConfig_->save();

    if (enabled) {
        startDisconnectDetection();
    } else {
        stopDisconnectDetection();
    }
}

void DeviceView::onTimeoutChanged(int seconds)
{
    timeoutMs_ = (qint64)seconds * 1000;
    detectionConfig_->setTimeoutSeconds(seconds);
    detectionConfig_->save();
}
