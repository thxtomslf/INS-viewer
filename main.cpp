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

    std::string groupName = "Акселерометр";
    DynamicSettingsFabric<int> accelerometrSettings;
    DynamicSettingsFabric<bool> accelerometrSettingsBoolean;
    accelerometrSettings.setGroupName(groupName);
    accelerometrSettingsBoolean.setGroupName(groupName);

    std::shared_ptr<DynamicSetting<bool>> isAcceleroMeasuresEnabled = accelerometrSettingsBoolean.createSetting("Вкл/выкл", true);
    std::shared_ptr<DynamicSetting<int>> acceleroMeasuresPrecision = accelerometrSettings.createSetting("Число точек после запятой", 2);

    groupName = "Гироскоп";
    DynamicSettingsFabric<int> gyroscopeSettings;
    DynamicSettingsFabric<bool> gyroscopeSettingsBoolean;
    gyroscopeSettings.setGroupName(groupName);
    gyroscopeSettingsBoolean.setGroupName(groupName);   

    std::shared_ptr<DynamicSetting<bool>> isGyroMeasuresEnabled = gyroscopeSettingsBoolean.createSetting("Вкл/выкл", true);
    std::shared_ptr<DynamicSetting<int>> gyroMeasuresPrecision = gyroscopeSettings.createSetting("Число точек после запятой", 2);

    groupName = "Магнитометр";
    DynamicSettingsFabric<int> magnetometerSettings;
    DynamicSettingsFabric<bool> magnetometerSettingsBoolean;
    magnetometerSettings.setGroupName(groupName);
    magnetometerSettingsBoolean.setGroupName(groupName);

    std::shared_ptr<DynamicSetting<bool>> isMagnetoMeasuresEnabled = magnetometerSettingsBoolean.createSetting("Вкл/выкл", true);
    std::shared_ptr<DynamicSetting<int>> magnetoMeasuresPrecision = magnetometerSettings.createSetting("Число точек после запятой", 2);
    
    groupName = "Окружающая среда";
    DynamicSettingsFabric<int> envSettings;
    DynamicSettingsFabric<bool> envSettingsBoolean;
    envSettings.setGroupName(groupName);
    envSettingsBoolean.setGroupName(groupName);

    std::shared_ptr<DynamicSetting<bool>> isEnvMeasuresEnabled = envSettingsBoolean.createSetting("Вкл/выкл", true);
    std::shared_ptr<DynamicSetting<int>> envMeasuresPrecision = envSettings.createSetting("Число точек после запятой", 2);
    
    settingsFabrics.push_back(accelerometrSettings);
    settingsFabrics.push_back(gyroscopeSettings);
    settingsFabrics.push_back(magnetometerSettings);
    settingsFabrics.push_back(envSettings);

    booleanSettingsFabrics.push_back(accelerometrSettingsBoolean);
    booleanSettingsFabrics.push_back(gyroscopeSettingsBoolean);
    booleanSettingsFabrics.push_back(magnetometerSettingsBoolean);
    booleanSettingsFabrics.push_back(envSettingsBoolean);

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
