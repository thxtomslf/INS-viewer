#ifndef CSVSENSORDATADAO_H
#define CSVSENSORDATADAO_H

#include "isensordatadao.h"
#include <QFile>
#include <QTextStream>
#include <QList>

class CsvSensorDataDAO : public ISensorDataDAO {
public:
    explicit CsvSensorDataDAO(const QString &filePath) : filePath(filePath) {}

    bool insertSensorData(const SensorData &data) override {
        QFile file(filePath);
        if (!file.open(QIODevice::Append | QIODevice::Text)) {
            return false;
        }

        QTextStream out(&file);
        out << data.timestamp.toString(Qt::ISODate) << ","
            << data.temperature << ","
            << data.humidity << ","
            << data.pressure << ","
            << data.gyro_x << ","
            << data.gyro_y << ","
            << data.gyro_z << ","
            << data.accelero_x << ","
            << data.accelero_y << ","
            << data.accelero_z << "\n";

        file.close();
        return true;
    }

    QList<TimestampedSensorData> selectSensorData(const QDateTime &start, const QDateTime &end) override {
        QList<TimestampedSensorData> dataList;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return dataList;
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');

            if (fields.size() == 10) {
                QDateTime timestamp = QDateTime::fromString(fields[0], Qt::ISODate);
                if (timestamp >= start && timestamp <= end) {
                    TimestampedSensorData data;
                    data.timestamp = timestamp;
                    data.temperature = fields[1].toDouble();
                    data.humidity = fields[2].toDouble();
                    data.pressure = fields[3].toDouble();
                    data.gyro_x = fields[4].toDouble();
                    data.gyro_y = fields[5].toDouble();
                    data.gyro_z = fields[6].toDouble();
                    data.accelero_x = fields[7].toDouble();
                    data.accelero_y = fields[8].toDouble();
                    data.accelero_z = fields[9].toDouble();

                    dataList.append(data);
                }
            }
        }

        file.close();
        return dataList;
    }

private:
    QString filePath;
};

#endif // CSVSENSORDATADAO_H