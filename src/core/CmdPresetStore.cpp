#include "CmdPresetStore.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CmdPresetStore::CmdPresetStore(QObject* parent)
    : QObject(parent)
{}

QString CmdPresetStore::filePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/cmd_presets.json";
}

void CmdPresetStore::load()
{
    presets_.clear();

    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;

    const QJsonObject root = doc.object();
    lastTopic_   = root.value("lastTopic").toString();
    lastPayload_ = root.value("lastPayload").toString();
    lastQos_     = root.value("lastQos").toInt(1);

    const QJsonArray arr = root.value("presets").toArray();
    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject obj = v.toObject();
        CmdPreset p;
        p.name    = obj.value("name").toString();
        p.topic   = obj.value("topic").toString();
        p.payload = obj.value("payload").toString();
        p.qos     = obj.value("qos").toInt(1);
        if (!p.name.isEmpty())
            presets_.append(p);
    }
}

void CmdPresetStore::save() const
{
    const QString path = filePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonArray arr;
    for (const CmdPreset& p : presets_) {
        QJsonObject obj;
        obj["name"]    = p.name;
        obj["topic"]   = p.topic;
        obj["payload"] = p.payload;
        obj["qos"]     = p.qos;
        arr.append(obj);
    }

    QJsonObject root;
    root["lastTopic"]   = lastTopic_;
    root["lastPayload"] = lastPayload_;
    root["lastQos"]     = lastQos_;
    root["presets"]     = arr;

    const QString tmpPath = path + ".tmp";
    QFile tmp(tmpPath);
    if (!tmp.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    tmp.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmp.close();

    QFile::remove(path);
    QFile::rename(tmpPath, path);
}

QString CmdPresetStore::uniqueName(const QString& base) const
{
    // Check if base name is already taken
    auto nameExists = [&](const QString& n) {
        for (const CmdPreset& p : presets_)
            if (p.name == n) return true;
        return false;
    };

    if (!nameExists(base))
        return base;

    for (int i = 2; ; ++i) {
        const QString candidate = base + QStringLiteral("-%1").arg(i);
        if (!nameExists(candidate))
            return candidate;
    }
}

void CmdPresetStore::addPreset(const QString& topic, const QString& payload, int qos)
{
    CmdPreset p;
    p.name    = uniqueName(topic);
    p.topic   = topic;
    p.payload = payload;
    p.qos     = qos;
    presets_.append(p);
    save();
}

void CmdPresetStore::removePreset(const QString& name)
{
    for (int i = 0; i < presets_.size(); ++i) {
        if (presets_[i].name == name) {
            presets_.removeAt(i);
            save();
            return;
        }
    }
}

void CmdPresetStore::saveLastUsed(const QString& topic, const QString& payload, int qos)
{
    lastTopic_   = topic;
    lastPayload_ = payload;
    lastQos_     = qos;
    save();
}
