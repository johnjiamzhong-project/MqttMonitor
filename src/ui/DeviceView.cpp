#include "DeviceView.h"

#include <QVBoxLayout>
#include <QLabel>

DeviceView::DeviceView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* label  = new QLabel("设备视图（待实现）", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
