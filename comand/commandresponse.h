#ifndef COMMANDRESPONSE_H
#define COMMANDRESPONSE_H

#include "bytearrayconvertible.h"

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <type_traits>

template<typename T>
class CommandResponse
{
    static_assert(std::is_base_of<ByteArrayConvertible, T>::value, "T must derive from ByteArrayConvertible");

public:

    enum ResponseType {
        Accepted = 0x01,
        Rejected = 0x02,
        CRC_FAIL = 0x03,
        BAD_RESPONSE = 0x04
    };


    explicit CommandResponse(const QByteArray &responseData);

    ResponseType getResponseType() const;
    T getMessageBody() const;
    QString getError() const;

private:
    uint8_t calculateCRC(const QByteArray &data) const;

    ResponseType responseType_;
    T messageBody_;
    QString error_;
};

// Реализация методов шаблонного класса CommandResponse

// Конструктор, принимающий QByteArray как ответ
template<typename T>
CommandResponse<T>::CommandResponse(const QByteArray &data)
{
    if (data.size() < 4) {
        responseType_ = BAD_RESPONSE;
        error_ = "Invalid message format";
        return;
    }

    const uint8_t startByte = data.at(0);
    const uint8_t responseType = data.at(1);
    const uint8_t messageLength = data.at(2);

    if (startByte != 0xAA) {
        responseType_ = BAD_RESPONSE;
        error_ = "Invalid start byte";
        return;
    }

    if (3 + messageLength >= data.size()) {
        responseType_ = BAD_RESPONSE;
        error_ = "Invalid message length";
        return;
    }

    QByteArray messageBodyData = data.mid(3, messageLength);
    uint8_t receivedCRC = static_cast<uint8_t>(data.at(3 + messageLength));
    uint8_t calculatedCRC = calculateCRC(data.left(3 + messageLength));

    if (receivedCRC != calculatedCRC) {
        responseType_ = CRC_FAIL;
        error_ = "CRC check failed";
        return;
    }

    responseType_ = static_cast<ResponseType>(responseType);

    // Предположим, у T есть метод fromBytes, который получает T из QByteArray.
    // Если это не так, вам нужно будет изменить реализацию в соответствии с вашим требованием.
    messageBody_.fromBytes(messageBodyData);
    error_.clear(); // Очистка сообщения об ошибке, если ошибок нет
}

// Возвращает тип ответа
template<typename T>
typename CommandResponse<T>::ResponseType CommandResponse<T>::getResponseType() const
{
    return responseType_;
}

// Возвращает тело сообщения
template<typename T>
T CommandResponse<T>::getMessageBody() const
{
    return messageBody_;
}

// Возвращает сообщение об ошибке
template<typename T>
QString CommandResponse<T>::getError() const
{
    return error_;
}

// Вычисляет CRC для данных
template<typename T>
uint8_t CommandResponse<T>::calculateCRC(const QByteArray &data) const
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

#endif // COMMANDRESPONSE_H
