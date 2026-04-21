#pragma once

#include <QFrame>
#include <QJsonObject>
#include <QString>

class QLabel;
class QFormLayout;

class DeviceCard : public QFrame {
    Q_OBJECT
    Q_PROPERTY(bool disconnected READ isDisconnected WRITE setDisconnected)

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

    bool isDisconnected() const { return disconnected_; }
    void setDisconnected(bool disconnected);

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
    bool         disconnected_ = false;
    QString      lastStatusKey_;   // 断联前的状态 key（online/offline/unknown），恢复时使用
};
