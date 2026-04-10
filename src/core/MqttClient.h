#pragma once

#include <string>
#include <functional>
#include <mqtt/client.h>
#include <mqtt/callback.h>

class MqttClient : public mqtt::callback {
public:
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;

    MqttClient(const std::string& brokerUrl, const std::string& clientId);
    ~MqttClient();

    // Connection management
    bool connect(const std::string& username = "", const std::string& password = "");
    bool disconnect();
    bool isConnected() const;

    // Subscribe
    bool subscribe(const std::string& topic, int qos = 1);
    // Unsubscribe
    bool unsubscribe(const std::string& topic);
    // Publish
    bool publish(const std::string& topic, const std::string& payload, int qos = 1, bool retained = false);

    // Set callback
    void setMessageCallback(MessageCallback callback);

    // Configure options
    void setKeepAliveInterval(int seconds);
    void setConnectTimeout(int milliseconds);

private:
    std::string brokerUrl_;
    std::string clientId_;
    std::unique_ptr<mqtt::client> client_;
    MessageCallback messageCallback_;
    int keepAliveInterval_ = 60;
    int connectTimeout_ = 3000;

    // mqtt::callback overrides (called on Paho's internal thread)
    void message_arrived(mqtt::const_message_ptr msg) override;
    void connection_lost(const std::string& cause) override;
    void delivery_complete(mqtt::delivery_token_ptr) override {}
};
