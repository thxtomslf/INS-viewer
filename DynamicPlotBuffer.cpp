#include "DynamicPlotBuffer.h"

DynamicPlotBuffer::DynamicPlotBuffer(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting)
    : plotBufferSize(maxBufferSizeSetting), headIndex_(0), currentSize_(0)
{
    if (plotBufferSize) {
        maxBufferSize_ = plotBufferSize->get();
        plotBufferSize.get()->setOnUpdateCallback([this](int newValue) {
            onMaxBufferSizeChanged(newValue);
        });
    } else {
        maxBufferSize_ = 500;
    }

    timeData_.resize(maxBufferSize_);
    valueData_.resize(maxBufferSize_);
}

void DynamicPlotBuffer::addPoint(const QDateTime& time, double value)
{
    double key = time.toMSecsSinceEpoch() / 1000.0;

    timeData_[headIndex_] = key;
    valueData_[headIndex_] = value;

    headIndex_ = (headIndex_ + 1) % maxBufferSize_; // Move head index to next position

    if (currentSize_ < maxBufferSize_) {
        ++currentSize_;
    }
}

void DynamicPlotBuffer::clear()
{
    timeData_.fill(0);
    valueData_.fill(0);
    headIndex_ = 0;
    currentSize_ = 0;
}

QVector<double> DynamicPlotBuffer::getVisibleTimeData() {
    return QVector<double>::fromList(timeData_.mid(0, currentSize_));
}

QVector<double> DynamicPlotBuffer::getVisibleData() {
    return QVector<double>::fromList(valueData_.mid(0, currentSize_));
}

void DynamicPlotBuffer::setMaxBufferSize(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting) {
    plotBufferSize = maxBufferSizeSetting;
    plotBufferSize.get()->setOnUpdateCallback([this](int newValue) {
        onMaxBufferSizeChanged(newValue);
    });    
}


QList<QPair<QDateTime, double>> DynamicPlotBuffer::getData()
{
    QVector<double> visibleTimeData = this->getVisibleTimeData();
    QVector<double> visibleData = this->getVisibleData();

    QList<QPair<QDateTime, double>> dataList;
    for (int i = 0; i < currentSize_; ++i) {
        QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(visibleTimeData[i] * 1000));
        dataList.append(qMakePair(time, visibleData[i]));
    }
    return dataList;
}

void DynamicPlotBuffer::onMaxBufferSizeChanged(int newSize)
{
    maxBufferSize_ = newSize;
    timeData_.resize(maxBufferSize_);
    valueData_.resize(maxBufferSize_);

    if (currentSize_ > maxBufferSize_) {
        currentSize_ = maxBufferSize_;
    }
}
