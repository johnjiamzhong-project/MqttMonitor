#pragma once

#include <QObject>
#include <QString>
#include <QList>

struct MqttProfile {
    QString name;
    QString broker;
    QString clientId;
    QString username;
    QString password;
    QString topic;
};

class ConfigStore : public QObject {
    Q_OBJECT

public:
    explicit ConfigStore(QObject* parent = nullptr);

    void load();
    void save() const;

    QStringList profileNames() const;
    MqttProfile findByName(const QString& name) const;
    void addOrUpdate(const MqttProfile& profile);
    void remove(const QString& name);

    QString lastProfile() const { return lastProfile_; }
    void setLastProfile(const QString& name);

private:
    QString filePath() const;

    QList<MqttProfile> profiles_;
    QString lastProfile_;
};
