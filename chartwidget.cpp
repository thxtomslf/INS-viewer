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
#include <QTabWidget>
#include <QFileDialog>
#include <QDir>


ChartWidget::ChartWidget(InsCommandProcessor *serial,
                         std::shared_ptr<DynamicSetting<int>> plotBufferSize,
                         std::shared_ptr<DynamicSetting<int>> plotSize,
                         FileStorageManager *storageManager,
                         QWidget *parent)
    : RoutableWidget(parent), processor(serial), ui(new Ui::ChartWidget), isUartWidgetVisible(true), storageManager(storageManager)
{
    ui->setupUi(this);

    initUartWidget();
    initRangeSlider();
    initSplitter();
    initToggleUartButton();


    // Connect signals
    initStorageButtons();

    // Setup charts
    initCharts(plotBufferSize, plotSize);

    // Connect ToggleButton signals
    initStartToggleButton();
}

ChartWidget::~ChartWidget()
{
    stopShowData();
    delete ui;
    delete storageManager;
}

void ChartWidget::onPageHide() {

}
void ChartWidget::onPageShow(Page page) {

}

void ChartWidget::initRangeSlider() {
    rangeSlider = new RangeSlider(this);
    rangeSlider->setVisible(false); // Скрываем до загрузки файла
    ui->horizontalLayout->insertWidget(5, rangeSlider); // Добавляем слайдер в layout

    connect(rangeSlider, &RangeSlider::rangeChanged, this, &ChartWidget::loadDataForPeriod);
}

void ChartWidget::initUartWidget() {
    uartWidget = new UartWidget(processor);
    QVBoxLayout* uartLayout = new QVBoxLayout(ui->uartContainer);
    uartLayout->addWidget(uartWidget);
    uartLayout->setContentsMargins(0, 0, 0, 0);
}

void ChartWidget::initSplitter() {
    // Set initial splitter sizes (1/3 for UART, 2/3 for charts)
    QList<int> sizes;
    sizes << width() / 3 << (width() * 2) / 3;
    ui->mainSplitter->setSizes(sizes);
}

void ChartWidget::initToggleUartButton() {
    ui->toggleUartButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->toggleUartButton->setToolTip("Скрыть UART");
    connect(ui->toggleUartButton, &QPushButton::clicked, this, &ChartWidget::toggleUartWidget);
    connect(processor, &InsCommandProcessor::stopped, ui->toggleUartButton, &QPushButton::click);
}

void ChartWidget::initStartToggleButton() {
    connect(ui->startToggleButton, &ToggleButton::startSignal, this, &ChartWidget::showData);
    connect(ui->startToggleButton, &ToggleButton::pauseSignal, this, &ChartWidget::stopShowData);
    connect(ui->startToggleButton, &ToggleButton::stopSignal, this, &ChartWidget::handleStopSignal);
    connect(processor, &InsCommandProcessor::stopped, ui->startToggleButton, &ToggleButton::onPauseClicked);
    ui->startToggleButton->setVisible(false);
}

void ChartWidget::initStorageButtons() {
    connect(processor, &InsCommandProcessor::connectionStatusChanged, this, &ChartWidget::onUartConnectionChanged);
    connect(ui->saveToFileButton, &QPushButton::clicked, this, &ChartWidget::saveToFile);
    connect(ui->loadButton, &QPushButton::clicked, this, &ChartWidget::loadFromFile);

}

void ChartWidget::initCharts(std::shared_ptr<DynamicSetting<int>> plotBufferSize, std::shared_ptr<DynamicSetting<int>> plotSize) {
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
}

void ChartWidget::clearGraphs()
{
    for (auto plot : findChildren<DynamicPlot*>()) {
        plot->clear();
    }
}

