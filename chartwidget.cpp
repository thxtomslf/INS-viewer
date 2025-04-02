#include "ui_chartwidget.h"

#include "chartwidget.h"
#include "dynamicplot.h"
#include "comand/commandresponse.h"
#include "pagerouter.h"
#include "uartwidget.h"
#include "dynamicplotsgroup.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QStyle>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QTabWidget>
#include <QFileDialog>
#include <QDir>
#include <QButtonGroup>


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
    initDisplayModeButtons();

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

void ChartWidget::initCharts(std::shared_ptr<DynamicSetting<int>> plotBufferSize, std::shared_ptr<DynamicSetting<int>> plotSize)
{
    // Создаем группы графиков
    envGroup_ = new DynamicPlotsGroup(this);
    envGroup_->addPlot("Температура, °C", plotBufferSize, plotSize);
    envGroup_->addPlot("Влажность, %", plotBufferSize, plotSize);
    envGroup_->addPlot("Давление, Па", plotBufferSize, plotSize);
    ui->verticalLayoutEnv_2->addWidget(envGroup_);

    acceleroGroup_ = new DynamicPlotsGroup(this);
    acceleroGroup_->addPlot("Линейное ускорение X, м/c^2", plotBufferSize, plotSize);
    acceleroGroup_->addPlot("Линейное ускорение Y, м/c^2", plotBufferSize, plotSize);
    acceleroGroup_->addPlot("Линейное ускорение Z, м/c^2", plotBufferSize, plotSize);
    ui->verticalLayoutAccelero_2->addWidget(acceleroGroup_);

    gyroGroup_ = new DynamicPlotsGroup(this);
    gyroGroup_->addPlot("Угловая скорость X, рад/c", plotBufferSize, plotSize);
    gyroGroup_->addPlot("Угловая скорость Y, рад/c", plotBufferSize, plotSize);
    gyroGroup_->addPlot("Угловая скорость Z, рад/c", plotBufferSize, plotSize);
    ui->verticalLayoutGyro_2->addWidget(gyroGroup_);

    magnetoGroup_ = new DynamicPlotsGroup(this);
    magnetoGroup_->addPlot("Магнитная индукция X, Гc", plotBufferSize, plotSize);
    magnetoGroup_->addPlot("Магнитная индукция Y, Гc", plotBufferSize, plotSize);
    magnetoGroup_->addPlot("Магнитная индукция Z, Гc", plotBufferSize, plotSize);
    ui->verticalLayoutMagneto_2->addWidget(magnetoGroup_);
}

void ChartWidget::clearGraphs()
{
    envGroup_->clear();
    acceleroGroup_->clear();
    gyroGroup_->clear();
    magnetoGroup_->clear();
}

