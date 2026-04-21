#pragma once

#include <string>
#include <functional>
#include <mqtt/client.h>
#include <mqtt/callback.h>

// Paho MQTT C++ 客户端的封装类，处理 MQTT 连接、订阅、发布
// 消息回调运行在 Paho 的内部线程，调用方需通过信号/队列实现线程安全的跨线程通信
class MqttClient : public mqtt::callback {
public:
    // 消息回调函数签名：接收 topic 和 payload
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;
    // 连接丢失回调函数签名
    using ConnectionLostCallback = std::function<void()>;

    // 构造函数：broker_url 如 "tcp://192.168.1.100:1883"，client_id 用于 MQTT 连接标识
    MqttClient(const std::string& brokerUrl, const std::string& clientId);
    ~MqttClient();

    // 连接管理
    // 与 MQTT Broker 建立连接，支持用户名/密码认证
    bool connect(const std::string& username = "", const std::string& password = "");
    // 断开连接
    bool disconnect();
    // 检查连接状态
    bool isConnected() const;

    // 订阅主题（QoS 默认 1）
    bool subscribe(const std::string& topic, int qos = 1);
    // 取消订阅
    bool unsubscribe(const std::string& topic);
    // 发布消息，retained 标志表示是否保留消息
    bool publish(const std::string& topic, const std::string& payload, int qos = 1, bool retained = false);

    // 设置消息到达回调，回调运行在 Paho 内部线程
    void setMessageCallback(MessageCallback callback);
    // 设置连接丢失回调，回调运行在 Paho 内部线程
    void setConnectionLostCallback(ConnectionLostCallback callback);

    // 配置选项
    // Keep-Alive 间隔，默认 60 秒
    void setKeepAliveInterval(int seconds);
    // 连接超时，默认 3000 毫秒
    void setConnectTimeout(int milliseconds);

private:
    std::string brokerUrl_;
    std::string clientId_;
    std::unique_ptr<mqtt::client> client_;
    MessageCallback messageCallback_;
    ConnectionLostCallback connectionLostCallback_;
    int keepAliveInterval_ = 60;
    int connectTimeout_ = 3000;

    // Paho 回调接口重写（运行在 Paho 内部线程，不要阻塞）
    // 消息到达回调
    void message_arrived(mqtt::const_message_ptr msg) override;
    // 连接丢失回调
    void connection_lost(const std::string& cause) override;
    // 消息发送完成回调（本实现不需要处理）
    void delivery_complete(mqtt::delivery_token_ptr) override {}
};
