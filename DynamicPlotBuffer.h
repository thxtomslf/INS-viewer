#ifndef DYNAMICPLOTBUFFER_H
#define DYNAMICPLOTBUFFER_H

#include <QDateTime>
#include <QVector>
#include <QList>
#include <QPair>
#include <memory>
#include <DynamicSetting.h>

class DynamicPlotBuffer
{
public:
    explicit DynamicPlotBuffer(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting = nullptr);

    void addPoint(const QDateTime& time, double value);
    void clear();
    QList<QPair<QDateTime, double>> getData();

    void setMaxBufferSize(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting);

    QVector<double> getVisibleTimeData() const;
    QVector<double> getVisibleData() const;

private:
    void onMaxBufferSizeChanged(int newSize);

    int maxBufferSize_;
    int headIndex_;
    int currentSize_;

    QVector<double> timeData_;
    QVector<double> valueData_;

    std::shared_ptr<DynamicSetting<int>> plotBufferSize;
};

#endif // DYNAMICPLOTBUFFER_H
