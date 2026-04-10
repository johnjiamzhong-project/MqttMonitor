#include "MqttClient.h"
#include <iostream>

MqttClient::MqttClient(const std::string& brokerUrl, const std::string& clientId)
    : brokerUrl_(brokerUrl), clientId_(clientId)
{
    try {
        client_ = std::make_unique<mqtt::client>(brokerUrl, clientId);
    } catch (const mqtt::exception& e) {
        std::cerr << "MQTT Client creation failed: " << e.what() << std::endl;
    }
}

MqttClient::~MqttClient()
{
    disconnect();
}

bool MqttClient::connect(const std::string& username, const std::string& password)
{
    if (!client_) return false;

    try {
        mqtt::connect_options connOpts;
        connOpts.set_clean_session(true);
        connOpts.set_keep_alive_interval(keepAliveInterval_);
        connOpts.set_connect_timeout(std::chrono::milliseconds(connectTimeout_));

        if (!username.empty()) {
            connOpts.set_user_name(username);
        }
        if (!password.empty()) {
            connOpts.set_password(password);
        }

        client_->set_callback(*this);
        client_->connect(connOpts);
        std::cout << "Connected to " << brokerUrl_ << std::endl;
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

bool MqttClient::disconnect()
{
    if (!client_) return false;

    try {
        if (isConnected()) {
            client_->disconnect();
            std::cout << "Disconnected from " << brokerUrl_ << std::endl;
        }
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Disconnect failed: " << e.what() << std::endl;
        return false;
    }
}

bool MqttClient::isConnected() const
{
    return client_ && client_->is_connected();
}

bool MqttClient::subscribe(const std::string& topic, int qos)
{
    if (!isConnected()) return false;

    try {
        client_->subscribe(topic, qos);
        std::cout << "Subscribed to: " << topic << std::endl;
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Subscribe failed: " << e.what() << std::endl;
        return false;
    }
}

bool MqttClient::unsubscribe(const std::string& topic)
{
    if (!isConnected()) return false;

    try {
        client_->unsubscribe(topic);
        std::cout << "Unsubscribed from: " << topic << std::endl;
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Unsubscribe failed: " << e.what() << std::endl;
        return false;
    }
}

bool MqttClient::publish(const std::string& topic, const std::string& payload, int qos, bool retained)
{
    if (!isConnected()) return false;

    try {
        client_->publish(topic, payload.data(), payload.size(), qos, retained);
        std::cout << "Published to " << topic << ": " << payload << std::endl;
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Publish failed: " << e.what() << std::endl;
        return false;
    }
}

void MqttClient::setMessageCallback(MessageCallback callback)
{
    messageCallback_ = callback;
}

void MqttClient::setKeepAliveInterval(int seconds)
{
    keepAliveInterval_ = seconds;
}

void MqttClient::setConnectTimeout(int milliseconds)
{
    connectTimeout_ = milliseconds;
}

void MqttClient::message_arrived(mqtt::const_message_ptr msg)
{
    if (messageCallback_) {
        messageCallback_(msg->get_topic(), msg->get_payload_str());
    }
}

void MqttClient::connection_lost(const std::string& cause)
{
    std::cerr << "Connection lost: " << cause << std::endl;
}