void ChartWidget::updateGraphs(const SensorData &data, const QDateTime &timestamp)
{
    if (!data.getEnvironmentalMeasures().empty()) {
        std::vector<double> envValues;
        for (const auto &value : data.getEnvironmentalMeasures()) {
            envValues.push_back(value);
        }
        envGroup_->addPoint(timestamp, envValues);
    }

    if (!data.getAcceleroMeasures().empty()) {
        std::vector<double> acceleroValues;
        for (const auto &value : data.getAcceleroMeasures()) {
            acceleroValues.push_back(value);
        }
        acceleroGroup_->addPoint(timestamp, acceleroValues);
    }

    if (!data.getGyroMeasures().empty()) {
        std::vector<double> gyroValues;
        for (const auto &value : data.getGyroMeasures()) {
            gyroValues.push_back(value);
        }
        gyroGroup_->addPoint(timestamp, gyroValues);
    }

    if (!data.getMagnetoMeasures().empty()) {
        std::vector<double> magnetoValues;
        for (const auto &value : data.getMagnetoMeasures()) {
            magnetoValues.push_back(value);
        }
        magnetoGroup_->addPoint(timestamp, magnetoValues);
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
    QList<QPair<QDateTime, double>> temperatureData = envGroup_->getAllData().at(0);
    QList<QPair<QDateTime, double>> humidityData = envGroup_->getAllData().at(1);
    QList<QPair<QDateTime, double>> pressureData = envGroup_->getAllData().at(2);

    // Acceleration data
    QList<QPair<QDateTime, double>> acceleroXData = acceleroGroup_->getAllData().at(0);
    QList<QPair<QDateTime, double>> acceleroYData = acceleroGroup_->getAllData().at(1);
    QList<QPair<QDateTime, double>> acceleroZData = acceleroGroup_->getAllData().at(2);

    // Gyroscope data
    QList<QPair<QDateTime, double>> gyroXData = gyroGroup_->getAllData().at(0);
    QList<QPair<QDateTime, double>> gyroYData = gyroGroup_->getAllData().at(1);
    QList<QPair<QDateTime, double>> gyroZData = gyroGroup_->getAllData().at(2);

    // Magnetometer data
    QList<QPair<QDateTime, double>> magnetoXData = magnetoGroup_->getAllData().at(0);
    QList<QPair<QDateTime, double>> magnetoYData = magnetoGroup_->getAllData().at(1);
    QList<QPair<QDateTime, double>> magnetoZData = magnetoGroup_->getAllData().at(2);

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
        storageManager->loadFile(this);
        if (storageManager->getReadFileName().isEmpty()) {
            return;
        }

        QList<TimestampedSensorData> allData = storageManager->loadAllData();
        if (allData.isEmpty()) {
            throw std::runtime_error("Файл не содержит данных.");
        }

        minTimestamp = allData.first().getTimestamp();
        maxTimestamp = allData.last().getTimestamp();

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
        rangeSlider->setVisible(true);
    }
}

void ChartWidget::loadDataForPeriod(const QDateTime &start, const QDateTime &end) {
    clearGraphs();

    QList<TimestampedSensorData> dataList = storageManager->loadDataForPeriod(start, end);

    // Environmental data
    std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> envExtractors = {
        {[](const TimestampedSensorData &d) { return d.getEnvironmentalMeasures().at(0); },
         [](const TimestampedSensorData &d) { return !d.getEnvironmentalMeasures().empty(); }},
        {[](const TimestampedSensorData &d) { return d.getEnvironmentalMeasures().at(1); },
         [](const TimestampedSensorData &d) { return d.getEnvironmentalMeasures().size() > 1; }},
        {[](const TimestampedSensorData &d) { return d.getEnvironmentalMeasures().at(2); },
         [](const TimestampedSensorData &d) { return d.getEnvironmentalMeasures().size() > 2; }}
    };
    envGroup_->plotSensorData(dataList, envExtractors);

    // Accelerometer data
    std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> acceleroExtractors = {
        {[](const TimestampedSensorData &d) { return d.getAcceleroMeasures().at(0); },
         [](const TimestampedSensorData &d) { return !d.getAcceleroMeasures().empty(); }},
        {[](const TimestampedSensorData &d) { return d.getAcceleroMeasures().at(1); },
         [](const TimestampedSensorData &d) { return d.getAcceleroMeasures().size() > 1; }},
        {[](const TimestampedSensorData &d) { return d.getAcceleroMeasures().at(2); },
         [](const TimestampedSensorData &d) { return d.getAcceleroMeasures().size() > 2; }}
    };
    acceleroGroup_->plotSensorData(dataList, acceleroExtractors);

    // Gyroscope data
    std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> gyroExtractors = {
        {[](const TimestampedSensorData &d) { return d.getGyroMeasures().at(0); },
         [](const TimestampedSensorData &d) { return !d.getGyroMeasures().empty(); }},
        {[](const TimestampedSensorData &d) { return d.getGyroMeasures().at(1); },
         [](const TimestampedSensorData &d) { return d.getGyroMeasures().size() > 1; }},
        {[](const TimestampedSensorData &d) { return d.getGyroMeasures().at(2); },
         [](const TimestampedSensorData &d) { return d.getGyroMeasures().size() > 2; }}
    };
    gyroGroup_->plotSensorData(dataList, gyroExtractors);

    // Magnetometer data
    std::vector<std::pair<
        std::function<double(const TimestampedSensorData&)>,
        std::function<bool(const TimestampedSensorData&)>
    >> magnetoExtractors = {
        {[](const TimestampedSensorData &d) { return d.getMagnetoMeasures().at(0); },
         [](const TimestampedSensorData &d) { return !d.getMagnetoMeasures().empty(); }},
        {[](const TimestampedSensorData &d) { return d.getMagnetoMeasures().at(1); },
         [](const TimestampedSensorData &d) { return d.getMagnetoMeasures().size() > 1; }},
        {[](const TimestampedSensorData &d) { return d.getMagnetoMeasures().at(2); },
         [](const TimestampedSensorData &d) { return d.getMagnetoMeasures().size() > 2; }}
    };
    magnetoGroup_->plotSensorData(dataList, magnetoExtractors);
}

