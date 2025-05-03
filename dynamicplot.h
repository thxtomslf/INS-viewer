#ifndef DYNAMICPLOT_H
#define DYNAMICPLOT_H

#include "DynamicPlotBuffer.h"
#include "DynamicSetting.h"
#include "extendedsensordata.h"

#include <QWidget>
#include <qcustomplot.h>
#include <memory>
#include <QScrollArea>
#include <QWheelEvent>
#include <QApplication>

class DynamicPlot : public QWidget
{
    Q_OBJECT

public:
    explicit DynamicPlot(QWidget *parent = nullptr,
                        DynamicPlotBuffer* buffer = nullptr);

    void addPoint(const QDateTime& time, double value);
    void setLabel(const QString &title);
    void setPlotSize(std::shared_ptr<DynamicSetting<int>> plotWidth);
    void clear();
    void plotSensorData(
        const QList<TimestampedSensorData> &dataList,
        std::function<double(const TimestampedSensorData&)> valueExtractor,
        std::function<bool(const TimestampedSensorData&)> shouldPlot = [](const TimestampedSensorData&) { return true; });
    QList<QPair<QDateTime, double>> getData();
    void update();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QCustomPlot *customPlot_;
    QCPGraph *graph_;

    DynamicPlotBuffer* buffer_;
    std::shared_ptr<DynamicSetting<int>> plotSize;

    bool shouldHandleWheelEvent(QWheelEvent *event) const;
    QScrollArea* findParentScrollArea() const;
};

#endif // DYNAMICPLOT_H
