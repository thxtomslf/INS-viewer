#ifndef CSVSENSORDATADAO_H
#define CSVSENSORDATADAO_H

#include "isensordatadao.h"
#include "comand/SensorData.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <stdexcept> // Для std::runtime_error

class CsvSensorDataDAO : public ISensorDataDAO {
public:
    explicit CsvSensorDataDAO(const QString &filePath) : filePath(filePath), file(filePath) {
        // Открываем файл на чтение и запись
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << "Failed to open file for reading and writing:" << filePath;
            throw std::runtime_error("Failed to open file for reading and writing.");
        }

        // Проверяем заголовок CSV-файла
        QTextStream in(&file);
        QString header = in.readLine();
        if (!header.isEmpty()) {
            QStringList columns = header.split(',');
            if (columns.size() != expectedColumnCount || !validateHeader(columns)) {
                qDebug() << "Invalid CSV header in file:" << filePath;
                throw std::runtime_error("Invalid CSV header in file.");
            }
        } else {
            // Если файл пустой, записываем заголовок
            QTextStream out(&file);
            out << "timestamp,temperature,humidity,pressure,gyro_x,gyro_y,gyro_z,accelero_x,accelero_y,accelero_z\n";
        }
    }

    ~CsvSensorDataDAO() {
        if (file.isOpen()) {
            file.close();
        }
    }

    bool insertSensorData(const SensorData &data) override {
        if (!file.isOpen()) {
            qDebug() << "File is not open:" << filePath;
            return false;
        }

        // Перемещаем указатель в конец файла для записи новых данных
        file.seek(file.size());

        QTextStream out(&file);
        out << data.getEnvironmentalMeasures().at(0) << ","
            << data.getEnvironmentalMeasures().at(1) << ","
            << data.getEnvironmentalMeasures().at(2) << ","
            << data.getGyroMeasures().at(0) << ","
            << data.getGyroMeasures().at(1) << ","
            << data.getGyroMeasures().at(2) << ","
            << data.getAcceleroMeasures().at(0) << ","
            << data.getAcceleroMeasures().at(1) << ","
            << data.getAcceleroMeasures().at(2) << ","
            << data.getMagnetoMeasures().at(0) << ","
            << data.getMagnetoMeasures().at(1) << ","
            << data.getMagnetoMeasures().at(2) << ","
            << static_cast<int>(data.getDataSendCount()) << "\n";

        return true;
    }

    QList<TimestampedSensorData> selectSensorData(const QDateTime &start, const QDateTime &end) override {
        QList<TimestampedSensorData> dataList;
        if (!file.isOpen()) {
            qDebug() << "File is not open:" << filePath;
            return dataList;
        }

        // Перемещаем указатель в начало файла для чтения
        file.seek(0);

        QTextStream in(&file);
        in.readLine(); // Пропускаем заголовок
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');

            if (fields.size() == expectedColumnCount) {
                QDateTime timestamp = QDateTime::fromString(fields[0], Qt::ISODate);
                if (timestamp >= start && timestamp <= end) {
                    QList<float> envMeasures = {fields[1].toFloat(), fields[2].toFloat(), fields[3].toFloat()};
                    QList<int16_t> gyroMeasures = {static_cast<int16_t>(fields[4].toInt()), static_cast<int16_t>(fields[5].toInt()), static_cast<int16_t>(fields[6].toInt())};
                    QList<int16_t> acceleroMeasures = {static_cast<int16_t>(fields[7].toInt()), static_cast<int16_t>(fields[8].toInt()), static_cast<int16_t>(fields[9].toInt())};
                    QList<int16_t> magnetoMeasures = {static_cast<int16_t>(fields[10].toInt()), static_cast<int16_t>(fields[11].toInt()), static_cast<int16_t>(fields[12].toInt())};

                    TimestampedSensorData data(envMeasures, gyroMeasures, acceleroMeasures, magnetoMeasures, timestamp);
                    dataList.append(data);
                }
            }
        }

        return dataList;
    }

private:
    QString filePath;
    QFile file;
    const int expectedColumnCount = 13; // Обновлено в соответствии с количеством полей

    bool validateHeader(const QStringList &columns) {
        QStringList expectedHeaders = {"timestamp", "temperature", "humidity", "pressure",
                                       "gyro_x", "gyro_y", "gyro_z",
                                       "accelero_x", "accelero_y", "accelero_z",
                                       "magneto_x", "magneto_y", "magneto_z"};
        return columns == expectedHeaders;
    }
};

#endif // CSVSENSORDATADAO_H
