#ifndef SENSORDATADAO_H
#define SENSORDATADAO_H

#include "extendedsensordata.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QIODevice>

#include <comand/SensorData.h>

#include <ISensorDataDAO.h>
#include <QList>


class SensorDataDAO : public ISensorDataDAO 
{
public:
    SensorDataDAO(const QString &databaseName)
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(databaseName);

        if (!db.open()) {
            qDebug() << "Failed to open database:" << db.lastError().text();
        } else {
            createTable();
        }
    }

    ~SensorDataDAO()
    {
        db.close();
    }

    bool insertSensorData(const TimestampedSensorData &data)
    {
        QSqlQuery query;
        query.prepare("INSERT INTO SensorData (timestamp, temperature, humidity, pressure, gyro_x, gyro_y, gyro_z, accelero_x, accelero_y, accelero_z, "
                      "magneto_x, magneto_y, magneto_z) "
                      "VALUES (:timestamp, :temperature, :humidity, :pressure, :gyro_x, :gyro_y, :gyro_z, :accelero_x, :accelero_y, :accelero_z, "
                      ":magneto_x, :magneto_y, :magneto_z)");

        query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
        const auto &envMeasures = data.getEnvironmentalMeasures();
        const auto &gyroMeasures = data.getGyroMeasures();
        const auto &acceleroMeasures = data.getAcceleroMeasures();
        const auto &magnetoMeasures = data.getMagnetoMeasures();

        if (envMeasures.size() >= 3) {
            query.bindValue(":temperature", envMeasures[0]);
            query.bindValue(":humidity", envMeasures[1]);
            query.bindValue(":pressure", envMeasures[2]);
        }

        if (gyroMeasures.size() >= 3) {
            query.bindValue(":gyro_x", gyroMeasures[0]);
            query.bindValue(":gyro_y", gyroMeasures[1]);
            query.bindValue(":gyro_z", gyroMeasures[2]);
        }

        if (acceleroMeasures.size() >= 3) {
            query.bindValue(":accelero_x", acceleroMeasures[0]);
            query.bindValue(":accelero_y", acceleroMeasures[1]);
            query.bindValue(":accelero_z", acceleroMeasures[2]);
        }

        if (magnetoMeasures.size() >= 3) {
            query.bindValue(":magneto_x", magnetoMeasures[0]);
            query.bindValue(":magneto_z", magnetoMeasures[1]);
            query.bindValue(":magneto_y", magnetoMeasures[2]);
        }

        if (!query.exec()) {
            qDebug() << "Failed to insert data:" << query.lastError().text();
            return false;
        }

        return true;
    }

    QList<TimestampedSensorData> selectAllSensorData() override {
        return QList<TimestampedSensorData>();
    }

    QList<TimestampedSensorData> selectSensorData(const QDateTime &start, const QDateTime &end)
    {
        QList<TimestampedSensorData> dataList;
        QSqlQuery query;
        query.prepare("SELECT timestamp, temperature, humidity, pressure, gyro_x, gyro_y,"
                      " gyro_z, accelero_x, accelero_y, accelero_z, magneto_x, magneto_y, magneto_z "
                      "FROM SensorData WHERE timestamp BETWEEN :start AND :end");
        query.bindValue(":start", start.toString(Qt::ISODate));
        query.bindValue(":end", end.toString(Qt::ISODate));

        if (!query.exec()) {
            qDebug() << "Failed to select data:" << query.lastError().text();
            return dataList;
        }

        while (query.next()) {
            QList<float> env;
            env.append(query.value("temperature").toFloat());
            env.append(query.value("humidity").toFloat());
            env.append(query.value("pressure").toFloat());

            QList<int16_t> accelero;
            accelero.append(static_cast<int16_t>(query.value("accelero_x").toInt()));
            accelero.append(static_cast<int16_t>(query.value("accelero_y").toInt()));
            accelero.append(static_cast<int16_t>(query.value("accelero_z").toInt()));

            QList<int16_t> gyro;
            gyro.append(static_cast<int16_t>(query.value("gyro_x").toInt()));
            gyro.append(static_cast<int16_t>(query.value("gyro_y").toInt()));
            gyro.append(static_cast<int16_t>(query.value("gyro_z").toInt()));

            QList<int16_t> magneto;
            magneto.append(static_cast<int16_t>(query.value("magneto_x").toInt()));
            magneto.append(static_cast<int16_t>(query.value("magneto_y").toInt()));
            magneto.append(static_cast<int16_t>(query.value("magneto_z").toInt()));


            QDateTime timestamp = QDateTime::fromString(query.value("timestamp").toString(), Qt::ISODate);
            TimestampedSensorData sensorData(env, gyro, accelero, magneto, timestamp);
            dataList.append(sensorData);
        }

        return dataList;
    }

private:
    void createTable()
    {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS SensorData ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "timestamp TEXT NOT NULL, "
                   "temperature REAL NOT NULL, "
                   "humidity REAL NOT NULL, "
                   "pressure REAL NOT NULL, "
                   "gyro_x INTEGER NOT NULL, "
                   "gyro_y INTEGER NOT NULL, "
                   "gyro_z INTEGER NOT NULL, "
                   "accelero_x INTEGER NOT NULL, "
                   "accelero_y INTEGER NOT NULL, "
                   "accelero_z INTEGER NOT NULL,"
                   "magneto_x INTEGER NOT NULL, "
                   "magneto_y INTEGER NOT NULL, "
                   "magneto_z INTEGER NOT NULL)");
    }

    QSqlDatabase db;
};

#endif // SENSORDATADAO_H
