#include "CardRuleStore.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

CardRuleStore::CardRuleStore(QObject* parent)
    : QObject(parent)
{}

QString CardRuleStore::filePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + "/card_rules.json";
}

void CardRuleStore::load()
{
    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;

    const QJsonObject root = doc.object();
    auto readStr = [&](const QString& key, QString& dest) {
        const QString v = root.value(key).toString();
        if (!v.isEmpty()) dest = v;
    };
    readStr("fieldDeviceId",       config_.fieldDeviceId);
    readStr("fieldName",           config_.fieldName);
    readStr("fieldStatus",         config_.fieldStatus);
    readStr("fieldData",           config_.fieldData);
    readStr("statusOnlineValue",   config_.statusOnlineValue);
    readStr("statusOfflineValue",  config_.statusOfflineValue);
}

void CardRuleStore::save() const
{
    const QString path = filePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonObject root;
    root["fieldDeviceId"]      = config_.fieldDeviceId;
    root["fieldName"]          = config_.fieldName;
    root["fieldStatus"]        = config_.fieldStatus;
    root["fieldData"]          = config_.fieldData;
    root["statusOnlineValue"]  = config_.statusOnlineValue;
    root["statusOfflineValue"] = config_.statusOfflineValue;

    const QString tmpPath = path + ".tmp";
    QFile tmp(tmpPath);
    if (!tmp.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    tmp.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmp.close();

    QFile::remove(path);
    QFile::rename(tmpPath, path);
}

void CardRuleStore::setConfig(const CardRuleConfig& cfg)
{
    config_ = cfg;
}
