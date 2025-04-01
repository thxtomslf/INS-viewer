#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "inscommandprocessor.h"
#include "routablewidget.cpp"
#include "storagemanager.h"
#include "uartwidget.h"

#include <CsvSensorDataDAO.h>
#include <DynamicSetting.h>
#include <QPushButton>
#include <RangeSlider.h>
#include "dynamicplotsgroup.h"

namespace Ui {
class ChartWidget;

}

class ChartWidget : public RoutableWidget {
    Q_OBJECT


public:

    enum WidgetMode {
        FILE,
        UART
    };

    explicit ChartWidget(InsCommandProcessor *serial,
                         std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                         std::shared_ptr<DynamicSetting<int>> plotSize, 
                         FileStorageManager *storageManager,
                         QWidget *parent = nullptr);
    ~ChartWidget();

    void onPageHide() override;
    void onPageShow(Page page) override;

private:
    void clearGraphs();
    void updateGraphs(const SensorData &data, const QDateTime &timstamp);
    void setMode(WidgetMode mode);

    void initUartWidget();
    void initRangeSlider();
    void initSplitter();
    void initToggleUartButton();
    void initStartToggleButton();
    void initCharts(std::shared_ptr<DynamicSetting<int>> plotBufferSize, std::shared_ptr<DynamicSetting<int>> plotSize);
    void initStorageButtons();
    void initDisplayModeButtons();

private slots:
    void showData();
    void stopShowData();
    void handleStopSignal();
    void toggleUartWidget();
    void saveToFile();
    void loadFromFile();
    void onUartConnectionChanged(bool connected);
    void loadDataForPeriod(const QDateTime &start, const QDateTime &end);
    void onDisplayModeChanged();

private:
    InsCommandProcessor *processor;
    UartWidget *uartWidget;
    bool isUartWidgetVisible;
    FileStorageManager *storageManager = nullptr;

    Ui::ChartWidget *ui;

    RangeSlider *rangeSlider;
    bool isFileLoaded;
    QDateTime minTimestamp;
    QDateTime maxTimestamp;

    DynamicPlotsGroup *envGroup_;
    DynamicPlotsGroup *acceleroGroup_;
    DynamicPlotsGroup *gyroGroup_;
    DynamicPlotsGroup *magnetoGroup_;

    void updateDisplayModeButtons(DynamicPlotsGroup::DisplayMode mode);
    void setDisplayMode(DynamicPlotsGroup::DisplayMode mode);
};

#endif // CHARTWIDGET_H
