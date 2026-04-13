#pragma once

#include <QString>

struct CmdPreset {
    QString name;       // 展示名（以 topic 为基础，重名加后缀 -2/-3）
    QString topic;      // 实际 topic 模板
    QString payload;
    int     qos = 1;
};