void ChartWidget::initDisplayModeButtons()
{
    // Создаем группу кнопок
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->separatePlotsButton);
    buttonGroup->addButton(ui->combinedPlotButton);
    buttonGroup->addButton(ui->tableViewButton);

    // Подключаем сигналы
    connect(ui->separatePlotsButton, &QPushButton::clicked, this, &ChartWidget::onDisplayModeChanged);
    connect(ui->combinedPlotButton, &QPushButton::clicked, this, &ChartWidget::onDisplayModeChanged);
    connect(ui->tableViewButton, &QPushButton::clicked, this, &ChartWidget::onDisplayModeChanged);

    // Устанавливаем стили для кнопок
    QString buttonStyle = R"(
        QPushButton {
            padding: 5px;
            border: 1px solid palette(mid);
            border-radius: 4px;
            font-size: 10pt;
        }
        QPushButton:checked {
            background-color: palette(highlight);
            color: palette(highlighted-text);
            border-color: palette(highlight);
        }
        QPushButton:hover:!checked {
            background-color: palette(light);
        }
        QPushButton:pressed {
            background-color: palette(dark);
        }
    )";

    ui->separatePlotsButton->setStyleSheet(buttonStyle);
    ui->combinedPlotButton->setStyleSheet(buttonStyle);
    ui->tableViewButton->setStyleSheet(buttonStyle);

    // Устанавливаем начальный режим
    updateDisplayModeButtons(DynamicPlotsGroup::SEPARATE_PLOTS);
}

void ChartWidget::onDisplayModeChanged()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    DynamicPlotsGroup::DisplayMode mode;
    if (button == ui->separatePlotsButton) {
        mode = DynamicPlotsGroup::SEPARATE_PLOTS;
    } else if (button == ui->combinedPlotButton) {
        mode = DynamicPlotsGroup::COMBINED_PLOT;
    } else if (button == ui->tableViewButton) {
        mode = DynamicPlotsGroup::TABLE_VIEW;
    } else {
        return;
    }

    setDisplayMode(mode);
}

void ChartWidget::setDisplayMode(DynamicPlotsGroup::DisplayMode mode)
{
    envGroup_->setMode(mode);
    acceleroGroup_->setMode(mode);
    gyroGroup_->setMode(mode);
    magnetoGroup_->setMode(mode);
    updateDisplayModeButtons(mode);
}

void ChartWidget::updateDisplayModeButtons(DynamicPlotsGroup::DisplayMode mode)
{
    ui->separatePlotsButton->setChecked(mode == DynamicPlotsGroup::SEPARATE_PLOTS);
    ui->combinedPlotButton->setChecked(mode == DynamicPlotsGroup::COMBINED_PLOT);
    ui->tableViewButton->setChecked(mode == DynamicPlotsGroup::TABLE_VIEW);
}