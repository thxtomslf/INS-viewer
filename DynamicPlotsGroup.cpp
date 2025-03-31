#include "DynamicPlotsGroup.h"


DynamicPlotsGroup::DynamicPlotsGroup(QWidget *parent, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting)
    : QWidget(parent)
    , maxBufferSizeSetting_(maxBufferSizeSetting)
{
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop); 
    plots_ = QMap<PlotType, DynamicPlot*>();
}

DynamicPlot* DynamicPlotsGroup::addPlot(const PlotType& plotType, std::shared_ptr<DynamicSetting<int>> maxBufferSizeSetting) {
    DynamicPlot* plot = new DynamicPlot(this, maxBufferSizeSetting);
    plot->setLabel(plotType);
    layout->addWidget(plot);
    plots_[plotType] = plot;
    return plot;
}

QList<QPair<QDateTime, double>> DynamicPlotsGroup::getData(const PlotType& plotType) const {
    return plots_[plotType]->getData();
}
    
DynamicPlotsGroup::~DynamicPlotsGroup() {
    delete layout;
}
