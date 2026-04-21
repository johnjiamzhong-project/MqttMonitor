#include "DisconnectDetectionConfig.h"

#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

DisconnectDetectionConfig::DisconnectDetectionConfig()
    : enabled_(false), timeoutSeconds_(30)
{
}

QString DisconnectDetectionConfig::filePath() const
{
    const QString appDataPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    return appDataPath + "/disconnect_detection.json";
}

void DisconnectDetectionConfig::load()
{
    const QString path = filePath();
    QFile file(path);
    if (!file.exists()) {
        enabled_ = false;
        timeoutSeconds_ = 30;
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return;

    const QJsonObject obj = doc.object();
    enabled_ = obj.value("enabled").toBool(false);
    timeoutSeconds_ = obj.value("timeoutSeconds").toInt(30);
    if (timeoutSeconds_ < 1)
        timeoutSeconds_ = 1;
    if (timeoutSeconds_ > 300)
        timeoutSeconds_ = 300;
}

void DisconnectDetectionConfig::save() const
{
    QJsonObject obj;
    obj["enabled"] = enabled_;
    obj["timeoutSeconds"] = timeoutSeconds_;

    const QJsonDocument doc(obj);
    const QString path = filePath();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(doc.toJson());
        file.close();
    }
}

void DisconnectDetectionConfig::setEnabled(bool enabled)
{
    enabled_ = enabled;
}

void DisconnectDetectionConfig::setTimeoutSeconds(int seconds)
{
    if (seconds < 1)
        seconds = 1;
    if (seconds > 300)
        seconds = 300;
    timeoutSeconds_ = seconds;
}
