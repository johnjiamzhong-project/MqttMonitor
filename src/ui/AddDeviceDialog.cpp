#include "AddDeviceDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>

AddDeviceDialog::AddDeviceDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("AddDeviceDialog");
    setWindowTitle("添加设备");
    setFixedWidth(360);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setContentsMargins(0, 0, 0, 0);

    idEdit_   = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    idEdit_->setPlaceholderText("如：device001");
    nameEdit_->setPlaceholderText("如：温控器-1号（可留空）");
    form->addRow("设备 ID *", idEdit_);
    form->addRow("设备名称",  nameEdit_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setObjectName("dialogButtonBox");
    buttons->button(QDialogButtonBox::Ok)->setObjectName("okBtn");
    buttons->button(QDialogButtonBox::Cancel)->setObjectName("cancelBtn");
    buttons->button(QDialogButtonBox::Ok)->setText("确定");
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(16);
    root->addLayout(form);
    root->addSpacing(4);
    root->addWidget(buttons);
}

QString AddDeviceDialog::deviceId() const   { return idEdit_->text().trimmed(); }
QString AddDeviceDialog::deviceName() const { return nameEdit_->text().trimmed(); }
