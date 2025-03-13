#include "DynamicSettingsFabric.h"
#include "chartwidget.h"
#include "inscommandprocessor.h"
#include "pagerouter.h"
#include "qmenubar.h"
#include "uartwidget.h"

#include <QApplication>
#include <QFile>
#include <QMainWindow>

void loadStyleSheet(QApplication &app, const QString &styleSheetFile) {
    QFile file(styleSheetFile);
    if (file.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
        file.close();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::vector<DynamicSettingsFabric<int>> settingsFabrics;

    std::vector<DynamicSettingsFabric<bool>> booleanSettingsFabrics;
    
    DynamicSettingsFabric<int> generalSettings;
    generalSettings.setGroupName("Общие настройки");
    
    std::shared_ptr<DynamicSetting<int>> plotBufferSize = generalSettings.createSetting("Размер буффера графиков", 500);
    std::shared_ptr<DynamicSetting<int>> plotSize = generalSettings.createSetting("Размер графика", 300);

    settingsFabrics.push_back(generalSettings);

    DynamicSettingsFabric<int> sensorDataSettings;
    DynamicSettingsFabric<bool> sensorDataSettingsBoolean;

    std::string groupName = "Настройки экспорта/импорта экспериментов";
    sensorDataSettings.setGroupName(groupName);
    sensorDataSettingsBoolean.setGroupName(groupName);

    std::shared_ptr<DynamicSetting<bool>> isEnvMeasuresEnabled = sensorDataSettingsBoolean.createSetting("Измерения окружающей среды", true);
    std::shared_ptr<DynamicSetting<bool>> isGyroMeasuresEnabled = sensorDataSettingsBoolean.createSetting("Измерения гироскопа", true);
    std::shared_ptr<DynamicSetting<bool>> isAcceleroMeasuresEnabled = sensorDataSettingsBoolean.createSetting("Измерения акселерометра", true);
    std::shared_ptr<DynamicSetting<bool>> isMagnetoMeasuresEnabled = sensorDataSettingsBoolean.createSetting("Измерения магнитометра", true);

    std::shared_ptr<DynamicSetting<int>> envMeasuresPrecision = sensorDataSettings.createSetting("Измерения окружающей среды, число точек после запятой", 2);
    std::shared_ptr<DynamicSetting<int>> gyroMeasuresPrecision = sensorDataSettings.createSetting("Гироскоп, число точек после запятой", 2);
    std::shared_ptr<DynamicSetting<int>> acceleroMeasuresPrecision = sensorDataSettings.createSetting("Акселерометр, число точек после запятой", 2);
    std::shared_ptr<DynamicSetting<int>> magnetoMeasuresPrecision = sensorDataSettings.createSetting("Магнитометр, число точек после запятой", 2);

    settingsFabrics.push_back(sensorDataSettings);
    booleanSettingsFabrics.push_back(sensorDataSettingsBoolean);

    MainWindow mainWindow(settingsFabrics, booleanSettingsFabrics);

    QMenuBar *menuBar = mainWindow.menuBar();
    QMenu *themeMenu = menuBar->addMenu("Интерфейс");

    QAction *lightThemeAction = new QAction("Светлая тема", &mainWindow);
    QAction *darkThemeAction = new QAction("Темная тема", &mainWindow);

    themeMenu->addAction(lightThemeAction);
    themeMenu->addAction(darkThemeAction);

    QObject::connect(lightThemeAction, &QAction::triggered, [&app]() {
        loadStyleSheet(app, ":/gui/lighttheme.qss");
    });

    QObject::connect(darkThemeAction, &QAction::triggered, [&app]() {
        loadStyleSheet(app, ":/gui/darktheme.qss");
    });

    PageRouter::instance().initialize(&mainWindow);

    InsCommandProcessor *processor = new InsCommandProcessor();
    ChartWidget *chartWidget = new ChartWidget(processor, plotBufferSize, plotSize);

    PageRouter::instance().registerWidget(Page::Graphics, chartWidget);

    PageRouter::instance().navigateTo(Page::Graphics);

    mainWindow.show();
    return app.exec();
}
