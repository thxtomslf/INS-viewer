#include "ui_chartwidget.h"

#include "chartwidget.h"
#include "dynamicplot.h"
#include "comand/commandresponse.h"
#include "pagerouter.h"
#include "uartwidget.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QStyle>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>


ChartWidget::ChartWidget(InsCommandProcessor *serial,
                         SensorDataDAO *dao,
                         std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                         std::shared_ptr<DynamicSetting<int>> plotSize,
                         QWidget *parent)
    : RoutableWidget(parent), processor(serial), dao(dao), ui(new Ui::ChartWidget), isUartWidgetVisible(true)
{
    ui->setupUi(this);

    // Initialize UartWidget
    uartWidget = new UartWidget(processor);
    QVBoxLayout* uartLayout = new QVBoxLayout(ui->uartContainer);
    uartLayout->addWidget(uartWidget);
    uartLayout->setContentsMargins(0, 0, 0, 0);

    // Set initial splitter sizes (1/3 for UART, 2/3 for charts)
    QList<int> sizes;
    sizes << width() / 3 << (width() * 2) / 3;
    ui->mainSplitter->setSizes(sizes);

    // Setup toggle button
    ui->toggleUartButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->toggleUartButton->setToolTip("Скрыть UART");

    // Connect signals
    connect(ui->toggleUartButton, &QPushButton::clicked, this, &ChartWidget::toggleUartWidget);
    connect(processor, &InsCommandProcessor::connectionStatusChanged, this, &ChartWidget::onUartConnectionChanged);

    // Setup charts
    ui->temperatureChart->setLabel("Температура, °C");
    ui->temperatureChart->setMaxBufferSize(plotBufferSize);
    ui->temperatureChart->setPlotSize(plotSize);

    ui->humidityChart->setLabel("Влажность, %");
    ui->humidityChart->setMaxBufferSize(plotBufferSize);
    ui->humidityChart->setPlotSize(plotSize);

    ui->pressureChart->setLabel("Давление, Па");
    ui->pressureChart->setMaxBufferSize(plotBufferSize);
    ui->pressureChart->setPlotSize(plotSize);

    ui->acceleroChartX->setLabel("Линейное ускорение X, м/c^2");
    ui->acceleroChartX->setMaxBufferSize(plotBufferSize);
    ui->acceleroChartX->setPlotSize(plotSize);

    ui->acceleroChartY->setLabel("Линейное ускорение Y, м/c^2");
    ui->acceleroChartY->setMaxBufferSize(plotBufferSize);
    ui->acceleroChartY->setPlotSize(plotSize);

    ui->acceleroChartZ->setLabel("Линейное ускорение Z, м/c^2");
    ui->acceleroChartZ->setMaxBufferSize(plotBufferSize);
    ui->acceleroChartZ->setPlotSize(plotSize);

    ui->gyroChartX->setLabel("Угловая скорость X, рад/c");
    ui->gyroChartX->setMaxBufferSize(plotBufferSize);
    ui->gyroChartX->setPlotSize(plotSize);

    ui->gyroChartY->setLabel("Угловая скорость Y, рад/c");
    ui->gyroChartY->setMaxBufferSize(plotBufferSize);
    ui->gyroChartY->setPlotSize(plotSize);

    ui->gyroChartZ->setLabel("Угловая скорость Z, рад/c");
    ui->gyroChartZ->setMaxBufferSize(plotBufferSize);
    ui->gyroChartZ->setPlotSize(plotSize);

    ui->magnetoChartX->setLabel("Магнитная индукция X, Гc");
    ui->magnetoChartX->setMaxBufferSize(plotBufferSize);
    ui->magnetoChartX->setPlotSize(plotSize);

    ui->magnetoChartY->setLabel("Магнитная индукция Y, Гc");
    ui->magnetoChartY->setMaxBufferSize(plotBufferSize);
    ui->magnetoChartY->setPlotSize(plotSize);

    ui->magnetoChartZ->setLabel("Магнитная индукция Z, Гc");
    ui->magnetoChartZ->setMaxBufferSize(plotBufferSize);
    ui->magnetoChartZ->setPlotSize(plotSize);

    ui->navigatorButton->setObjects({ui->envScrollArea, ui->acceleroScrollArea, ui->gyroScrollArea, ui->magnetoScrollArea});

    // Connect ToggleButton signals
    connect(ui->startToggleButton, &ToggleButton::startSignal, this, &ChartWidget::showData);
    connect(ui->startToggleButton, &ToggleButton::pauseSignal, this, &ChartWidget::stopShowData);
    connect(ui->startToggleButton, &ToggleButton::stopSignal, this, &ChartWidget::handleStopSignal);

    connect(ui->loadFromDbButton, &QPushButton::clicked, this, &ChartWidget::loadDataForPeriod);

    connect(processor, &InsCommandProcessor::stopped, ui->startToggleButton, &ToggleButton::onPauseClicked);

    ui->startRangeWidget->setDateTime(QDateTime::currentDateTime());
    ui->endRangeWidget->setDateTime(QDateTime::currentDateTime().addMSecs(10000));

    // Initialize controls visibility
    updateControlsVisibility(false);
}

