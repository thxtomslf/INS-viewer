#ifndef ISENSORDATADAO_H
#define ISENSORDATADAO_H

#include "comand/SensorData.h"
#include <QString>
#include <QList>

class ISensorDataDAO {
public:
    virtual ~ISensorDataDAO() = default;

    virtual bool insertSensorData(const SensorData &data) = 0;
    virtual QList<TimestampedSensorData> selectSensorData(const QDateTime &start, const QDateTime &end) = 0; 
};

#endif // ISENSORDATADAO_H