#include "rangeslider.h"

RangeSlider::RangeSlider(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    startSlider = new QSlider(Qt::Horizontal, this);
    endSlider = new QSlider(Qt::Horizontal, this);

    startLabel = new QLabel(this);
    endLabel = new QLabel(this);

    layout->addWidget(startLabel);
    layout->addWidget(startSlider);
    layout->addWidget(endLabel);
    layout->addWidget(endSlider);

    startSlider->setRange(0, 100);
    endSlider->setRange(0, 100);

    startSlider->setValue(0);
    endSlider->setValue(100);

    connect(startSlider, &QSlider::valueChanged, this, &RangeSlider::updateStartSlider);
    connect(endSlider, &QSlider::valueChanged, this, &RangeSlider::updateEndSlider);

    updateStartSlider(startSlider->value());
    updateEndSlider(endSlider->value());

    endLabel->setAlignment(Qt::AlignRight); // Align the endLabel to the right
}

void RangeSlider::setRange(const QDateTime &min, const QDateTime &max) {
    minTimestamp = min;
    maxTimestamp = max;
    startLabel->setText(getStartTimestamp().toString("yyyy-MM-dd HH:mm:ss"));
    endLabel->setText(getEndTimestamp().toString("yyyy-MM-dd HH:mm:ss"));
    startSlider->setValue(0);
    endSlider->setValue(100);
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
    startLabel->setText(getStartTimestamp().toString("yyyy-MM-dd HH:mm:ss"));
    emit rangeChanged(getStartTimestamp(), getEndTimestamp());
}

void RangeSlider::updateEndSlider(int value) {
    if (value <= startSlider->value()) {
        endSlider->setValue(startSlider->value() + 1);
    }
    endLabel->setText(getEndTimestamp().toString("yyyy-MM-dd HH:mm:ss"));
    emit rangeChanged(getStartTimestamp(), getEndTimestamp());
}
