#ifndef DATATABLEWIDGET_H
#define DATATABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHeaderView>
#include <memory>
#include "DynamicSetting.h"
#include "DynamicPlotBuffer.h"

class DataTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataTableWidget(QWidget *parent = nullptr,
                           const std::vector<DynamicPlotBuffer*>& buffers = {});
    
    void addDataColumn(const QString &label, 
                      std::shared_ptr<DynamicSetting<int>> bufferSize);
    void clear();
    QList<QList<QPair<QDateTime, double>>> getAllData() const;
    
    void update();
    void updateBuffers(const std::vector<DynamicPlotBuffer*>& newBuffers);
    void update(const QDateTime &timestamp, const std::vector<double> &values);

private:
    void loadDataBatch(int startRow, int count);
    void setupTable();
    void updateTable();
    void resizeColumnsToContents();

    int BATCH_SIZE = 100; // Размер пакета для загрузки
    QTableWidget *table_;
    QVBoxLayout *layout_;
    std::vector<QString> columnLabels_;
    std::vector<DynamicPlotBuffer*> dataBuffers_;
    bool updatesEnabled_;
};

#endif // DATATABLEWIDGET_H 
