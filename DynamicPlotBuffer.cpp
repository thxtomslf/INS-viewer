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
QVector<double> DynamicPlotBuffer::getVisibleTimeData() const
{
    QVector<double> result;
    result.reserve(currentSize_);
    
    if (currentSize_ == 0) {
        return result;
    }
    
    if (headIndex_ >= currentSize_) {
        // Данные лежат непрерывно от (headIndex_ - currentSize_) до headIndex_
        result = timeData_.mid(headIndex_ - currentSize_, currentSize_);
    } else {
        // Данные разорваны - берем конец и начало буфера
        int firstPartSize = currentSize_ - headIndex_;
        int secondPartSize = headIndex_;
        
        result = timeData_.mid(maxBufferSize_ - firstPartSize, firstPartSize);
        result += timeData_.mid(0, secondPartSize);
    }
    
    return result;
}

QVector<double> DynamicPlotBuffer::getVisibleData() const
{
    QVector<double> result;
    result.reserve(currentSize_);
    
    if (currentSize_ == 0) {
        return result;
    }
    
    if (headIndex_ >= currentSize_) {
        // Данные лежат непрерывно от (headIndex_ - currentSize_) до headIndex_
        result = valueData_.mid(headIndex_ - currentSize_, currentSize_);
    } else {
        // Данные разорваны - берем конец и начало буфера
        int firstPartSize = currentSize_ - headIndex_;
        int secondPartSize = headIndex_;
        
        result = valueData_.mid(maxBufferSize_ - firstPartSize, firstPartSize);
        result += valueData_.mid(0, secondPartSize);
    }
    
    return result;
}

void DynamicPlotBuffer::setMaxBufferSize(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting) {
    plotBufferSize = maxBufferSizeSetting;
    plotBufferSize.get()->setOnUpdateCallback([this](int newValue) {
        onMaxBufferSizeChanged(newValue);
    });    
}


QList<QPair<QDateTime, double>> DynamicPlotBuffer::getData() const
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

QVector<double> DynamicPlotBuffer::getAllTimeData() const
{
    return timeData_;
}

QVector<double> DynamicPlotBuffer::getAllData() const
{
    return valueData_;
}
