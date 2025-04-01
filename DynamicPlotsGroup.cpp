#include "dynamicplotsgroup.h"

DynamicPlotsGroup::DynamicPlotsGroup(QWidget *parent)
    : QWidget(parent)
    , currentMode_(DisplayMode::TABLE_VIEW)
    , multiLinePlot_(nullptr)
{
    setupLayout();
    tableWidget_ = new DataTableWidget(contentWidget_);
    contentLayout_->addWidget(tableWidget_);
}

void DynamicPlotsGroup::setupLayout()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    scrollArea_ = new QScrollArea(this);
    contentWidget_ = new QWidget(scrollArea_);
    contentLayout_ = new QVBoxLayout(contentWidget_);
    
    scrollArea_->setWidget(contentWidget_);
    scrollArea_->setWidgetResizable(true);
    
    mainLayout->addWidget(scrollArea_);
}

void DynamicPlotsGroup::setMode(DisplayMode mode)
{
    if (currentMode_ != mode) {
        currentMode_ = mode;
        updateLayout();
    }
}

void DynamicPlotsGroup::addPlot(const QString &label,
                               std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                               std::shared_ptr<DynamicSetting<int>> plotSize)
{
    plotLabels_.push_back(label);
    plotBufferSizes_.push_back(plotBufferSize);
    plotSizes_.push_back(plotSize);

    dataBuffers_.emplace_back(plotBufferSize);

    tableWidget_->addDataColumn(label, plotBufferSize);

    auto plot = new DynamicPlot(contentWidget_, plotBufferSize);
    plot->setPlotSize(plotSize);
    plot->setLabel(label);
    
    plots_.push_back(plot);
    if (!multiLinePlot_) {
        multiLinePlot_ = new MultiLinePlot(contentWidget_);
    }
    multiLinePlot_->addGraph(label, plotBufferSize, plotSize);

    updateLayout();
}

void DynamicPlotsGroup::clear()
{
    for (auto &buffer : dataBuffers_) {
        buffer.clear();
    }
    
    updateDisplayedData();
}

void DynamicPlotsGroup::plotSensorData(
    const QList<TimestampedSensorData> &dataList,
    const std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> &extractors)
{
    for (auto &buffer : dataBuffers_) {
        buffer.clear();
    }

    for (const auto &data : dataList) {
        for (size_t i = 0; i < extractors.size() && i < dataBuffers_.size(); ++i) {
            if (extractors[i].second(data)) {
                dataBuffers_[i].addPoint(data.getTimestamp(), extractors[i].first(data));
            }
        }
    }

    updateDisplayedData();
}

QList<QList<QPair<QDateTime, double>>> DynamicPlotsGroup::getAllData() const
{
    QList<QList<QPair<QDateTime, double>>> result;
    QTimeZone timeZone = QTimeZone::utc();
    
    // Преобразуем данные из каждого буфера
    for (const auto& buffer : dataBuffers_) {
        QList<QPair<QDateTime, double>> plotData;
        
        // Получаем все временные метки и значения из буфера
        QVector<double> times = buffer.getVisibleTimeData();
        QVector<double> values = buffer.getVisibleData();
        
        // Преобразуем временные метки из Unix timestamp в QDateTime
        for (int i = 0; i < times.size(); ++i) {
            qint64 seconds = static_cast<qint64>(times[i]);
            int milliseconds = static_cast<int>((times[i] - seconds) * 1000);

            // Создаем QDateTime из секунд и добавляем миллисекунды
            QDateTime timestamp = QDateTime::fromSecsSinceEpoch(seconds, timeZone);
            timestamp = timestamp.addMSecs(milliseconds);

            plotData.append(qMakePair(timestamp, values[i]));
        }
        
        result.append(plotData);
    }
    
    return result;
}

void DynamicPlotsGroup::updateLayout()
{
    // Скрываем все виджеты
    if (tableWidget_) tableWidget_->hide();
    if (multiLinePlot_) multiLinePlot_->hide();
    for (auto plot : plots_) {
        plot->hide();
    }

    // Очищаем текущий layout
    QLayoutItem *child;
    while ((child = contentLayout_->takeAt(0)) != nullptr) {
        delete child;
    }

    // Показываем нужный виджет
    switch (currentMode_) {
        case TABLE_VIEW:
            if (tableWidget_) {
                tableWidget_->show();
                contentLayout_->addWidget(tableWidget_);
            }
            break;
        case SEPARATE_PLOTS:
            for (auto plot : plots_) {
                plot->show();
                contentLayout_->addWidget(plot);
            }
            break;
        case COMBINED_PLOT:
            if (multiLinePlot_) {
                multiLinePlot_->show();
                contentLayout_->addWidget(multiLinePlot_);
            }
            break;
    }

    // Обновляем данные в текущем виджете
    updateDisplayedData();
}

void DynamicPlotsGroup::updateDisplayedData()
{
    switch (currentMode_) {
        case TABLE_VIEW:
            if (tableWidget_) {
                tableWidget_->updateFromBuffers(dataBuffers_);
            }
            break;
        case SEPARATE_PLOTS:
            for (size_t i = 0; i < plots_.size(); ++i) {
                plots_[i]->updateFromBuffer(dataBuffers_[i]);
            }
            break;
        case COMBINED_PLOT:
            if (multiLinePlot_) {
                multiLinePlot_->updateFromBuffers(dataBuffers_);
            }
            break;
    }
}

void DynamicPlotsGroup::addPoint(const QDateTime &timestamp, const std::vector<double> &values)
{
    if (values.size() != dataBuffers_.size()) {
        qDebug() << "Error: Number of values doesn't match number of buffers";
        return;
    }

    // Добавляем данные в буферы
    for (size_t i = 0; i < dataBuffers_.size(); ++i) {
        dataBuffers_[i].addPoint(timestamp, values[i]);
    }

    // Обновляем отображение в зависимости от текущего режима
    switch (currentMode_) {
        case TABLE_VIEW:
            if (tableWidget_) {
                tableWidget_->addPoint(timestamp, values);
            }
            break;
        case SEPARATE_PLOTS:
            for (size_t i = 0; i < plots_.size(); ++i) {
                plots_[i]->updateFromBuffer(dataBuffers_[i]);
            }
            break;
        case COMBINED_PLOT:
            if (multiLinePlot_) {
                multiLinePlot_->updateFromBuffers(dataBuffers_);
            }
            break;
    }
}
