#include "DeviceView.h"
#include "DeviceCard.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QFrame>
#include <QJsonDocument>
#include <QJsonObject>

static constexpr int kColumns = 3;

DeviceView::DeviceView(QWidget* parent)
    : QWidget(parent)
{
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* content = new QWidget;
    grid_ = new QGridLayout(content);
    grid_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    grid_->setSpacing(12);
    grid_->setContentsMargins(12, 12, 12, 12);

    scroll->setWidget(content);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(scroll);
}

void DeviceView::updateDevice(const QString& /*topic*/, const QString& payload)
{
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (!doc.isObject())
        return;

    const QJsonObject obj = doc.object();
    const QString id = obj["device_id"].toString();
    if (id.isEmpty())
        return;

    DeviceCard* card = cards_.value(id, nullptr);
    if (!card) {
        card = new DeviceCard(id, grid_->parentWidget());
        cards_[id] = card;
        int pos = cards_.size() - 1;
        grid_->addWidget(card, pos / kColumns, pos % kColumns);
    }

    card->update(obj);
}
