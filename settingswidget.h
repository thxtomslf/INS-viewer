#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QLineEdit>

#include "dynamicsettingsfabric.h"

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(const DynamicSettingsFabric &fabric, QWidget *parent = nullptr);
    ~SettingsWidget();

private slots:
    void applySettings();

private:
    Ui::SettingsWidget *ui;
    DynamicSettingsFabric fabric_;
    QMap<QString, QLineEdit*> lineEdits_;
};

#endif // SETTINGSWIDGET_H
