#pragma once

#include <QString>

class DisconnectDetectionConfig {
public:
    DisconnectDetectionConfig();

    void load();
    void save() const;

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled);

    int timeoutSeconds() const { return timeoutSeconds_; }
    void setTimeoutSeconds(int seconds);

private:
    QString filePath() const;

    bool enabled_;
    int  timeoutSeconds_;  // seconds
};
