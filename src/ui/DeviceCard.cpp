#include "DeviceCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QJsonObject>
#include <QJsonValue>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

DeviceCard::DeviceCard(const QString& deviceId, QWidget* parent)
    : QFrame(parent)
    , deviceId_(deviceId)
{
    setFrameShape(QFrame::StyledPanel);
    setObjectName("DeviceCard");   // QSS: QFrame#DeviceCard
    setFixedSize(220, 160);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    // --- top row: name + status badge ---
    auto* topRow = new QHBoxLayout;
    topRow->setSpacing(6);

    nameLabel_ = new QLabel(deviceId, this);
    QFont f = nameLabel_->font();
    f.setBold(true);
    nameLabel_->setFont(f);
    nameLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    statusLabel_ = new QLabel("未知", this);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setFixedSize(48, 20);
    statusLabel_->setStyleSheet("background:#d4a040;color:#1e1e2e;border-radius:3px;font-weight:bold;");

    topRow->addWidget(nameLabel_);
    topRow->addWidget(statusLabel_);
    root->addLayout(topRow);

    // --- separator ---
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    root->addWidget(sep);

    // --- data KV area ---
    dataLayout_ = new QFormLayout;
    dataLayout_->setSpacing(2);
    dataLayout_->setContentsMargins(0, 0, 0, 0);
    root->addLayout(dataLayout_);

    root->addStretch();
}

void DeviceCard::update(const QJsonObject& obj)
{
    // name
    if (obj.contains("name") && !obj["name"].toString().isEmpty())
        nameLabel_->setText(obj["name"].toString());

    // status
    if (obj.contains("status"))
        setStatus(obj["status"].toString());

    // data KV — rebuild rows
    while (dataLayout_->rowCount() > 0)
        dataLayout_->removeRow(0);

    if (obj.contains("data") && obj["data"].isObject()) {
        const QJsonObject data = obj["data"].toObject();
        for (auto it = data.begin(); it != data.end(); ++it) {
            QString val = it.value().isString()
                ? it.value().toString()
                : QString::number(it.value().toDouble());
            dataLayout_->addRow(it.key() + ":", new QLabel(val, this));
        }
    }
}

void DeviceCard::setSelected(bool selected)
{
    selected_ = selected;
    updateSelectionStyle();
    emit selectionChanged(deviceId_, selected_);
}

void DeviceCard::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        selected_ = !selected_;
        updateSelectionStyle();
        emit selectionChanged(deviceId_, selected_);
    }
    QFrame::mousePressEvent(event);
}

void DeviceCard::updateSelectionStyle()
{
    if (selected_)
        setStyleSheet("QFrame#DeviceCard { border: 2px solid #89b4fa; background-color: #2a2a45; border-radius: 6px; }");
    else
        setStyleSheet("");  // 回退到全局 QSS 中 QFrame#DeviceCard 的默认样式
}

void DeviceCard::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    QAction* del = menu.addAction("删除设备");
    if (menu.exec(event->globalPos()) == del)
        emit removeRequested(deviceId_);
}

void DeviceCard::setStatus(const QString& status)
{
    if (status == "online") {
        statusLabel_->setText("在线");
        statusLabel_->setStyleSheet("background:#40a870;color:#1e1e2e;border-radius:3px;font-weight:bold;");
    } else if (status == "offline") {
        statusLabel_->setText("离线");
        statusLabel_->setStyleSheet("background:#e05c6e;color:#1e1e2e;border-radius:3px;font-weight:bold;");
    } else {
        statusLabel_->setText("未知");
        statusLabel_->setStyleSheet("background:#d4a040;color:#1e1e2e;border-radius:3px;font-weight:bold;");
    }
}
