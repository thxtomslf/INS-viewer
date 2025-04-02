#include "DataTableWidget.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QApplication>

DataTableWidget::DataTableWidget(QWidget *parent)
    : QWidget(parent)
    , updatesEnabled_(true)
{
    layout_ = new QVBoxLayout(this);
    setupTable();
}

void DataTableWidget::setupTable()
{
    table_ = new QTableWidget(this);
    table_->setColumnCount(1);
    table_->setHorizontalHeaderItem(0, new QTableWidgetItem("Время"));
    
    // Оптимизация производительности таблицы
    table_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    table_->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    table_->setShowGrid(false); // Отключаем отрисовку сетки для повышения производительности
    
    table_->setAlternatingRowColors(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    
    layout_->addWidget(table_);
}

void DataTableWidget::addDataColumn(const QString &label, 
                                  std::shared_ptr<DynamicSetting<int>> bufferSize)
{
    // Добавляем новую колонку в таблицу
    int columnIndex = table_->columnCount();
    table_->setColumnCount(columnIndex + 1);
    table_->setHorizontalHeaderItem(columnIndex, new QTableWidgetItem(label));
    
    // Сохраняем метку
    columnLabels_.push_back(label);
    
    // Создаем новый буфер данных
    dataBuffers_.emplace_back(bufferSize);
    
    resizeColumnsToContents();
}

void DataTableWidget::addPoint(const QDateTime &timestamp, 
                             const std::vector<double> &values)
{
    if (!updatesEnabled_ || values.size() != dataBuffers_.size()) {
        return;
    }

    int newRow = table_->rowCount();
    table_->insertRow(newRow);

    QTableWidgetItem *timeItem = new QTableWidgetItem(
        timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz")
    );
    table_->setItem(newRow, 0, timeItem);

    for (size_t i = 0; i < values.size(); ++i) {
        QTableWidgetItem *valueItem = new QTableWidgetItem(
            QString::number(values[i], 'f', 3)
        );
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(newRow, i + 1, valueItem);
    }

    // Добавляем данные в буферы
    for (size_t i = 0; i < values.size(); ++i) {
        dataBuffers_[i].addPoint(timestamp, values[i]);
    }

    // Прокручиваем к последней строке только если пользователь уже находится внизу
    QScrollBar* vScrollBar = table_->verticalScrollBar();
    if (vScrollBar->value() == vScrollBar->maximum()) {
        table_->scrollToBottom();
    }
}

void DataTableWidget::updateTable()
{
    if (dataBuffers_.empty()) {
        return;
    }

    // Получаем данные из первого буфера для определения количества строк
    auto firstBufferData = dataBuffers_[0].getData();
    int rowCount = firstBufferData.size();
    
    table_->setRowCount(rowCount);

    // Заполняем временные метки
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            firstBufferData[row].first.toString("yyyy-MM-dd HH:mm:ss.zzz")
        );
        table_->setItem(row, 0, timeItem);
    }

    // Заполняем значения для каждого буфера
    for (size_t col = 0; col < dataBuffers_.size(); ++col) {
        auto data = dataBuffers_[col].getData();
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem *valueItem = new QTableWidgetItem(
                QString::number(data[row].second, 'f', 3)
            );
            valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            table_->setItem(row, col + 1, valueItem);
        }
    }

    // Прокручиваем к последней строке
    table_->scrollToBottom();
    
    resizeColumnsToContents();
}

void DataTableWidget::clear()
{
    table_->setRowCount(0);
    for (auto &buffer : dataBuffers_) {
        buffer.clear();
    }
}

QList<QList<QPair<QDateTime, double>>> DataTableWidget::getAllData() const
{
    QList<QList<QPair<QDateTime, double>>> result;
    
    for (const auto &buffer : dataBuffers_) {
        QList<QPair<QDateTime, double>> columnData = buffer.getData();
        result.append(columnData);
    }
    
    return result;
}

void DataTableWidget::updateFromBuffers(const std::vector<DynamicPlotBuffer> &buffers)
{
    if (buffers.empty() || buffers.size() != dataBuffers_.size()) {
        return;
    }

    // Временно отключаем обновления UI
    table_->setUpdatesEnabled(false);
    updatesEnabled_ = false;

    // Очищаем таблицу
    table_->setRowCount(0);

    // Получаем общее количество строк
    auto firstBufferData = buffers[0].getData();
    int totalRows = firstBufferData.size();
    
    // Устанавливаем количество строк сразу
    table_->setRowCount(totalRows);

    // Загружаем данные пакетами
    for (int i = 0; i < totalRows; i += BATCH_SIZE) {
        int batchSize = std::min(BATCH_SIZE, totalRows - i);
        loadDataBatch(buffers, i, batchSize);
        
        // Даем возможность обработать события UI
        QApplication::processEvents();
    }

    // Обновляем буферы
    dataBuffers_ = buffers;

    // Включаем обновления UI
    table_->setUpdatesEnabled(true);
    updatesEnabled_ = true;

    // Прокручиваем к последней строке
    if (totalRows > 0) {
        table_->scrollToBottom();
    }
}

void DataTableWidget::loadDataBatch(const std::vector<DynamicPlotBuffer> &buffers, 
                                  int startRow, int count)
{
    auto firstBufferData = buffers[0].getData();
    
    for (int i = 0; i < count; ++i) {
        int currentRow = startRow + i;
        if (currentRow >= firstBufferData.size()) break;

        // Добавляем временную метку
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            firstBufferData[currentRow].first.toString("yyyy-MM-dd HH:mm:ss.zzz")
        );
        table_->setItem(currentRow, 0, timeItem);

        // Добавляем значения из каждого буфера
        for (size_t j = 0; j < buffers.size(); ++j) {
            auto data = buffers[j].getData();
            if (currentRow < data.size()) {
                QTableWidgetItem *valueItem = new QTableWidgetItem(
                    QString::number(data[currentRow].second, 'f', 3)
                );
                valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                table_->setItem(currentRow, j + 1, valueItem);
            }
        }
    }
}

void DataTableWidget::resizeColumnsToContents()
{
    for (int i = 0; i < table_->columnCount(); ++i) {
        table_->resizeColumnToContents(i);
    }
}
