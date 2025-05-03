#include "dynamicplot.h"
#include <QVBoxLayout>
#include <QSharedPointer>
#include <QDateTime>
#include <QApplication>
#include <QScrollArea>

DynamicPlot::DynamicPlot(QWidget *parent,
                        DynamicPlotBuffer* buffer)
    : QWidget(parent)
    , buffer_(buffer)
{
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

    // Устанавливаем фильтр событий для QCustomPlot
    customPlot_->installEventFilter(this);
    customPlot_->setFocusPolicy(Qt::StrongFocus); // Чтобы получать события клавиатуры
}

void DynamicPlot::setPlotSize(std::shared_ptr<DynamicSetting<int>> plotSize) {
    this->plotSize = plotSize;
    customPlot_->setMinimumSize(plotSize->get(), plotSize->get());

    this->plotSize.get()->setOnUpdateCallback([this](int newValue) {
        qDebug() << this->plotSize->get();
        customPlot_->setMinimumSize(this->plotSize->get(), this->plotSize->get());
    });
}

void DynamicPlot::clear()
{
    // Очистка данных графика
    graph_->data()->clear();

    // Сброс буферов
    buffer_->clear();

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
    if (buffer_) {
        buffer_->addPoint(time, value);

        QVector<double> visibleTimeData = buffer_->getVisibleTimeData();
        QVector<double> visibleValueData = buffer_->getVisibleData();

        graph_->setData(visibleTimeData, visibleValueData);
        customPlot_->xAxis->setRange(visibleTimeData.first(), visibleTimeData.last());
        customPlot_->rescaleAxes(true);
        customPlot_->replot();
    }
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

void DynamicPlot::update()
{
    if (!buffer_) {
        return;
    }

    QVector<double> timeData = buffer_->getVisibleTimeData();
    QVector<double> valueData = buffer_->getVisibleData();
    
    graph_->setData(timeData, valueData);
    if (!timeData.isEmpty()) {
        customPlot_->xAxis->setRange(timeData.first(), timeData.last());
    }
    customPlot_->rescaleAxes(true);
    customPlot_->replot();
}

bool DynamicPlot::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == customPlot_ && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        
        // Если зажат Ctrl, позволяем QCustomPlot обрабатывать событие (зум)
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            return false;
        }

        // Передаем событие в родительский QScrollArea
        if (QScrollArea *scrollArea = findParentScrollArea()) {
            QWheelEvent *newEvent = new QWheelEvent(
                wheelEvent->position(),
                wheelEvent->globalPosition(),
                wheelEvent->pixelDelta(),
                wheelEvent->angleDelta(),
                wheelEvent->buttons(),
                wheelEvent->modifiers(),
                wheelEvent->phase(),
                wheelEvent->inverted()
            );
            QApplication::sendEvent(scrollArea->viewport(), newEvent);
            delete newEvent;
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void DynamicPlot::wheelEvent(QWheelEvent *event)
{
    // Если зажат Ctrl, обрабатываем событие (зум)
    if (event->modifiers() & Qt::ControlModifier) {
        QWidget::wheelEvent(event);
        return;
    }

    // В противном случае передаем событие родительскому QScrollArea
    if (QScrollArea *scrollArea = findParentScrollArea()) {
        QWheelEvent *newEvent = new QWheelEvent(
            event->position(),
            event->globalPosition(),
            event->pixelDelta(),
            event->angleDelta(),
            event->buttons(),
            event->modifiers(),
            event->phase(),
            event->inverted()
        );
        QApplication::sendEvent(scrollArea->viewport(), newEvent);
        delete newEvent;
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

QScrollArea* DynamicPlot::findParentScrollArea() const
{
    QWidget *parent = parentWidget();
    while (parent) {
        if (QScrollArea *scrollArea = qobject_cast<QScrollArea*>(parent)) {
            return scrollArea;
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}
