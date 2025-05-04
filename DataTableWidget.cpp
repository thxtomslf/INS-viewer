#include "DataTableWidget.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QApplication>

DataTableWidget::DataTableWidget(QWidget *parent, const std::vector<DynamicPlotBuffer*>& buffers)
    : QWidget(parent)
    , updatesEnabled_(true)
    , dataBuffers_(buffers)
{
    layout_ = new QVBoxLayout(this);
    loadingLabel_ = new QLabel("Загрузка...", this);
    loadingLabel_->setAlignment(Qt::AlignCenter);
    loadingLabel_->setStyleSheet("font-size: 16px; color: gray;");
    loadingLabel_->hide();
    
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
    
    layout_->addWidget(loadingLabel_);
    layout_->addWidget(table_);
}

void DataTableWidget::showLoadingIndicator(bool show)
{
    if (show) {
        loadingLabel_->show();
        table_->hide();
    } else {
        loadingLabel_->hide();
        table_->show();
    }
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
    
    resizeColumnsToContents();
}

void DataTableWidget::clear()
{
    table_->setRowCount(0);
    for (auto buffer : dataBuffers_) {
        if (buffer) {
            buffer->clear();
        }
    }
}

QList<QList<QPair<QDateTime, double>>> DataTableWidget::getAllData() const
{
    QList<QList<QPair<QDateTime, double>>> result;
    
    for (const auto &buffer : dataBuffers_) {
        if (buffer) {
            QList<QPair<QDateTime, double>> columnData = buffer->getData();
            result.append(columnData);
        }
    }
    
    return result;
}

void DataTableWidget::update(const QDateTime &timestamp, const std::vector<double> &values)
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
            QString::number(values[i], 'f', 6)
        );
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(newRow, i + 1, valueItem);
    }

    // Прокручиваем к последней строке только если пользователь уже находится внизу
    QScrollBar* vScrollBar = table_->verticalScrollBar();
    if (vScrollBar->value() == vScrollBar->maximum()) {
        table_->scrollToBottom();
    }
}

void DataTableWidget::update()
{
    if (dataBuffers_.empty()) {
        return;
    }

    // Временно отключаем обновления UI и показываем индикатор загрузки
    showLoadingIndicator(true);
    table_->setUpdatesEnabled(false);
    updatesEnabled_ = false;

    // Очищаем таблицу
    table_->setRowCount(0);

    // Получаем общее количество строк
    auto firstBufferData = dataBuffers_[0]->getData();
    int totalRows = firstBufferData.size();

    // Устанавливаем количество строк сразу
    table_->setRowCount(totalRows);

    // Загружаем данные пакетами
    for (int i = 0; i < totalRows; i += BATCH_SIZE) {
        int batchSize = std::min(BATCH_SIZE, totalRows - i);
        loadDataBatch(i, batchSize);

        // Даем возможность обработать события UI
        QApplication::processEvents();
    }

    // Включаем обновления UI и скрываем индикатор загрузки
    table_->setUpdatesEnabled(true);
    updatesEnabled_ = true;
    showLoadingIndicator(false);

    // Прокручиваем к последней строке
    if (totalRows > 0) {
        table_->scrollToBottom();
    }

    // Обновляем размеры колонок
    resizeColumnsToContents();
}

void DataTableWidget::loadDataBatch(int startRow, int count)
{
    auto firstBufferData = dataBuffers_[0]->getData();

    for (int i = 0; i < count; ++i) {
        int currentRow = startRow + i;
        if (currentRow >= firstBufferData.size()) break;

        // Добавляем временную метку
        QTableWidgetItem *timeItem = new QTableWidgetItem(
            firstBufferData[currentRow].first.toString("yyyy-MM-dd HH:mm:ss.zzz")
        );
        table_->setItem(currentRow, 0, timeItem);

        // Добавляем значения из каждого буфера
        for (size_t j = 0; j < dataBuffers_.size(); ++j) {
            auto data = dataBuffers_[j]->getData();
            if (currentRow < data.size()) {
                QTableWidgetItem *valueItem = new QTableWidgetItem(
                    QString::number(data[currentRow].second, 'f', 6)
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

void DataTableWidget::updateBuffers(const std::vector<DynamicPlotBuffer*>& newBuffers)
{
    dataBuffers_ = newBuffers;
    update();
}