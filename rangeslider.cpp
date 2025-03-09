#include "rangeslider.h"

RangeSlider::RangeSlider(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    startSlider = new QSlider(Qt::Horizontal, this);
    endSlider = new QSlider(Qt::Horizontal, this);

    layout->addWidget(new QLabel("Начало", this));
    layout->addWidget(startSlider);
    layout->addWidget(new QLabel("Конец", this));
    layout->addWidget(endSlider);

    startSlider->setRange(0, 100);
    endSlider->setRange(0, 100);

    startSlider->setValue(0);
    endSlider->setValue(100);

    connect(startSlider, &QSlider::valueChanged, this, &RangeSlider::updateStartSlider);
    connect(endSlider, &QSlider::valueChanged, this, &RangeSlider::updateEndSlider);
}

void RangeSlider::setRange(const QDateTime &min, const QDateTime &max) {
    minTimestamp = min;
    maxTimestamp = max;

}

QDateTime RangeSlider::getStartTimestamp() const {
    int value = startSlider->value();
    qint64 diff = maxTimestamp.toMSecsSinceEpoch() - minTimestamp.toMSecsSinceEpoch();
    return minTimestamp.addMSecs((diff * value) / 100);
}

QDateTime RangeSlider::getEndTimestamp() const {
    int value = endSlider->value();
    qint64 diff = maxTimestamp.toMSecsSinceEpoch() - minTimestamp.toMSecsSinceEpoch();
    return minTimestamp.addMSecs((diff * value) / 100);
}

void RangeSlider::updateStartSlider(int value) {
    if (value >= endSlider->value()) {
        startSlider->setValue(endSlider->value() - 1);
    }
    emit rangeChanged(getStartTimestamp(), getEndTimestamp());
}

void RangeSlider::updateEndSlider(int value) {
    if (value <= startSlider->value()) {
        endSlider->setValue(startSlider->value() + 1);
    }
    emit rangeChanged(getStartTimestamp(), getEndTimestamp());
}
