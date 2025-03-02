#ifndef TIMESTAMPEDSENSORDATA_H
#define TIMESTAMPEDSENSORDATA_H

#include <QDateTime>

#include <comand/SensorData.h>


class TimestampedSensorData : public SensorData
{
public:
    // Конструктор по умолчанию
    TimestampedSensorData() {}

    // Конструктор, принимающий QByteArray и временную метку
    TimestampedSensorData(const QByteArray &data, const QDateTime &timestamp)
        : SensorData(data), timestamp_(timestamp) {}

    // Конструктор, принимающий SensorData и временную метку
    TimestampedSensorData(const SensorData &sensorData, const QDateTime &timestamp)
        : SensorData(sensorData), timestamp_(timestamp) {}

    // Конструктор, принимающий все приватные переменные
    TimestampedSensorData(const QList<float> &envMeasures, const QList<int16_t> &gyroMeasures, const QList<int16_t> &acceleroMeasures, const QList<int16_t> &magnetoMeasures, const QDateTime &timestamp)
        : SensorData(envMeasures, gyroMeasures, acceleroMeasures, magnetoMeasures), timestamp_(timestamp) {}

    // Получение временной метки
    QDateTime getTimestamp() const {
        return timestamp_;
    }

    // Установка временной метки
    void setTimestamp(const QDateTime &timestamp) {
        timestamp_ = timestamp;
    }

private:
    QDateTime timestamp_;
};

#endif // TIMESTAMPEDSENSORDATA_H
