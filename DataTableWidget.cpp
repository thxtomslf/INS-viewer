#include "DataTableWidget.h"
#include <QScrollBar>
#include <QHeaderView>

DataTableWidget::DataTableWidget(QWidget *parent)
    : QWidget(parent)
{
    layout_ = new QVBoxLayout(this);
    setupTable();
}

void DataTableWidget::setupTable()
{
    table_ = new QTableWidget(this);
    table_->setColumnCount(1); // Первая колонка для временных меток
    table_->setHorizontalHeaderItem(0, new QTableWidgetItem("Время"));
    
    // Настройка таблицы
    table_->setAlternatingRowColors(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    
    // Добавляем таблицу в layout
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

void DataTableWidget::addPoint(const QDateTime &timestamp, const std::vector<double> &values)
{
    if (values.size() != dataBuffers_.size()) {
        return;
    }

    // Добавляем новую строку
    int newRow = table_->rowCount();
    table_->insertRow(newRow);

    // Добавляем временную метку
    QTableWidgetItem *timeItem = new QTableWidgetItem(
        timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz")
    );
    table_->setItem(newRow, 0, timeItem);

    // Добавляем значения
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

    // Прокручиваем к последней строке
    table_->scrollToBottom();
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

    // Очищаем таблицу
    table_->setRowCount(0);

    // Получаем данные из первого буфера для определения количества строк
    auto firstBufferData = buffers[0].getData();
    
    // Добавляем строки
    for (int i = 0; i < firstBufferData.size(); ++i) {
        table_->insertRow(i);
        
        // Добавляем временную метку
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            firstBufferData[i].first.toString("yyyy-MM-dd HH:mm:ss.zzz")
        );
        table_->setItem(i, 0, timeItem);

        // Добавляем значения из каждого буфера
        for (size_t j = 0; j < buffers.size(); ++j) {
            auto data = buffers[j].getData();
            if (i < data.size()) {
                QTableWidgetItem *valueItem = new QTableWidgetItem(
                    QString::number(data[i].second, 'f', 3)
                );
                valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                table_->setItem(i, j + 1, valueItem);
            }
        }
    }

    // Обновляем буферы
    dataBuffers_ = buffers;

    // Прокручиваем к последней строке
    if (table_->rowCount() > 0) {
        table_->scrollToBottom();
    }
}

void DataTableWidget::resizeColumnsToContents()
{
    for (int i = 0; i < table_->columnCount(); ++i) {
        table_->resizeColumnToContents(i);
    }
}
