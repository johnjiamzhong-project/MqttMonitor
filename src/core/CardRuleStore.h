#pragma once

#include <QObject>
#include "CardRuleConfig.h"

class CardRuleStore : public QObject {
    Q_OBJECT

public:
    explicit CardRuleStore(QObject* parent = nullptr);

    void load();
    void save() const;

    const CardRuleConfig& config() const { return config_; }
    void setConfig(const CardRuleConfig& cfg);

private:
    QString filePath() const;

    CardRuleConfig config_;
};
