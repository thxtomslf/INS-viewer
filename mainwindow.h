#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dynamicsettingsfabric.h"
#include "settingswidget.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const DynamicSettingsFabric &settingsFabric, QWidget *parent = nullptr);
    ~MainWindow();

    void setWidget(QWidget* widget);
    void newWidget(QWidget *widget);
    QWidget *getCurrentWidget();

private slots:
    void openSettingsWindow();

private:
    Ui::MainWindow *ui;
    DynamicSettingsFabric settingsFabric;
    SettingsWidget *settingsWidget;
    bool currentWidgetChosen = false;
};
#endif // MAINWINDOW_H
