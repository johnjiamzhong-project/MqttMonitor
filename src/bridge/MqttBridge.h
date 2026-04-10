#pragma once

#include <QObject>
#include <QString>
#include "../core/MqttClient.h"

class MqttBridge : public QObject {
    Q_OBJECT

public:
    explicit MqttBridge(MqttClient& client, QObject* parent = nullptr);

signals:
    void messageReceived(const QString& topic, const QString& payload);
    void connectionLost();

private:
    MqttClient& client_;
};
