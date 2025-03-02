#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QThread>


MainWindow::MainWindow(const DynamicSettingsFabric &settingsFabric, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      settingsFabric(settingsFabric)
{
    ui->setupUi(this);

    QMenu *settingsMenu = menuBar()->addMenu("Настройки");

    // Создаем действие для открытия окна настроек
    QAction *openSettingsAction = new QAction("Открыть настройки", this);
    settingsMenu->addAction(openSettingsAction);

    // Подключаем сигнал нажатия к слоту
    connect(openSettingsAction, &QAction::triggered, this, &MainWindow::openSettingsWindow);

    settingsWidget = new SettingsWidget(settingsFabric);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete settingsWidget;
}

void MainWindow::openSettingsWindow() {
    settingsWidget->show();
}

void MainWindow::setWidget(QWidget *widget) {
    ui->mainWidget->setCurrentWidget(widget);
    currentWidgetChosen = true;
}

QWidget *MainWindow::getCurrentWidget() {
    if (currentWidgetChosen) {
        return ui->mainWidget->currentWidget();
    }

    return nullptr;
}

void MainWindow::newWidget(QWidget *widget) {
    ui->mainWidget->addWidget(widget);
}

