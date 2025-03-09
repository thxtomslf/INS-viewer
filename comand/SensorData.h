#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <QByteArray>
#include <QList>
#include <QDebug>
#include "bytearrayconvertible.h"

class SensorData : public ByteArrayConvertible
{
public:
    // Конструктор по умолчанию
    SensorData() {}

    // Конструктор, принимающий QByteArray
    explicit SensorData(const QByteArray &data)
    {
        fromBytes(data);
    }

    // Конструктор, принимающий все приватные переменные
    SensorData(const QList<float> &envMeasures, const QList<int16_t> &gyroMeasures, const QList<int16_t> &acceleroMeasures, const QList<int16_t> &magnetoMeasures)
        : envMeasures_(envMeasures), gyroMeasures_(gyroMeasures), acceleroMeasures_(acceleroMeasures) , magnetoMeasures_(magnetoMeasures){}

    SensorData(const QList<float> &envMeasures, const QList<int16_t> &gyroMeasures, const QList<int16_t> &acceleroMeasures, const QList<int16_t> &magnetoMeasures, uint8_t dataSendCount)
        : envMeasures_(envMeasures), gyroMeasures_(gyroMeasures), acceleroMeasures_(acceleroMeasures) , magnetoMeasures_(magnetoMeasures){
        this->dataSendCount = dataSendCount;
    }

    void fromBytes(const QByteArray &data) override
    {
        if (data.size() < sizeof(float) * 3 + sizeof(int16_t) * 3 * 3) {
            qDebug() << "Data size is too small for SensorData";
            return;
        }

        int index = 0;

        // Извлечение данных среды
        envMeasures_.clear();
        for (int i = 0; i < 3; ++i) {
            float measure;
            std::memcpy(&measure, data.constData() + index, sizeof(float));
            envMeasures_.append(measure);
            index += sizeof(float);
        }

        // Извлечение данных гироскопа
        gyroMeasures_.clear();
        for (int i = 0; i < 3; ++i) {
            int16_t measure;
            std::memcpy(&measure, data.constData() + index, sizeof(int16_t));
            gyroMeasures_.append(measure);
            index += sizeof(int16_t);
        }

        // Извлечение данных акселерометра
        acceleroMeasures_.clear();
        for (int i = 0; i < 3; ++i) {
            int16_t measure;
            std::memcpy(&measure, data.constData() + index, sizeof(int16_t));
            acceleroMeasures_.append(measure);
            index += sizeof(int16_t);
        }

        magnetoMeasures_.clear();
        for (int i = 0; i < 3; ++i) {
            int16_t measure;
            std::memcpy(&measure, data.constData() + index, sizeof(int16_t));
            magnetoMeasures_.append(measure);
            index += sizeof(int16_t);
        }

        this->dataSendCount = data.at(data.size() - 1);
    }

    QList<float> getEnvironmentalMeasures() const
    {
        return envMeasures_;
    }

    QList<int16_t> getGyroMeasures() const
    {
        return gyroMeasures_;
    }

    QList<int16_t> getAcceleroMeasures() const
    {
        return acceleroMeasures_;
    }

    QList<int16_t> getMagnetoMeasures() const
    {
        return magnetoMeasures_;
    }

    uint8_t getDataSendCount() const {
        return dataSendCount;
    }

    void setEnvironmentalMeasures(const QList<float> &measures) {
        envMeasures_ = measures;
    }

    void setGyroMeasures(const QList<int16_t> &measures) {
        gyroMeasures_ = measures;
    }

    void setAcceleroMeasures(const QList<int16_t> &measures) {
        acceleroMeasures_ = measures;
    }

    void setMagnetoMeasures(const QList<int16_t> &measures) {
        magnetoMeasures_ = measures;
    }

private:
    QList<float> envMeasures_;
    QList<int16_t> gyroMeasures_;
    QList<int16_t> acceleroMeasures_;
    QList<int16_t> magnetoMeasures_;
    uint8_t dataSendCount = 0;
};

#endif // SENSORDATA_H
