#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "inscommandprocessor.h"
#include "routablewidget.cpp"
#include "SensorDataDAO.h"
#include "uartwidget.h"

#include <DynamicSetting.h>
#include <QPushButton>

namespace Ui {
class ChartWidget;
}

class ChartWidget : public RoutableWidget {
    Q_OBJECT

public:
    explicit ChartWidget(InsCommandProcessor *serial,
                         SensorDataDAO *dao,
                         std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                         std::shared_ptr<DynamicSetting<int>> plotSize,
                         QWidget *parent = nullptr);
    ~ChartWidget();

    void onPageHide() override;
    void onPageShow(Page page) override;

private:
    void clearGraphs();
    void updateGraphs(const SensorData &data, const QDateTime &timstamp);

private slots:
    void loadDataForPeriod();
    void handleStopSignal();
    void showData();
    void stopShowData();
    void toggleUartWidget();
    void onUartConnectionChanged(bool connected);
    void updateControlsVisibility(bool visible);

private:
    InsCommandProcessor *processor;
    SensorDataDAO *dao;
    UartWidget *uartWidget;
    bool isUartWidgetVisible;

    Ui::ChartWidget *ui;
};

#endif // CHARTWIDGET_H
