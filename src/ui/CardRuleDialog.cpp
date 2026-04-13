#include "CardRuleDialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

CardRuleDialog::CardRuleDialog(const CardRuleConfig& current, QWidget* parent)
    : QDialog(parent)
{
    setObjectName("CardRuleDialog");
    setWindowTitle("卡片规则");
    setFixedWidth(360);

    deviceIdEdit_    = new QLineEdit(current.fieldDeviceId,       this);
    nameEdit_        = new QLineEdit(current.fieldName,           this);
    statusEdit_      = new QLineEdit(current.fieldStatus,         this);
    dataEdit_        = new QLineEdit(current.fieldData,           this);
    onlineValueEdit_ = new QLineEdit(current.statusOnlineValue,   this);
    offlineValueEdit_= new QLineEdit(current.statusOfflineValue,  this);

    auto* form = new QFormLayout;
    form->addRow("设备 ID 字段:", deviceIdEdit_);
    form->addRow("名称字段:",     nameEdit_);
    form->addRow("状态字段:",     statusEdit_);
    form->addRow("数据字段:",     dataEdit_);
    form->addRow("在线值:",       onlineValueEdit_);
    form->addRow("离线值:",       offlineValueEdit_);

    auto* hint = new QLabel("提示：修改后仅对新收到的消息生效，已有卡片不变。", this);
    hint->setWordWrap(true);
    hint->setObjectName("hintLabel");

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(hint);
    root->addSpacing(4);
    root->addWidget(buttons);
}

CardRuleConfig CardRuleDialog::result() const
{
    CardRuleConfig cfg;
    auto take = [](QLineEdit* e, QString& dest) {
        const QString v = e->text().trimmed();
        if (!v.isEmpty()) dest = v;
    };
    take(deviceIdEdit_,    cfg.fieldDeviceId);
    take(nameEdit_,        cfg.fieldName);
    take(statusEdit_,      cfg.fieldStatus);
    take(dataEdit_,        cfg.fieldData);
    take(onlineValueEdit_, cfg.statusOnlineValue);
    take(offlineValueEdit_,cfg.statusOfflineValue);
    return cfg;
}
