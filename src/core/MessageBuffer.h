#pragma once

#include <deque>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

// 单条 MQTT 消息，由 Paho 回调线程写入，UI 主线程读取
struct MqttMessage {
    std::string topic;     // 消息主题，如 devices/device001/status
    std::string payload;   // 消息体，原始字符串（通常为 JSON）
    long long timestamp;   // 收到时刻的 Unix 毫秒时间戳

    MqttMessage(const std::string& t, const std::string& p, long long ts)
        : topic(t), payload(p), timestamp(ts) {}
};

class MessageBuffer {
public:
    MessageBuffer(size_t maxSize = 1000);

    // Push message (called by Paho callback thread)
    void push(const std::string& topic, const std::string& payload);

    // Pop one message
    bool pop(MqttMessage& msg);

    // Pop all messages
    std::vector<MqttMessage> popAll();

    // Clear buffer
    void clear();

    // Get current queue size
    size_t size() const;

    // Get max capacity
    size_t maxSize() const;

    // Check if empty
    bool empty() const;

private:
    std::deque<MqttMessage> buffer_;
    mutable std::mutex mutex_;
    size_t maxSize_;

    // Get current timestamp in milliseconds
    long long getCurrentTimestamp() const;
};
