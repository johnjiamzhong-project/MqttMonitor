#include "ConfigStore.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ConfigStore::ConfigStore(QObject* parent)
    : QObject(parent)
{}

QString ConfigStore::filePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/profiles.json";
}

void ConfigStore::load()
{
    profiles_.clear();
    lastProfile_.clear();

    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;

    const QJsonObject root = doc.object();
    lastProfile_ = root.value("lastProfile").toString();

    const QJsonArray arr = root.value("profiles").toArray();
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject obj = v.toObject();
        if (!obj.contains("name")) continue;

        MqttProfile p;
        p.name     = obj.value("name").toString();
        p.broker   = obj.value("broker").toString();
        p.clientId = obj.value("clientId").toString();
        p.username = obj.value("username").toString();
        p.password = obj.value("password").toString();
        p.topic    = obj.value("topic").toString();
        profiles_.append(p);
    }
}

void ConfigStore::save() const
{
    const QString path = filePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray arr;
    for (const MqttProfile& p : profiles_) {
        QJsonObject obj;
        obj["name"]     = p.name;
        obj["broker"]   = p.broker;
        obj["clientId"] = p.clientId;
        obj["username"] = p.username;
        obj["password"] = p.password;
        obj["topic"]    = p.topic;
        arr.append(obj);
    }

    QJsonObject root;
    root["lastProfile"] = lastProfile_;
    root["profiles"]    = arr;

    // Write to temp file then rename to avoid partial writes
    const QString tmpPath = path + ".tmp";
    QFile tmp(tmpPath);
    if (!tmp.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    tmp.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmp.close();

    QFile::remove(path);
    QFile::rename(tmpPath, path);
}

QStringList ConfigStore::profileNames() const
{
    QStringList names;
    names.reserve(profiles_.size());
    for (const MqttProfile& p : profiles_)
        names.append(p.name);
    return names;
}

MqttProfile ConfigStore::findByName(const QString& name) const
{
    for (const MqttProfile& p : profiles_) {
        if (p.name == name)
            return p;
    }
    return MqttProfile{};
}

void ConfigStore::addOrUpdate(const MqttProfile& profile)
{
    for (MqttProfile& p : profiles_) {
        if (p.name == profile.name) {
            p = profile;
            return;
        }
    }
    profiles_.append(profile);
}

void ConfigStore::remove(const QString& name)
{
    for (int i = 0; i < profiles_.size(); ++i) {
        if (profiles_[i].name == name) {
            profiles_.removeAt(i);
            return;
        }
    }
    if (lastProfile_ == name)
        lastProfile_.clear();
}

void ConfigStore::setLastProfile(const QString& name)
{
    lastProfile_ = name;
}
