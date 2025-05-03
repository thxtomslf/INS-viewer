#ifndef MULTILINEPLOT_H
#define MULTILINEPLOT_H

#include "DynamicPlotBuffer.h"
#include "DynamicSetting.h"
#include "extendedsensordata.h"

#include <QWidget>
#include <qcustomplot.h>
#include <memory>
#include <vector>

class MultiLinePlot : public QWidget
{
    Q_OBJECT

public:
    explicit MultiLinePlot(QWidget *parent = nullptr, 
                          const std::vector<DynamicPlotBuffer*>& buffers = {});

    void addGraph(const QString &label, 
                 std::shared_ptr<DynamicSetting<int>> plotSize);

    void clear();
    
    void plotSensorData(
        const QList<TimestampedSensorData> &dataList,
        const std::vector<std::pair<
            std::function<double(const TimestampedSensorData&)>,
            std::function<bool(const TimestampedSensorData&)>
        >> &extractors);

    QList<QList<QPair<QDateTime, double>>> getAllData();
    void update();
    void updateBuffers(const std::vector<DynamicPlotBuffer*>& newBuffers);

private:
    void setupPlot();
    void setupLegend();
    void updatePlotSize(int newSize);

    QCustomPlot *customPlot_;
    std::vector<DynamicPlotBuffer*> buffers_;
    std::vector<QString> labels_;
    std::shared_ptr<DynamicSetting<int>> plotSize_;

    // Цвета для графиков
    const QVector<QColor> colors_ = {
        Qt::red, Qt::blue,
        Qt::darkRed, Qt::darkBlue,
        Qt::darkGreen, Qt::darkCyan, Qt::darkMagenta
    };
};

#endif // MULTILINEPLOT_H 
