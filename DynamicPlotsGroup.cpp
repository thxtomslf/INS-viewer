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

    dataBuffers_.emplace_back(plotBufferSize);

    auto plot = new DynamicPlot(contentWidget_, plotBufferSize);
    plot->setPlotSize(plotSize);
    plot->setLabel(label);
    
    plots_.push_back(plot);
    if (!multiLinePlot_) {
        multiLinePlot_ = new MultiLinePlot(contentWidget_);
    }
    multiLinePlot_->addGraph(label, plotBufferSize, plotSize);

    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        contentLayout_->addWidget(plot);
    }
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

    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        if (multiLinePlot_) {
            multiLinePlot_->setParent(nullptr);
        }
        for (auto plot : plots_) {
            contentLayout_->addWidget(plot);
        }
    } else {
        for (auto plot : plots_) {
            plot->setParent(nullptr);
        }
        contentLayout_->addWidget(multiLinePlot_);
    }

    updateDisplayedData();
}

void DynamicPlotsGroup::updateDisplayedData()
{
    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        for (size_t i = 0; i < plots_.size(); ++i) {
            plots_[i]->updateFromBuffer(dataBuffers_[i]);
        }
    } else if (multiLinePlot_) {
        multiLinePlot_->updateFromBuffers(dataBuffers_);
    }
}

void DynamicPlotsGroup::addPoint(const QDateTime &timestamp, const std::vector<double> &values)
{
    if (values.size() != dataBuffers_.size()) {
        qDebug() << "Error: Number of values doesn't match number of buffers";
        return;
    }

    for (size_t i = 0; i < dataBuffers_.size(); ++i) {
        dataBuffers_[i].addPoint(timestamp, values[i]);
    }

    if (currentMode_ == DisplayMode::SEPARATE_PLOTS) {
        for (size_t i = 0; i < plots_.size(); ++i) {
            plots_[i]->updateFromBuffer(dataBuffers_[i]);
        }
    } else if (multiLinePlot_) {
        multiLinePlot_->updateFromBuffers(dataBuffers_);
    }
}
