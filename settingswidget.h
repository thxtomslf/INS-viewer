#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QLineEdit>
#include <qformlayout.h>
#include <QCheckBox>

#include "dynamicsettingsfabric.h"

namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(
        const std::vector<DynamicSettingsFabric<int>> &numericFabrics,
        const std::vector<DynamicSettingsFabric<bool>> &booleanFabrics,
        QWidget *parent = nullptr);
    ~SettingsWidget();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void applySettings();

private:
    Ui::SettingsWidget *ui;
    const std::vector<DynamicSettingsFabric<int>> numericFabrics;
    const std::vector<DynamicSettingsFabric<bool>> booleanFabrics;
    QMap<QString, QLineEdit*> lineEdits_;
    QMap<QString, QCheckBox*> checkBoxes_;
    QFormLayout *formLayout;
};

#endif // SETTINGSWIDGET_H
