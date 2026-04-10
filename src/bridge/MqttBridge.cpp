#include "MqttBridge.h"

MqttBridge::MqttBridge(MqttClient& client, QObject* parent)
    : QObject(parent), client_(client)
{
    client_.setMessageCallback([this](const std::string& topic, const std::string& payload) {
        // Called on Paho's thread — emit with QueuedConnection delivers to Qt main thread
        emit messageReceived(QString::fromStdString(topic), QString::fromStdString(payload));
    });
}
