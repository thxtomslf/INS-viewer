#ifndef RANGESLIDER_H
#define RANGESLIDER_H

#include <QWidget>
#include <QSlider>
#include <QVBoxLayout>
#include <QLabel>
#include <QDateTime>

class RangeSlider : public QWidget {
    Q_OBJECT

public:
    explicit RangeSlider(QWidget *parent = nullptr);
    void setRange(const QDateTime &min, const QDateTime &max);
    QDateTime getStartTimestamp() const;
    QDateTime getEndTimestamp() const;

signals:
    void rangeChanged(const QDateTime &start, const QDateTime &end);

private slots:
    void updateStartSlider(int value);
    void updateEndSlider(int value);

private:
    QSlider *startSlider;
    QSlider *endSlider;
    QDateTime minTimestamp;
    QDateTime maxTimestamp;
};

#endif // RANGESLIDER_H