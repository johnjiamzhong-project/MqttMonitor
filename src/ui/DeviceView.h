#pragma once

#include <QWidget>
#include <QMap>
#include <QString>

class QGridLayout;
class DeviceCard;

class DeviceView : public QWidget {
    Q_OBJECT

public:
    explicit DeviceView(QWidget* parent = nullptr);

public slots:
    void updateDevice(const QString& topic, const QString& payload);

private:
    QGridLayout*           grid_;
    QMap<QString, DeviceCard*> cards_;
};
