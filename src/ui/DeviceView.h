#pragma once

#include <QWidget>
#include <QMap>
#include <QSet>
#include <QString>
#include "../core/CardRuleConfig.h"

class QGridLayout;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QComboBox;
class QPushButton;
class QTimer;
class QCheckBox;
class QSpinBox;
class DeviceCard;
class CardRuleStore;
class CmdPresetStore;
class DisconnectDetectionConfig;

struct DeviceState {
    QString deviceId;
    qint64  lastMessageTime = 0;
    bool    isDisconnected = false;
};

class DeviceView : public QWidget {
    Q_OBJECT

public:
    explicit DeviceView(QWidget* parent = nullptr);

signals:
    void publishRequested(const QString& topic, const QString& payload, int qos);

public slots:
    void updateDevice(const QString& topic, const QString& payload);

public:
    void startDisconnectDetection();
    void stopDisconnectDetection();
    void markAllDevicesDisconnected();

private slots:
    void onAddDeviceClicked();
    void onCardRuleClicked();
    void removeDevice(const QString& deviceId);
    void onCardSelectionChanged(const QString& deviceId, bool selected);
    void onSendClicked();
    void onClearSelectionClicked();
    void onPresetSelected(int index);
    void onSavePresetClicked();
    void onDelPresetClicked();
    void onCmdFieldChanged();
    void checkDisconnectedDevices();
    void onDetectionToggled(bool enabled);
    void onTimeoutChanged(int seconds);

private:
    void addCard(const QString& id, const QString& name);
    void rebuildGrid();
    void updateCommandPanel();
    void refreshPresetCombo();
    void markDeviceDisconnected(const QString& deviceId);
    void markDeviceConnected(const QString& deviceId);

    QGridLayout*               grid_;
    QMap<QString, DeviceCard*> cards_;
    QSet<QString>              selectedIds_;
    QMap<QString, DeviceState> deviceStates_;
    QSet<QString>              checkQueue_;
    QLabel*                    diagLabel_ = nullptr;
    qint64                     lastMsgTime_ = 0;
    QString                    lastMsgId_;

    CardRuleConfig  cardRule_;
    CardRuleStore*  ruleStore_;
    CmdPresetStore* presetStore_;
    DisconnectDetectionConfig* detectionConfig_;

    QTimer*         disconnectCheckTimer_;
    qint64          timeoutMs_;  // timeout in milliseconds

    // Disconnect detection controls
    QCheckBox*      detectionCheckBox_;
    QSpinBox*       timeoutSpinBox_;

    // Command panel widgets
    QWidget*        cmdPanel_;
    QLabel*         selectionLabel_;
    QComboBox*      presetCombo_;
    QPushButton*    savePresetBtn_;
    QPushButton*    delPresetBtn_;
    QLineEdit*      topicEdit_;
    QPlainTextEdit* payloadEdit_;
    QComboBox*      qosCombo_;
};
