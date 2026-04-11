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

signals:
    void removeRequested(const QString& deviceId);
    void selectionChanged(const QString& deviceId, bool selected);

public:
    bool isSelected() const { return selected_; }
    void setSelected(bool selected);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setStatus(const QString& status);
    void updateSelectionStyle();

    QString      deviceId_;
    QLabel*      nameLabel_;
    QLabel*      statusLabel_;
    QFormLayout* dataLayout_;
    bool         selected_ = false;
};
