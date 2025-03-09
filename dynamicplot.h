#ifndef DYNAMICPLOT_H
#define DYNAMICPLOT_H

#include "DynamicSetting.h"
#include "extendedsensordata.h"

#include <QWidget>
#include <qcustomplot.h>
#include <memory>

class DynamicPlot : public QWidget
{
    Q_OBJECT

public:
    explicit DynamicPlot(QWidget *parent = nullptr, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting = nullptr);

    void addPoint(const QDateTime& time, double value);
    void setLabel(const QString &title);
    void setMaxBufferSize(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting);
    void setPlotSize(std::shared_ptr<DynamicSetting<int>> plotWidth);
    void clear();
    void plotSensorData(const QList<TimestampedSensorData> &dataList, std::function<double(const TimestampedSensorData&)> valueExtractor);
    QList<QPair<QDateTime, double>> getData() const;

private slots:
    void onMaxBufferSizeChanged(int newSize);

private:
    QCustomPlot *customPlot_;
    QCPGraph *graph_;

    int maxBufferSize_;
    int headIndex_;
    int currentSize_;

    QVector<double> timeData_;
    QVector<double> valueData_;

    std::shared_ptr<DynamicSetting<int>> plotBufferSize;
    std::shared_ptr<DynamicSetting<int>> plotSize;
};

#endif // DYNAMICPLOT_H
