#pragma once

#include <QDialog>
#include "../core/CardRuleConfig.h"

class QLineEdit;

class CardRuleDialog : public QDialog {
    Q_OBJECT

public:
    explicit CardRuleDialog(const CardRuleConfig& current, QWidget* parent = nullptr);

    CardRuleConfig result() const;

private:
    QLineEdit* deviceIdEdit_;
    QLineEdit* nameEdit_;
    QLineEdit* statusEdit_;
    QLineEdit* dataEdit_;
    QLineEdit* onlineValueEdit_;
    QLineEdit* offlineValueEdit_;
};
