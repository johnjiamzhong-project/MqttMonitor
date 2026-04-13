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
class DeviceCard;
class CardRuleStore;
class CmdPresetStore;

class DeviceView : public QWidget {
    Q_OBJECT

public:
    explicit DeviceView(QWidget* parent = nullptr);

signals:
    void publishRequested(const QString& topic, const QString& payload, int qos);

public slots:
    void updateDevice(const QString& topic, const QString& payload);

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

private:
    void addCard(const QString& id, const QString& name);
    void rebuildGrid();
    void updateCommandPanel();
    void refreshPresetCombo();

    QGridLayout*               grid_;
    QMap<QString, DeviceCard*> cards_;
    QSet<QString>              selectedIds_;

    CardRuleConfig  cardRule_;
    CardRuleStore*  ruleStore_;
    CmdPresetStore* presetStore_;

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
