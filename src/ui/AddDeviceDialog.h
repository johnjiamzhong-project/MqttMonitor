#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

class AddDeviceDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddDeviceDialog(QWidget* parent = nullptr);

    QString deviceId() const;
    QString deviceName() const;

private:
    QLineEdit* idEdit_;
    QLineEdit* nameEdit_;
};
