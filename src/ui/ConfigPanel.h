#pragma once

#include <QWidget>
#include <QString>

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

private slots:
    void onConnectClicked();

private:
    Ui::ConfigPanel* ui;
};
