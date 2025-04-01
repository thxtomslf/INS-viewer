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
    explicit DataTableWidget(QWidget *parent = nullptr);
    
    void addDataColumn(const QString &label, 
                      std::shared_ptr<DynamicSetting<int>> bufferSize);
    void addPoint(const QDateTime &timestamp, const std::vector<double> &values);
    void clear();
    QList<QList<QPair<QDateTime, double>>> getAllData() const;
    
    void updateFromBuffers(const std::vector<DynamicPlotBuffer> &buffers);

private:
    void setupTable();
    void updateTable();
    void resizeColumnsToContents();

    QTableWidget *table_;
    QVBoxLayout *layout_;
    std::vector<QString> columnLabels_;
    std::vector<DynamicPlotBuffer> dataBuffers_;
};

#endif // DATATABLEWIDGET_H 
