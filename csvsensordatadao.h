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
    explicit CsvSensorDataDAO(
        const QString &filePath,
        bool envMeasuresEnabled = true,
        int envMeasuresPrecision = 2,
        bool gyroMeasuresEnabled = true,
        int gyroMeasuresPrecision = 2,
        bool acceleroMeasuresEnabled = true,
        int acceleroMeasuresPrecision = 2,
        bool magnetoMeasuresEnabled = true,
        int magnetoMeasuresPrecision = 2
    ) : filePath(filePath), file(filePath),
        acceleroMeasuresEnabled(acceleroMeasuresEnabled),
        acceleroMeasuresPrecision(acceleroMeasuresPrecision),
        gyroMeasuresEnabled(gyroMeasuresEnabled), 
        gyroMeasuresPrecision(gyroMeasuresPrecision),
        magnetoMeasuresEnabled(magnetoMeasuresEnabled),
        magnetoMeasuresPrecision(magnetoMeasuresPrecision),
        envMeasuresEnabled(envMeasuresEnabled),
        envMeasuresPrecision(envMeasuresPrecision) {
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
            if (!validateHeader(columns)) {
                qDebug() << "Invalid CSV header in file:" << filePath;
                throw std::runtime_error("Invalid CSV header in file.");
            }
        } else {
            // Если файл пустой, записываем заголовок
            QTextStream out(&file);
            out << generateHeader() << "\n";
        }
    }

    ~CsvSensorDataDAO() {
        if (file.isOpen()) {
            file.close();
        }
    }

    bool insertSensorData(const TimestampedSensorData &data) override {
        if (!file.isOpen()) {
            qDebug() << "File is not open:" << filePath;
            return false;
        }

        // Перемещаем указатель в конец файла для записи новых данных
        file.seek(file.size());

        QTextStream out(&file);
        out << data.getTimestamp().toMSecsSinceEpoch() << ","; // Записываем timestamp в формате epoch

        if (envMeasuresEnabled) {
            out << QString::number(data.getEnvironmentalMeasures().at(0), 'f', envMeasuresPrecision) << ","
                << QString::number(data.getEnvironmentalMeasures().at(1), 'f', envMeasuresPrecision) << ","
                << QString::number(data.getEnvironmentalMeasures().at(2), 'f', envMeasuresPrecision) << ",";
        }

        if (gyroMeasuresEnabled) {
            out << data.getGyroMeasures().at(0) << ","
                << data.getGyroMeasures().at(1) << ","
                << data.getGyroMeasures().at(2) << ",";
        }

        if (acceleroMeasuresEnabled) {
            out << data.getAcceleroMeasures().at(0) << ","
                << data.getAcceleroMeasures().at(1) << ","
                << data.getAcceleroMeasures().at(2) << ",";
        }

        if (magnetoMeasuresEnabled) {
            out << data.getMagnetoMeasures().at(0) << ","
                << data.getMagnetoMeasures().at(1) << ","
                << data.getMagnetoMeasures().at(2);
        }

        out << "\n";
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
        QString header = in.readLine(); // Читаем заголовок
        QStringList columns = header.split(',');

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (!fields.isEmpty() && fields.last().isEmpty()) {
                fields.removeLast();
            }

            if (fields.size() == columns.size()) {
                qint64 epochTime = fields[0].toLongLong(); // Считываем timestamp в формате epoch
                QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(epochTime); // Преобразуем в QDateTime

                if (timestamp >= start && timestamp <= end) {
                    int index = 1;
                    QList<float> envMeasures;
                    if (columns.contains("temperature") && columns.contains("humidity") && columns.contains("pressure")) {
                        envMeasures = {fields[index++].toFloat(), fields[index++].toFloat(), fields[index++].toFloat()};
                    }
                    QList<int16_t> gyroMeasures;
                    if (columns.contains("gyro_x") && columns.contains("gyro_y") && columns.contains("gyro_z")) {
                        gyroMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                    }
                    QList<int16_t> acceleroMeasures;
                    if (columns.contains("accelero_x") && columns.contains("accelero_y") && columns.contains("accelero_z")) {
                        acceleroMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                    }
                    QList<int16_t> magnetoMeasures;
                    if (columns.contains("magneto_x") && columns.contains("magneto_y") && columns.contains("magneto_z")) {
                        magnetoMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                    }

                    TimestampedSensorData data(envMeasures, gyroMeasures, acceleroMeasures, magnetoMeasures, timestamp);
                    dataList.append(data);
                }
            }
        }

        return dataList;
    }

    QList<TimestampedSensorData> selectAllSensorData() override {
        QList<TimestampedSensorData> dataList;
        if (!file.isOpen()) {
            qDebug() << "File is not open:" << filePath;
            return dataList;
        }

        // Перемещаем указатель в начало файла для чтения
        file.seek(0);

        QTextStream in(&file);
        QString header = in.readLine(); // Читаем заголовок
        QStringList columns = header.split(',');

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');
            if (!fields.isEmpty() && fields.last().isEmpty()) {
                fields.removeLast();
            }

            if (fields.size() == columns.size()) {
                qint64 epochTime = fields[0].toLongLong(); // Считываем timestamp в формате epoch
                QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(epochTime); // Преобразуем в QDateTime

                int index = 1;
                QList<float> envMeasures;
                if (columns.contains("temperature") && columns.contains("humidity") && columns.contains("pressure")) {
                    envMeasures = {fields[index++].toFloat(), fields[index++].toFloat(), fields[index++].toFloat()};
                }
                QList<int16_t> gyroMeasures;
                if (columns.contains("gyro_x") && columns.contains("gyro_y") && columns.contains("gyro_z")) {
                    gyroMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                }
                QList<int16_t> acceleroMeasures;
                if (columns.contains("accelero_x") && columns.contains("accelero_y") && columns.contains("accelero_z")) {
                    acceleroMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                }
                QList<int16_t> magnetoMeasures;
                if (columns.contains("magneto_x") && columns.contains("magneto_y") && columns.contains("magneto_z")) {
                    magnetoMeasures = {static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt()), static_cast<int16_t>(fields[index++].toInt())};
                }

                TimestampedSensorData data(envMeasures, gyroMeasures, acceleroMeasures, magnetoMeasures, timestamp);
                dataList.append(data);
            }
        }

        return dataList;
    }

