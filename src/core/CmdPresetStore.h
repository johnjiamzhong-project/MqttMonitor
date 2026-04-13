#pragma once

#include <QObject>
#include <QList>
#include "CmdPreset.h"

class CmdPresetStore : public QObject {
    Q_OBJECT

public:
    explicit CmdPresetStore(QObject* parent = nullptr);

    void load();
    void save() const;

    QList<CmdPreset> presets() const { return presets_; }

    // 以 topic 为基础命名，重名自动加 -2/-3 后缀
    void addPreset(const QString& topic, const QString& payload, int qos);
    void removePreset(const QString& name);

    // 最后使用值（跨重启恢复，不绑定任何预设）
    QString lastTopic()   const { return lastTopic_; }
    QString lastPayload() const { return lastPayload_; }
    int     lastQos()     const { return lastQos_; }
    void    saveLastUsed(const QString& topic, const QString& payload, int qos);

private:
    QString filePath() const;
    QString uniqueName(const QString& base) const;

    QList<CmdPreset> presets_;
    QString lastTopic_;
    QString lastPayload_;
    int     lastQos_ = 1;
};
