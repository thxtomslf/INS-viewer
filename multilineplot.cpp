#include "multilineplot.h"
#include <QVBoxLayout>

MultiLinePlot::MultiLinePlot(QWidget *parent, const std::vector<DynamicPlotBuffer*>& buffers)
    : QWidget(parent)
    , buffers_(buffers)
{
    setupPlot();
}

void MultiLinePlot::setupPlot()
{
    customPlot_ = new QCustomPlot(this);
    
    // Настройка осей
    customPlot_->xAxis->setLabel("Время");
    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeFormat("hh:mm:ss\ndd.MM.yyyy");
    customPlot_->xAxis->setTicker(dateTimeTicker);

    // Настройка легенды
    customPlot_->legend->setVisible(true);
    customPlot_->legend->setFont(QFont("Helvetica", 9));
    customPlot_->legend->setBrush(QBrush(QColor(255,255,255,230)));
    
    // Добавляем график в layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(customPlot_);
    setLayout(layout);
}

void MultiLinePlot::addGraph(const QString &label,
                           std::shared_ptr<DynamicSetting<int>> plotSize)
{
    // Создаем новый график
    QCPGraph *graph = customPlot_->addGraph();
    graph->setName(label);
    
    // Настраиваем внешний вид
    int colorIndex = buffers_.size() % colors_.size();
    graph->setPen(QPen(colors_[colorIndex]));
    graph->setLineStyle(QCPGraph::lsLine);

    // Добавляем буфер для данных
    labels_.push_back(label);

    // Сохраняем настройку размера графика
    if (!plotSize_) {
        plotSize_ = plotSize;
        plotSize_->setOnUpdateCallback([this](int newSize) {
            updatePlotSize(newSize);
        });
        updatePlotSize(plotSize_->get());
    }
}

void MultiLinePlot::clear()
{
    for (size_t i = 0; i < buffers_.size(); ++i) {
        if (buffers_[i] && i < customPlot_->graphCount()) {
            buffers_[i]->clear();
            customPlot_->graph(i)->data()->clear();
        }
    }
    customPlot_->replot();
}

void MultiLinePlot::plotSensorData(
    const QList<TimestampedSensorData> &dataList,
    const std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> &extractors)
{
    if (extractors.size() != buffers_.size()) {
        qDebug() << "Error: Number of extractors doesn't match number of graphs";
        return;
    }

    // Очищаем текущие данные
    clear();

    // Заполняем графики новыми данными
    for (size_t i = 0; i < extractors.size(); ++i) {
        QVector<double> timeData;
        QVector<double> valueData;

        for (const auto &data : dataList) {
            if (!extractors[i].second(data)) {
                continue;
            }
            double key = data.getTimestamp().toMSecsSinceEpoch() / 1000.0;
            double value = extractors[i].first(data);
            timeData.append(key);
            valueData.append(value);
        }

        customPlot_->graph(i)->setData(timeData, valueData);
    }

    // Обновляем отображение
    customPlot_->rescaleAxes();
    customPlot_->replot();
}

QList<QList<QPair<QDateTime, double>>> MultiLinePlot::getAllData()
{
    QList<QList<QPair<QDateTime, double>>> allData;
    
    for (size_t i = 0; i < buffers_.size(); ++i) {
        allData.append(buffers_[i]->getData());
    }
    
    return allData;
}

void MultiLinePlot::updatePlotSize(int newSize)
{
    customPlot_->setMinimumSize(newSize, newSize);
}

void MultiLinePlot::update()
{
    if (buffers_.empty()) {
        return;
    }

    for (size_t i = 0; i < buffers_.size(); ++i) {
        if (buffers_[i] && i < customPlot_->graphCount()) {
            QVector<double> timeData = buffers_[i]->getVisibleTimeData();
            QVector<double> valueData = buffers_[i]->getVisibleData();
            customPlot_->graph(i)->setData(timeData, valueData);
        }
    }

    customPlot_->rescaleAxes();
    customPlot_->replot();
}

void MultiLinePlot::updateBuffers(const std::vector<DynamicPlotBuffer*>& newBuffers)
{
    buffers_ = newBuffers;
    update();
} 
