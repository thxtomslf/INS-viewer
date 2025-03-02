#ifndef BYTEARRAYCONVERTIBLE_H
#define BYTEARRAYCONVERTIBLE_H

#include <QByteArray>

class ByteArrayConvertible
{
public:
    // Фабричный метод для создания объекта из QByteArray
    virtual void fromBytes(const QByteArray &data) {
        return;
    }

    virtual QByteArray toBytes() const {
        return QByteArray();
    }
};

#endif // BYTEARRAYCONVERTIBLE_H
