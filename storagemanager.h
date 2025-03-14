#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QList>
#include <QDateTime>
#include <QString>
#include <CsvSensorDataDAO.h>
#include "extendedsensordata.h"
#include "DynamicSetting.h"

class FileStorageManager {


public:
    FileStorageManager(
        std::shared_ptr<DynamicSetting<bool>> isEnvMeasuresEnabled,
        std::shared_ptr<DynamicSetting<int>> envMeasuresPrecision,
        std::shared_ptr<DynamicSetting<bool>> isGyroMeasuresEnabled,
        std::shared_ptr<DynamicSetting<int>> gyroMeasuresPrecision,
        std::shared_ptr<DynamicSetting<bool>> isAcceleroMeasuresEnabled,
        std::shared_ptr<DynamicSetting<int>> acceleroMeasuresPrecision,
        std::shared_ptr<DynamicSetting<bool>> isMagnetoMeasuresEnabled,
        std::shared_ptr<DynamicSetting<int>> magnetoMeasuresPrecision);
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
    std::shared_ptr<DynamicSetting<bool>> isEnvMeasuresEnabled;
    std::shared_ptr<DynamicSetting<int>> envMeasuresPrecision;
    std::shared_ptr<DynamicSetting<bool>> isGyroMeasuresEnabled;
    std::shared_ptr<DynamicSetting<int>> gyroMeasuresPrecision;
    std::shared_ptr<DynamicSetting<bool>> isAcceleroMeasuresEnabled;
    std::shared_ptr<DynamicSetting<int>> acceleroMeasuresPrecision;
    std::shared_ptr<DynamicSetting<bool>> isMagnetoMeasuresEnabled;
    std::shared_ptr<DynamicSetting<int>> magnetoMeasuresPrecision;
};

#endif // STORAGEMANAGER_H
