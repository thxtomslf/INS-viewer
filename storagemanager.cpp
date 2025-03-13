#include "storagemanager.h"

StorageManager::StorageManager(ISensorDataDAO* newDao) {
    dao = newDao;
}

void setDAO(ISensorDataDAO* newDao) {
    if (newDao) {
        delete newDao;
    }

    dao = newDao;
}

void StorageManager::saveData(QList<TimestampedSensorData> data) {
    for (const auto& item : data) {
        dao->insertSensorData(item);
    }
}

QList<TimestampedSensorData> StorageManager::loadData() {
    return dao->selectAllSensorData();
}

QList<TimestampedSensorData> StorageManager::loadDataForPeriod(const QDateTime &start, const QDateTime &end) {
    return dao->selectSensorData(start, end);
}
