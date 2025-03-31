#ifndef DYNAMICPLOTSGROUP_H
#define DYNAMICPLOTSGROUP_H

#include "dynamicplot.h"
#include "extendedsensordata.h"
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <vector>
#include <memory>

class DynamicPlotsGroup : public QWidget
{
    Q_OBJECT

public:
    enum DisplayMode {
        SEPARATE_PLOTS // Режим отдельных графиков (текущая реализация)
        // Здесь можно будет добавить другие режимы отображения
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

    QList<QList<QPair<QDateTime, double>>> getAllData() const;
    
    void addPoint(const QDateTime &timestamp, const std::vector<double> &values);

private:
    void setupLayout();
    void updateLayout();

    DisplayMode currentMode_;
    QScrollArea *scrollArea_;
    QWidget *contentWidget_;
    QVBoxLayout *contentLayout_;
    std::vector<DynamicPlot*> plots_;
};

#endif // DYNAMICPLOTSGROUP_H 