#include "dynamicplotsgroup.h"

DynamicPlotsGroup::DynamicPlotsGroup(QWidget *parent)
    : QWidget(parent)
    , currentMode_(DisplayMode::SEPARATE_PLOTS)
    , multiLinePlot_(nullptr)
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
    plotLabels_.push_back(label);
    plotBufferSizes_.push_back(plotBufferSize);
    plotSizes_.push_back(plotSize);

    auto plot = new DynamicPlot(contentWidget_, plotBufferSize);
    plot->setPlotSize(plotSize);
    plot->setLabel(label);
    
    plots_.push_back(plot);
    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        contentLayout_->addWidget(plot);
    }
}

void DynamicPlotsGroup::clear()
{
    for (auto plot : plots_) {
        plot->clear();
    }
    if (multiLinePlot_) {
        multiLinePlot_->clear();
    }
}

void DynamicPlotsGroup::plotSensorData(
    const QList<TimestampedSensorData> &dataList,
    const std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> &extractors)
{
    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        for (size_t i = 0; i < plots_.size() && i < extractors.size(); ++i) {
            plots_[i]->plotSensorData(dataList, extractors[i].first, extractors[i].second);
        }
    } else if (currentMode_ == DisplayMode::COMBINED_PLOT && multiLinePlot_) {
        multiLinePlot_->plotSensorData(dataList, extractors);
    }
}

QList<QList<QPair<QDateTime, double>>> DynamicPlotsGroup::getAllData() const
{
    if (currentMode_ == DisplayMode::COMBINED_PLOT && multiLinePlot_) {
        return multiLinePlot_->getAllData();
    }

    QList<QList<QPair<QDateTime, double>>> result;
    for (const auto& plot : plots_) {
        result.append(plot->getData());
    }
    return result;
}

void DynamicPlotsGroup::updateLayout()
{
    QLayoutItem *child;
    while ((child = contentLayout_->takeAt(0)) != nullptr) {
        if (child->widget() != nullptr) {
            child->widget()->setParent(nullptr);
        }
        delete child;
    }

    switch (currentMode_) {
        case DisplayMode::SEPARATE_PLOTS:
            if (multiLinePlot_ != nullptr) {
                multiLinePlot_->setParent(nullptr);
                delete multiLinePlot_;
                multiLinePlot_ = nullptr;
            }
            for (auto plot : plots_) {
                contentLayout_->addWidget(plot);
            }
            break;

        case DisplayMode::COMBINED_PLOT:
            multiLinePlot_ = new MultiLinePlot(contentWidget_);
            for (size_t i = 0; i < plots_.size(); ++i) {
                multiLinePlot_->addGraph(plotLabels_[i], plotBufferSizes_[i], plotSizes_[i]);
            }
            contentLayout_->addWidget(multiLinePlot_);
            break;
    }
}

void DynamicPlotsGroup::addPoint(const QDateTime &timestamp, const std::vector<double> &values)
{
    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        for (size_t i = 0; i < plots_.size() && i < values.size(); ++i) {
            plots_[i]->addPoint(timestamp, values[i]);
        }
    } else if (currentMode_ == DisplayMode::COMBINED_PLOT && multiLinePlot_) {
        multiLinePlot_->addPoint(timestamp, values);
    }
}
