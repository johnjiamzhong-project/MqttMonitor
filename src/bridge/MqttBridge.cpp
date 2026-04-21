#include "MqttBridge.h"

MqttBridge::MqttBridge(MqttClient& client, QObject* parent)
    : QObject(parent), client_(client)
{
    client_.setMessageCallback([this](const std::string& topic, const std::string& payload) {
        emit messageReceived(QString::fromStdString(topic), QString::fromStdString(payload));
    });

    client_.setConnectionLostCallback([this]() {
        std::cerr << "[MqttBridge] Emitting connectionLost signal" << std::endl;
        emit connectionLost();
    });
}
