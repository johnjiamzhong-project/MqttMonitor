#pragma once

#include <QFrame>
#include <QJsonObject>
#include <QString>

class QLabel;
class QFormLayout;

class DeviceCard : public QFrame {
    Q_OBJECT

public:
    explicit DeviceCard(const QString& deviceId, QWidget* parent = nullptr);

    const QString& deviceId() const { return deviceId_; }

    void update(const QJsonObject& obj);

private:
    void setStatus(const QString& status);

    QString      deviceId_;
    QLabel*      nameLabel_;
    QLabel*      statusLabel_;
    QFormLayout* dataLayout_;
};
