#include "AddDeviceDialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

AddDeviceDialog::AddDeviceDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("添加设备");
    setFixedWidth(320);

    auto* form = new QFormLayout;
    idEdit_   = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    idEdit_->setPlaceholderText("如：device001");
    nameEdit_->setPlaceholderText("如：温控器-1号（可留空）");
    form->addRow("设备 ID *", idEdit_);
    form->addRow("设备名称",  nameEdit_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);
}

QString AddDeviceDialog::deviceId() const   { return idEdit_->text().trimmed(); }
QString AddDeviceDialog::deviceName() const { return nameEdit_->text().trimmed(); }
