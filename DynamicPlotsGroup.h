#ifndef DYNAMICPLOTSGROUP_H
#define DYNAMICPLOTSGROUP_H

#include "dynamicplot.h"
#include "extendedsensordata.h"
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <vector>
#include <memory>
#include <MultiLinePlot.h>
#include "DataTableWidget.h"

class DynamicPlotsGroup : public QWidget
{
    Q_OBJECT

public:
    enum DisplayMode {
        TABLE_VIEW,
        SEPARATE_PLOTS, // Режим отдельных графиков
        COMBINED_PLOT   // Режим совмещенных графиков на одном полотне
    };

    explicit DynamicPlotsGroup(QWidget *parent = nullptr);
    
    void setMode(DisplayMode mode);
    void addPlot(const QString &label, 
                 std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                 std::shared_ptr<DynamicSetting<int>> plotSize);
    void clear();
    
    void plotSensorData(
        const QList<TimestampedSensorData> &dataList,
        const std::vector<std::pair<
            std::function<double(const TimestampedSensorData&)>,
            std::function<bool(const TimestampedSensorData&)>
        >> &extractors);

    void addPoint(const QDateTime &timestamp, const std::vector<double> &values);
    QList<QList<QPair<QDateTime, double>>> getAllData() const;
    
private:
    void setupLayout();
    void updateLayout();
    void syncDataToMultiLinePlot();  // Новый метод для синхронизации данных
    void syncDataFromMultiLinePlot(); // Новый метод для синхронизации данных
    void updateDisplayedData(); // Добавляем объявление метода

    DisplayMode currentMode_;
    QScrollArea *scrollArea_;
    QWidget *contentWidget_;
    QVBoxLayout *contentLayout_;
    std::vector<DynamicPlot*> plots_;
    MultiLinePlot *multiLinePlot_;
    DataTableWidget *tableWidget_;
    
    std::vector<QString> plotLabels_;
    std::vector<std::shared_ptr<DynamicSetting<int>>> plotBufferSizes_;
    std::vector<std::shared_ptr<DynamicSetting<int>>> plotSizes_;

    // Общее хранилище данных для всех режимов отображения
    std::vector<DynamicPlotBuffer> dataBuffers_;
};

#endif // DYNAMICPLOTSGROUP_H 
