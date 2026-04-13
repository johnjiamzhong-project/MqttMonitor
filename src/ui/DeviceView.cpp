#include "DeviceView.h"
#include "DeviceCard.h"
#include "AddDeviceDialog.h"
#include "CardRuleDialog.h"
#include "../core/CardRuleStore.h"
#include "../core/CmdPresetStore.h"

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
#include <QJsonDocument>
#include <QJsonObject>

static constexpr int kColumns = 3;

DeviceView::DeviceView(QWidget* parent)
    : QWidget(parent)
{
    // --- CardRuleStore: load persisted field-name mappings ---
    ruleStore_ = new CardRuleStore(this);
    ruleStore_->load();
    cardRule_ = ruleStore_->config();

    // --- CmdPresetStore: load command presets + last used values ---
    presetStore_ = new CmdPresetStore(this);
    presetStore_->load();

    // --- Toolbar ---
    auto* addBtn  = new QPushButton("+ 添加设备", this);
    auto* ruleBtn = new QPushButton("卡片规则", this);
    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(addBtn);
    toolbar->addWidget(ruleBtn);
    toolbar->addStretch();
    connect(addBtn,  &QPushButton::clicked, this, &DeviceView::onAddDeviceClicked);
    connect(ruleBtn, &QPushButton::clicked, this, &DeviceView::onCardRuleClicked);

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

    // --- Root layout ---
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addLayout(toolbar);
    root->addWidget(scroll, 1);
    root->addWidget(cmdPanel_);
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
        else if (raw == cardRule_.statusOfflineValue)
            normalised["status"] = QStringLiteral("offline");
        else
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
