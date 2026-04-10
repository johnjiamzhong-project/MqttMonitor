#pragma once

#include <deque>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

struct MqttMessage {
    std::string topic;
    std::string payload;
    long long timestamp;  // milliseconds

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
