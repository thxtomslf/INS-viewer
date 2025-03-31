#ifndef DYNAMICPLOT_H
#define DYNAMICPLOT_H

#include "DynamicSetting.h"
#include "dynamicplot.h"
#include "extendedsensordata.h"

#include <QWidget>
#include <qcustomplot.h>
#include <memory>
#include "DynamicSetting.h"


enum class PlotType {
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    ACCELEROMETER_X,
    ACCELEROMETER_Y,
    ACCELEROMETER_Z,
    GYROSCOPE_X,
    GYROSCOPE_Y,
    GYROSCOPE_Z,
    MAGNETOMETER_X,
    MAGNETOMETER_Y,
    MAGNETOMETER_Z
};


class DynamicPlotsGroup : public QWidget
{
    Q_OBJECT

public:
    explicit DynamicPlotsGroup(QWidget *parent = nullptr, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting = nullptr);
    ~DynamicPlotsGroup();
    
    DynamicPlot* addPlot(const PlotType& plotType, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting = nullptr);

    QList<QPair<QDateTime, double>> getData(const PlotType& plotType) const;

private:
    QMap<PlotType, DynamicPlot*> plots_;


};

#endif // DYNAMICPLOT_H