private:
    QString filePath;
    QFile file;
    bool envMeasuresEnabled;
    int envMeasuresPrecision;
    bool acceleroMeasuresEnabled;
    int acceleroMeasuresPrecision;
    bool gyroMeasuresEnabled;
    int gyroMeasuresPrecision;
    bool magnetoMeasuresEnabled;
    int magnetoMeasuresPrecision;

    QString generateHeader() const {
        QStringList headers = {"timestamp"};
        if (envMeasuresEnabled) {
            headers << "temperature" << "humidity" << "pressure";
        }
        if (gyroMeasuresEnabled) {
            headers << "gyro_x" << "gyro_y" << "gyro_z";
        }
        if (acceleroMeasuresEnabled) {
            headers << "accelero_x" << "accelero_y" << "accelero_z";
        }
        if (magnetoMeasuresEnabled) {
            headers << "magneto_x" << "magneto_y" << "magneto_z";
        }
        return headers.join(',');
    }

    bool validateHeader(const QStringList &columns) const {
        if (!columns.contains("timestamp")) {
            return false;
        }
        QStringList validHeaders = {"timestamp", "temperature", "humidity", "pressure", "gyro_x", "gyro_y", "gyro_z", "accelero_x", "accelero_y", "accelero_z", "magneto_x", "magneto_y", "magneto_z"};
        for (const QString &column : columns) {
            if (!validHeaders.contains(column)) {
                return false;
            }
        }
        return true;
    }
};

#endif // CSVSENSORDATADAO_H