void ChartWidget::updateGraphs(const SensorData &data, const QDateTime &timestamp)
{
    // Проверяем, что данные не пусты
    if (!data.getEnvironmentalMeasures().empty()) {
        ui->temperatureChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(0));
        ui->humidityChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(1));
        ui->pressureChart->addPoint(timestamp, data.getEnvironmentalMeasures().at(2));
    }

    if (!data.getAcceleroMeasures().empty()) {
        ui->acceleroChartX->addPoint(timestamp, data.getAcceleroMeasures().at(0));
        ui->acceleroChartY->addPoint(timestamp, data.getAcceleroMeasures().at(1));
        ui->acceleroChartZ->addPoint(timestamp, data.getAcceleroMeasures().at(2));
    }

    if (!data.getGyroMeasures().empty()) {
        ui->gyroChartX->addPoint(timestamp, data.getGyroMeasures().at(0));
        ui->gyroChartY->addPoint(timestamp, data.getGyroMeasures().at(1));
        ui->gyroChartZ->addPoint(timestamp, data.getGyroMeasures().at(2));
    }

    if (!data.getMagnetoMeasures().empty()) {
        ui->magnetoChartX->addPoint(timestamp, data.getMagnetoMeasures().at(0));
        ui->magnetoChartY->addPoint(timestamp, data.getMagnetoMeasures().at(1));
        ui->magnetoChartZ->addPoint(timestamp, data.getMagnetoMeasures().at(2));
    }

    ui->writeSpeedLabel->setText(QString::number(data.getDataSendCount()));
    ui->readSpeedLabel->setText(QString::number(processor->getFrequency()));
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
    setMode(ChartWidget::WidgetMode::UART);

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
    });
}

void ChartWidget::stopShowData()
{
    processor->interrupt();
}

void ChartWidget::onUartConnectionChanged(bool connected)
{
    ui->startToggleButton->setVisible(connected);
    toggleUartWidget();
}

void ChartWidget::saveToFile()
{

    // Collect data from all charts
    QList<TimestampedSensorData> allData;

    // Environment data
    QList<QPair<QDateTime, double>> temperatureData = ui->temperatureChart->getData();
    QList<QPair<QDateTime, double>> humidityData = ui->humidityChart->getData();
    QList<QPair<QDateTime, double>> pressureData = ui->pressureChart->getData();

    // Acceleration data
    QList<QPair<QDateTime, double>> acceleroXData = ui->acceleroChartX->getData();
    QList<QPair<QDateTime, double>> acceleroYData = ui->acceleroChartY->getData();
    QList<QPair<QDateTime, double>> acceleroZData = ui->acceleroChartZ->getData();

    // Gyroscope data
    QList<QPair<QDateTime, double>> gyroXData = ui->gyroChartX->getData();
    QList<QPair<QDateTime, double>> gyroYData = ui->gyroChartY->getData();
    QList<QPair<QDateTime, double>> gyroZData = ui->gyroChartZ->getData();

    // Magnetometer data
    QList<QPair<QDateTime, double>> magnetoXData = ui->magnetoChartX->getData();
    QList<QPair<QDateTime, double>> magnetoYData = ui->magnetoChartY->getData();
    QList<QPair<QDateTime, double>> magnetoZData = ui->magnetoChartZ->getData();

    // Check if all data lists have the same size
    if (temperatureData.size() != humidityData.size() || temperatureData.size() != pressureData.size() ||
        temperatureData.size() != acceleroXData.size() || temperatureData.size() != acceleroYData.size() ||
        temperatureData.size() != acceleroZData.size() || temperatureData.size() != gyroXData.size() ||
        temperatureData.size() != gyroYData.size() || temperatureData.size() != gyroZData.size() ||
        temperatureData.size() != magnetoXData.size() || temperatureData.size() != magnetoYData.size() ||
        temperatureData.size() != magnetoZData.size()) {
        qDebug() << "Error: Data sizes do not match!";
        return;
    }

    // Combine data into TimestampedSensorData objects
    for (int i = 0; i < temperatureData.size(); ++i) {
        TimestampedSensorData data;
        data.setTimestamp(temperatureData[i].first);
        data.setEnvironmentalMeasures({
            static_cast<float>(temperatureData[i].second),
            static_cast<float>(humidityData[i].second),
            static_cast<float>(pressureData[i].second)
        });
        data.setAcceleroMeasures({
            static_cast<int16_t>(acceleroXData[i].second),
            static_cast<int16_t>(acceleroYData[i].second),
            static_cast<int16_t>(acceleroZData[i].second)
        });
        data.setGyroMeasures({
            static_cast<int16_t>(gyroXData[i].second),
            static_cast<int16_t>(gyroYData[i].second),
            static_cast<int16_t>(gyroZData[i].second)
        });
        data.setMagnetoMeasures({
            static_cast<int16_t>(magnetoXData[i].second),
            static_cast<int16_t>(magnetoYData[i].second),
            static_cast<int16_t>(magnetoZData[i].second)
        });
        allData.append(data);
    }

    storageManager->openFileToSave();
    // Write data to CSV
    for (const auto& data : allData) {
        storageManager->saveData(data);
    }

    QMessageBox::information(this, "Статус записи", QString("Экперимент успешно сохранен по пути:\n %1").arg(storageManager->getSaveFileName()));
}

