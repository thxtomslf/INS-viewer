#ifndef ISENSORDATADAO_H
#define ISENSORDATADAO_H

#include "extendedsensordata.h"
#include <QString>
#include <QList>

class ISensorDataDAO {
public:
    virtual ~ISensorDataDAO() = default;

    virtual bool insertSensorData(const TimestampedSensorData &data) = 0;
    virtual QList<TimestampedSensorData> selectSensorData(const QDateTime &start, const QDateTime &end) = 0;
    virtual QList<TimestampedSensorData> selectAllSensorData() = 0; // Новый метод
};

#endif // ISENSORDATADAO_H
