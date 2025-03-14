#include "storagemanager.h"

#include <qfiledialog.h>

FileStorageManager::FileStorageManager(
    std::shared_ptr<DynamicSetting<bool>> isEnvMeasuresEnabled,
    std::shared_ptr<DynamicSetting<int>> envMeasuresPrecision,
    std::shared_ptr<DynamicSetting<bool>> isGyroMeasuresEnabled,
    std::shared_ptr<DynamicSetting<int>> gyroMeasuresPrecision,
    std::shared_ptr<DynamicSetting<bool>> isAcceleroMeasuresEnabled,
    std::shared_ptr<DynamicSetting<int>> acceleroMeasuresPrecision,
    std::shared_ptr<DynamicSetting<bool>> isMagnetoMeasuresEnabled,
    std::shared_ptr<DynamicSetting<int>> magnetoMeasuresPrecision
) {
    this->isEnvMeasuresEnabled = isEnvMeasuresEnabled;
    this->envMeasuresPrecision = envMeasuresPrecision;
    this->isGyroMeasuresEnabled = isGyroMeasuresEnabled;
    this->gyroMeasuresPrecision = gyroMeasuresPrecision;
    this->isAcceleroMeasuresEnabled = isAcceleroMeasuresEnabled;
    this->acceleroMeasuresPrecision = acceleroMeasuresPrecision;
    this->isMagnetoMeasuresEnabled = isMagnetoMeasuresEnabled;
    this->magnetoMeasuresPrecision = magnetoMeasuresPrecision;
    cachedData.clear();
}

FileStorageManager::~FileStorageManager() {
    freeFile(csvDaoToRead);
    freeFile(csvDaoToSave);
}

void FileStorageManager::loadFile(QWidget *widget) {

    readFilePath = QFileDialog::getOpenFileName(widget, "Выберите CSV файл", QDir::currentPath(), "CSV Files (*.csv)");
    if (readFilePath.isEmpty()) {
        qDebug() << "File wasn't chosen";
        return;
    }

    freeFile(csvDaoToRead);

    this->csvDaoToRead = new CsvSensorDataDAO(readFilePath);


    cachedData = csvDaoToRead->selectAllSensorData();
}

QList<TimestampedSensorData> FileStorageManager::loadDataForPeriod(const QDateTime &start, const QDateTime &end) const {
    QList<TimestampedSensorData> filteredData;
    for (const auto &data : cachedData) {
        if (data.getTimestamp() >= start && data.getTimestamp() <= end) {
            filteredData.append(data);
        }
    }
    return filteredData;
}


QList<TimestampedSensorData> FileStorageManager::loadAllData() const {
    return cachedData;
}

void FileStorageManager::openFileToSave() {
    qDebug() << "Starting saveToFile method...";

    // Define the directory for saving files
    QString experimentsDir = QDir::currentPath() + "/experiments"; // Исправлено название папки
    QDir dir(experimentsDir);
    if (!dir.exists()) {
        qDebug() << "Creating experiments directory...";
        dir.mkpath(".");
    }

    QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss-zzz") + ".csv"; // Добавлены миллисекунды и улучшен формат
    saveFilePath = experimentsDir + "/" + fileName;
    qDebug() << "File path:" << saveFilePath;

    freeFile(csvDaoToSave);

    this->csvDaoToSave = new CsvSensorDataDAO(
        saveFilePath,
        isEnvMeasuresEnabled->get(),
        envMeasuresPrecision->get(),
        isGyroMeasuresEnabled->get(),
        gyroMeasuresPrecision->get(),
        isAcceleroMeasuresEnabled->get(),
        acceleroMeasuresPrecision->get(),
        isMagnetoMeasuresEnabled->get(),
        magnetoMeasuresPrecision->get());
}

void FileStorageManager::saveData(const TimestampedSensorData &data) {
    if (csvDaoToSave == nullptr) {
        return;
    }

    csvDaoToSave->insertSensorData(data);
}

QString FileStorageManager::getReadFileName() const {
    return QFileInfo(readFilePath).fileName();
}

QString FileStorageManager::getSaveFileName() const {
    return saveFilePath;
}   

void FileStorageManager::freeFile(CsvSensorDataDAO *dao) {
    if (dao) {
        delete dao;
        dao = nullptr;
    }
}

