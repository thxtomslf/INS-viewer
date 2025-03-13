#include "dynamicplot.h"
#include <QVBoxLayout>
#include <QSharedPointer>
#include <QDateTime>

DynamicPlot::DynamicPlot(QWidget *parent, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting)
    : QWidget(parent), plotBufferSize(maxBufferSizeSetting), headIndex_(0), currentSize_(0)
{
    if (plotBufferSize) {
        maxBufferSize_ = plotBufferSize->get();
        plotBufferSize.get()->setOnUpdateCallback([this](int newValue) {
            onMaxBufferSizeChanged(newValue);
        });
    } else {
        maxBufferSize_ = 500;
    }

    customPlot_ = new QCustomPlot(this);

    graph_ = customPlot_->addGraph();

    // Setup axes
    customPlot_->xAxis->setLabel("Время");
    customPlot_->yAxis->setLabel("Величина");
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy"); // Формат времени и даты
    customPlot_->xAxis->setTicker(dateTimeTicker);

    timeData_.resize(maxBufferSize_);
    valueData_.resize(maxBufferSize_);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(customPlot_);
    setLayout(layout);
}

void DynamicPlot::setPlotSize(std::shared_ptr<DynamicSetting<int>> plotSize) {
    this->plotSize = plotSize;
    customPlot_->setMinimumSize(plotSize->get(), plotSize->get());

    this->plotSize.get()->setOnUpdateCallback([this](int newValue) {
        qDebug() << this->plotSize->get();
        customPlot_->setMinimumSize(this->plotSize->get(), this->plotSize->get());
    });
}

void DynamicPlot::setMaxBufferSize(std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting) {
    clear();
    plotBufferSize = maxBufferSizeSetting;
    maxBufferSize_ = plotBufferSize->get();
    plotBufferSize.get()->setOnUpdateCallback([this](int newValue) {
        onMaxBufferSizeChanged(newValue);
    });
}

void DynamicPlot::clear()
{
    // Очистка данных графика
    graph_->data()->clear();

    // Сброс буферов
    timeData_.fill(0);
    valueData_.fill(0);
    headIndex_ = 0;
    currentSize_ = 0;

    // Перерисовка графика
    customPlot_->replot();
}

void DynamicPlot::setLabel(const QString &title) {
    // Устанавливаем заголовок графика с использованием QCPTextElement
    QCPTextElement *titleElement = new QCPTextElement(customPlot_, title, QFont("sans", 12, QFont::Bold));
    customPlot_->plotLayout()->insertRow(0);
    customPlot_->plotLayout()->addElement(0, 0, titleElement);

    customPlot_->yAxis->setLabel(title);
}

void DynamicPlot::addPoint(const QDateTime& time, double value)
{
    double key = time.toMSecsSinceEpoch() / 1000.0;

    timeData_[headIndex_] = key;
    valueData_[headIndex_] = value;

    headIndex_ = (headIndex_ + 1) % maxBufferSize_; // Move head index to next position

    if (currentSize_ < maxBufferSize_) {
        ++currentSize_;
    }

    QVector<double> visibleTimeData = QVector<double>::fromList(timeData_.mid(0, currentSize_));
    QVector<double> visibleValueData = QVector<double>::fromList(valueData_.mid(0, currentSize_));

    graph_->setData(visibleTimeData, visibleValueData);
    customPlot_->xAxis->setRange(visibleTimeData.first(), visibleTimeData.last());
    customPlot_->rescaleAxes(true);
    customPlot_->replot();
}

void DynamicPlot::plotSensorData(
    const QList<TimestampedSensorData> &dataList,
    std::function<double(const TimestampedSensorData&)> valueExtractor,
    std::function<bool(const TimestampedSensorData&)> shouldPlot)
{
    QVector<double> timeData;
    QVector<double> valueData;

    for (const auto &data : dataList) {
        if (!shouldPlot(data)) {
            continue;
        }
        double key = data.getTimestamp().toMSecsSinceEpoch() / 1000.0;
        double value = valueExtractor(data);
        timeData.append(key);
        valueData.append(value);
    }

    graph_->setData(timeData, valueData);
    if (!timeData.isEmpty()) {
        customPlot_->xAxis->setRange(timeData.first(), timeData.last());
    }
    customPlot_->rescaleAxes(true);
    customPlot_->replot();
}

void DynamicPlot::onMaxBufferSizeChanged(int newSize)
{
    maxBufferSize_ = newSize;
    timeData_.resize(maxBufferSize_);
    valueData_.resize(maxBufferSize_);
    clear();
}

QList<QPair<QDateTime, double>> DynamicPlot::getData() const
{
    QList<QPair<QDateTime, double>> dataList;
    for (int i = 0; i < currentSize_; ++i) {
        QDateTime time = QDateTime::fromMSecsSinceEpoch(timeData_[i] * 1000);
        dataList.append(qMakePair(time, valueData_[i]));
    }
    return dataList;
}
