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

    DynamicSettingsFabric settingsFabric;

    std::shared_ptr<DynamicSetting<int>> plotBufferSize = settingsFabric.createSetting("Размер буффера графиков", 500);

    std::shared_ptr<DynamicSetting<int>> plotSize = settingsFabric.createSetting("Размер графика", 300);


    MainWindow mainWindow(settingsFabric);

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
