#pragma once

#include <QWidget>
#include <QString>
#include "../core/ConfigStore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ConfigPanel; }
QT_END_NAMESPACE

class ConfigPanel : public QWidget {
    Q_OBJECT

public:
    explicit ConfigPanel(QWidget* parent = nullptr);
    ~ConfigPanel();

signals:
    void connectRequested(const QString& brokerUrl,
                          const QString& clientId,
                          const QString& username,
                          const QString& password,
                          const QString& topic);
    void disconnectRequested();

public slots:
    void onConnected();
    void onDisconnected();
    void appendMessage(const QString& topic, const QString& payload);

private slots:
    void onConnectClicked();
    void onProfileSelected(const QString& name);
    void onSaveProfileClicked();
    void onDeleteProfileClicked();

private:
    void populateProfileCombo();
    void loadProfile(const QString& name);

    Ui::ConfigPanel* ui;
    ConfigStore* store_;
};
