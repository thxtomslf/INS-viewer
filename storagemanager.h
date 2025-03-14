#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QList>
#include <QDateTime>
#include <QString>
#include <CsvSensorDataDAO.h>
#include "extendedsensordata.h"

class FileStorageManager {


public:
    FileStorageManager();
    ~FileStorageManager();
    void loadFile(QWidget *widget);

    QList<TimestampedSensorData> loadDataForPeriod(const QDateTime &start, const QDateTime &end) const;
    QList<TimestampedSensorData> loadAllData() const;

    void openFileToSave();
    void saveData(const TimestampedSensorData &data);

    QString getReadFileName() const;
    QString getSaveFileName() const;


private:
    void freeFile(CsvSensorDataDAO *dao);
private:
    CsvSensorDataDAO *csvDaoToRead = nullptr;
    CsvSensorDataDAO *csvDaoToSave = nullptr;
    QString readFilePath;
    QString saveFilePath;
    QList<TimestampedSensorData> cachedData;
};

#endif // STORAGEMANAGER_H