ChartWidget::~ChartWidget()
{
    stopShowData();
    delete ui;
}

void ChartWidget::onPageHide() {

}
void ChartWidget::onPageShow(Page page) {

}

void ChartWidget::clearGraphs()
{
    for (auto plot : findChildren<DynamicPlot*>()) {
        plot->clear();
    }
}

void ChartWidget::updateGraphs(const SensorData &data, const QDateTime &timestamp)
{
    ui->temperatureChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(0));
    ui->humidityChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(1));
    ui->pressureChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(2));

    ui->acceleroChartX->addPoint(timestamp, data.getAcceleroMeasures().at(0));
    ui->acceleroChartY->addPoint(timestamp, data.getAcceleroMeasures().at(1));
    ui->acceleroChartZ->addPoint(timestamp, data.getAcceleroMeasures().at(2));

    ui->gyroChartX->addPoint(timestamp, data.getGyroMeasures().at(0));
    ui->gyroChartY->addPoint(timestamp, data.getGyroMeasures().at(1));
    ui->gyroChartZ->addPoint(timestamp, data.getGyroMeasures().at(2));

    ui->magnetoChartX->addPoint(timestamp, data.getMagnetoMeasures().at(0));
    ui->magnetoChartY->addPoint(timestamp, data.getMagnetoMeasures().at(1));
    ui->magnetoChartZ->addPoint(timestamp, data.getMagnetoMeasures().at(2));

    ui->writeSpeedLabel->setText(QString::number(data.getDataSendCount()));
    ui->readSpeedLabel->setText(QString::number(processor->getFrequency()));
}


void ChartWidget::loadDataForPeriod()
{
    clearGraphs();

    QDateTime start = ui->startRangeWidget->dateTime();
    QDateTime end = ui->endRangeWidget->dateTime();
    QList<TimestampedSensorData> dataList = dao->selectSensorData(start, end);

    ui->temperatureChart->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(0);
    });

    ui->humidityChart->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(1);
    });

    ui->pressureChart->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(2);
    });

    ui->acceleroChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(0);
    });

    ui->acceleroChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(1);
    });

    ui->acceleroChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(2);
    });

    ui->gyroChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(0);
    });

    ui->gyroChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(1);
    });

    ui->gyroChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(2);
    });

    ui->magnetoChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(0);
    });

    ui->magnetoChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(1);
    });

    ui->magnetoChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(2);
    });
}

void ChartWidget::toggleUartWidget()
{
    isUartWidgetVisible = !isUartWidgetVisible;
    
    // Update button icon and tooltip
    if (isUartWidgetVisible) {
        ui->toggleUartButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
        ui->toggleUartButton->setToolTip("Скрыть UART");
        
        // Restore the original splitter sizes
        QList<int> sizes;
        sizes << width() / 3 << (width() * 2) / 3;
        ui->mainSplitter->setSizes(sizes);
        
        // Show all UART controls
        uartWidget->setExpanded(true);
    } else {
        ui->toggleUartButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
        ui->toggleUartButton->setToolTip("Показать UART");
        
        // Minimize the left side
        QList<int> sizes;
        sizes << 100 << width() - 100;
        ui->mainSplitter->setSizes(sizes);
        
        // Collapse UART controls
        uartWidget->setExpanded(false);
    }
}

void ChartWidget::handleStopSignal()
{
    stopShowData();
    clearGraphs();
}

void ChartWidget::showData()
{
    processor->readData([this](const QByteArray &data) {

        CommandResponse<SensorData> response(data);
        if (response.getResponseType() == CommandResponse<SensorData>::BAD_RESPONSE) {
            qDebug() << "Bad response: " << response.getError();
            return;
        }

        if (response.getResponseType() == CommandResponse<SensorData>::CRC_FAIL) {
            qDebug() << "CRC check failed: " << response.getError();
            return;
        }

        if (response.getResponseType() == CommandResponse<SensorData>::Rejected) {
            qDebug() << "Rejected";
            return;
        }

        updateGraphs(response.getMessageBody(), QDateTime::currentDateTime());

        dao->insertSensorData(response.getMessageBody());
    });
}

void ChartWidget::stopShowData()
{
    processor->interrupt();
}

void ChartWidget::onUartConnectionChanged(bool connected)
{
    // Implementation of the method
}

void ChartWidget::updateControlsVisibility(bool visible)
{
    // Implementation of the method
}
