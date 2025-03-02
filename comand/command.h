#ifndef COMMAND_H
#define COMMAND_H

#include "bytearrayconvertible.h"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <cstdint>

enum CommandType {
    GetData = 1,
    Stop = 2,
    ReconfigureUart = 3,
};

template<typename T>
class Command
{
    static_assert(std::is_base_of<ByteArrayConvertible, T>::value, "T must derive from ByteArrayConvertible");
public:

    Command(CommandType commandType);
    void setBody(const T& obj);
    QByteArray toByteArray() const;

    static constexpr uint8_t START_BYTE = 0xAA; // Пример стартового байта

private:
    uint8_t calculateCRC(const QByteArray& data) const;

    CommandType commandType_;
    QByteArray body_;
};

template<typename T>
Command<T>::Command(CommandType commandType)
    : commandType_(commandType)
{
}

template<typename T>
void Command<T>::setBody(const T& obj)
{
    body_ = obj.toBytes();
}

template<typename T>
uint8_t Command<T>::calculateCRC(const QByteArray& data) const
{
    uint8_t polynomial = 0x07;
    uint8_t crc = 0x00;

    for (size_t i = 0; i < data.size(); i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}


template<typename T>
QByteArray Command<T>::toByteArray() const
{
    QByteArray message;
    message.append(START_BYTE);
    message.append(static_cast<uint8_t>(commandType_));
    message.append(static_cast<uint8_t>(body_.size()));
    message.append(body_);
    uint8_t crc = calculateCRC(message);
    message.append(crc);
    return message;
}

#endif // COMMAND_H

