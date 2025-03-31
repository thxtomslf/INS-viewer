#include "dynamicplot.h"
#include <QVBoxLayout>
#include <QSharedPointer>
#include <QDateTime>

DynamicPlot::DynamicPlot(QWidget *parent, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting)
    : QWidget(parent)
{
    if (maxBufferSizeSetting) {
        setMaxBufferSize(maxBufferSizeSetting);
    }

    customPlot_ = new QCustomPlot(this);

    graph_ = customPlot_->addGraph();

    // Setup axes
    customPlot_->xAxis->setLabel("Время");
    customPlot_->yAxis->setLabel("Величина");
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy"); // Формат времени и даты
    customPlot_->xAxis->setTicker(dateTimeTicker);

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
    buffer_.setMaxBufferSize(maxBufferSizeSetting);
    maxBufferSizeSetting.get()->setOnUpdateCallback([this](int newValue) {
        onMaxBufferSizeChanged(newValue);
    });
}

void DynamicPlot::clear()
{
    // Очистка данных графика
    graph_->data()->clear();

    // Сброс буферов
    buffer_.clear();

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
    buffer_.addPoint(time, value);

    QVector<double> visibleTimeData = buffer_.getVisibleTimeData();
    QVector<double> visibleValueData = buffer_.getVisibleData();

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
    graph_->setLineStyle(QCPGraph::lsLine);
    if (!timeData.isEmpty()) {
        customPlot_->xAxis->setRange(timeData.first(), timeData.last());
    }
    customPlot_->rescaleAxes(true);
    customPlot_->replot();
}

void DynamicPlot::onMaxBufferSizeChanged(int newSize)
{
    clear();
}

QList<QPair<QDateTime, double>> DynamicPlot::getData()
{
    QList<QPair<QDateTime, double>> dataList;

    if (!graph_ || !graph_->data()) {
        return dataList;
    }

    // Получаем данные из графа
    const QCPDataContainer<QCPGraphData> *dataContainer = graph_->data().data();
    for (auto it = dataContainer->constBegin(); it != dataContainer->constEnd(); ++it) {
        double key = it->key;
        double value = it->value;
        QDateTime time = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(key * 1000));
        dataList.append(qMakePair(time, value));
    }

    return dataList;
}
