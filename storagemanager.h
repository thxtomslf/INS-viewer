#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QList>
#include <QDateTime>
#include "extendedsensordata.h"

class StorageManager {
public:
    StorageManager(ISensorDataDAO* newDao);
    void setDAO(ISensorDataDAO* newDao);
    void saveData(QList<TimestampedSensorData> data);
    QList<TimestampedSensorData> loadData();
    QList<TimestampedSensorData> loadDataForPeriod(const QDateTime &start, const QDateTime &end);
private:
    ISensorDataDAO* dao = nullptr;

};

#endif // STORAGEMANAGER_H