void ChartWidget::loadFromFile() {
    try {
        // Получаем все данные из файла
        storageManager->loadFile(this);
        if (storageManager->getReadFileName().isEmpty()) {
            return;
        }

        QList<TimestampedSensorData> allData = storageManager->loadAllData();
        if (allData.isEmpty()) {
            throw std::runtime_error("Файл не содержит данных.");
        }

        // Получаем минимальный и максимальный timestamp
        minTimestamp = allData.first().getTimestamp();
        maxTimestamp = allData.last().getTimestamp();

        // Устанавливаем диапазон слайдера
        rangeSlider->setRange(minTimestamp, maxTimestamp);

        loadDataForPeriod(minTimestamp, maxTimestamp);

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Ошибка", QString("Не удалось загрузить файл: %1").arg(e.what()));
        return;
    }

    setMode(ChartWidget::WidgetMode::FILE);
}

void ChartWidget::setMode(WidgetMode mode) {
    if (mode == ChartWidget::WidgetMode::UART) {
        rangeSlider->setVisible(false);
        ui->currentFileLabel->setVisible(false);
        ui->label->setVisible(true);
        ui->label_2->setVisible(true);
        ui->readSpeedLabel->setVisible(true);
        ui->writeSpeedLabel->setVisible(true);
        ui->currentFileLabel->setVisible(false);
    } else {
        ui->label->setVisible(false);
        ui->label_2->setVisible(false);
        ui->readSpeedLabel->setVisible(false);
        ui->writeSpeedLabel->setVisible(false);

        rangeSlider->setVisible(true);
        ui->currentFileLabel->setVisible(true);

        ui->startToggleButton->onPauseClicked();

        ui->currentFileLabel->setText(storageManager->getReadFileName());

        rangeSlider->setVisible(true); // Показываем слайдер
    }
}


void ChartWidget::loadDataForPeriod(const QDateTime &start, const QDateTime &end) {
    // Очищаем графики
    clearGraphs();


    QList<TimestampedSensorData> dataList = storageManager->loadDataForPeriod(start, end);

    ui->temperatureChart->plotSensorData(dataList,
     [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(0);
    },
    [](const TimestampedSensorData &data) {
        return !data.getEnvironmentalMeasures().empty();
    });

    ui->humidityChart->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(1);
    },
    [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().size() > 1;
    });

    ui->pressureChart->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().at(2);
    },
    [](const TimestampedSensorData &data) {
        return data.getEnvironmentalMeasures().size() > 2;
    });

    ui->acceleroChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(0);
    },
    [](const TimestampedSensorData &data) {
        return !data.getAcceleroMeasures().empty();
    });

    ui->acceleroChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(1);
    },
    [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().size() > 1;
    });

    ui->acceleroChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().at(2);
    },
    [](const TimestampedSensorData &data) {
        return data.getAcceleroMeasures().size() > 2;
    });

    ui->gyroChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(0);
    },
    [](const TimestampedSensorData &data) {
        return !data.getGyroMeasures().empty();
    });

    ui->gyroChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(1);
    },
    [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().size() > 1;
    });

    ui->gyroChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().at(2);
    },
    [](const TimestampedSensorData &data) {
        return data.getGyroMeasures().size() > 2;
    });

    ui->magnetoChartX->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(0);
    },
    [](const TimestampedSensorData &data) {
        return !data.getMagnetoMeasures().empty();
    });

    ui->magnetoChartY->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(1);
    },
    [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().size() > 1;
    });

    ui->magnetoChartZ->plotSensorData(dataList, [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().at(2);
    },
    [](const TimestampedSensorData &data) {
        return data.getMagnetoMeasures().size() > 2;
    });
}
