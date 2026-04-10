#include "MessageBuffer.h"
#include <chrono>

MessageBuffer::MessageBuffer(size_t maxSize)
    : maxSize_(maxSize)
{
}

void MessageBuffer::push(const std::string& topic, const std::string& payload)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 如果缓冲区已满，移除最旧的消息
    if (buffer_.size() >= maxSize_) {
        buffer_.pop_front();
    }

    buffer_.emplace_back(topic, payload, getCurrentTimestamp());
}

bool MessageBuffer::pop(MqttMessage& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (buffer_.empty()) {
        return false;
    }

    msg = buffer_.front();
    buffer_.pop_front();
    return true;
}

std::vector<MqttMessage> MessageBuffer::popAll()
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<MqttMessage> result(buffer_.begin(), buffer_.end());
    buffer_.clear();
    return result;
}

void MessageBuffer::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
}

size_t MessageBuffer::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.size();
}

size_t MessageBuffer::maxSize() const
{
    return maxSize_;
}

bool MessageBuffer::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_.empty();
}

long long MessageBuffer::getCurrentTimestamp() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
