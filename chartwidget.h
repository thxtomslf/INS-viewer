#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include "inscommandprocessor.h"
#include "routablewidget.cpp"
#include "SensorDataDAO.h"

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

private:
    void clearGraphs();
    void updateGraphs(const SensorData &data, const QDateTime &timstamp);

private slots:
    void loadDataForPeriod();
    void onUartButtonClicked();
    void showData();
    void stopShowData();

private:
    InsCommandProcessor *processor;
    SensorDataDAO *dao;

    Ui::ChartWidget *ui;
};

#endif // CHARTWIDGET_H
