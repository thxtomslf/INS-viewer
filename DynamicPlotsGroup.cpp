#include "dynamicplotsgroup.h"

DynamicPlotsGroup::DynamicPlotsGroup(QWidget *parent)
    : QWidget(parent)
    , currentMode_(DisplayMode::SEPARATE_PLOTS)
{
    setupLayout();
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
    auto plot = new DynamicPlot(contentWidget_, plotBufferSize);
    plot->setPlotSize(plotSize);
    plot->setLabel(label);
    
    plots_.push_back(plot);
    contentLayout_->addWidget(plot);
}

void DynamicPlotsGroup::clear()
{
    for (auto plot : plots_) {
        plot->clear();
    }
}

void DynamicPlotsGroup::plotSensorData(
    const QList<TimestampedSensorData> &dataList,
    const std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> &extractors)
{
    if (extractors.size() != plots_.size()) {
        qDebug() << "Error: Number of extractors doesn't match number of plots";
        return;
    }

    for (size_t i = 0; i < plots_.size(); ++i) {
        plots_[i]->plotSensorData(dataList, extractors[i].first, extractors[i].second);
    }
}

QList<QList<QPair<QDateTime, double>>> DynamicPlotsGroup::getAllData() const
{
    QList<QList<QPair<QDateTime, double>>> allData;
    for (const auto plot : plots_) {
        allData.append(plot->getData());
    }
    return allData;
}

void DynamicPlotsGroup::updateLayout()
{
    // В будущем здесь будет логика для разных режимов отображения
    switch (currentMode_) {
        case DisplayMode::SEPARATE_PLOTS:
            // Текущая реализация уже соответствует этому режиму
            break;
    }
}

void DynamicPlotsGroup::addPoint(const QDateTime &timestamp, const std::vector<double> &values)
{
    if (values.size() != plots_.size()) {
        qDebug() << "Error: Number of values doesn't match number of plots";
        return;
    }

    for (size_t i = 0; i < plots_.size(); ++i) {
        plots_[i]->addPoint(timestamp, values[i]);
    }
} 