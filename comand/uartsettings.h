#ifndef UARTSETTINGS_H
#define UARTSETTINGS_H

#include "ByteArrayConvertible.h"
#include <QByteArray>
#include <QSerialPort>
#include <cstdint>
#include <stdexcept>

class UartSettings : public ByteArrayConvertible
{
public:
    UartSettings(QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::FlowControl flowControl, QSerialPort::StopBits stopBits)
        : baudRate_(baudRate), dataBits_(dataBits), parity_(parity), flowControl_(flowControl), stopBits_(stopBits) {}

    virtual QByteArray toBytes() const override {
        uint8_t baudRate = mapBaudRate(baudRate_);
        uint8_t dataBitsMapped = mapDataBits(dataBits_);
        uint8_t stopBitsMapped = mapStopBits(stopBits_);
        uint8_t parityMapped = mapParity(parity_);
        uint8_t flowControlMapped = mapFlowControl(flowControl_);

        QByteArray result;
        result.append(static_cast<char>(baudRate));
        result.append(static_cast<char>(dataBitsMapped));
        result.append(static_cast<char>(stopBitsMapped));
        result.append(static_cast<char>(parityMapped));
        result.append(static_cast<char>(flowControlMapped));

        return result;
    }

private:
    QSerialPort::BaudRate baudRate_;
    QSerialPort::DataBits dataBits_;
    QSerialPort::Parity parity_;
    QSerialPort::FlowControl flowControl_;
    QSerialPort::StopBits stopBits_;

    uint8_t mapBaudRate(QSerialPort::BaudRate baudRate) const {
        switch (baudRate) {
        case QSerialPort::BaudRate::Baud9600: return 1;
        case QSerialPort::BaudRate::Baud19200: return 2;
        case QSerialPort::BaudRate::Baud38400: return 3;
        case QSerialPort::BaudRate::Baud57600: return 4;
        case QSerialPort::BaudRate::Baud115200: return 5;
        default:
            throw std::invalid_argument("Invalid DataBits value");
        }
    }

    uint8_t mapDataBits(QSerialPort::DataBits dataBits) const {
        switch (dataBits) {
            case QSerialPort::Data7: return 1; // First condition
            case QSerialPort::Data8: return 2; // Second condition
            default:
                throw std::invalid_argument("Invalid DataBits value");
        }
    }

    uint8_t mapStopBits(QSerialPort::StopBits stopBits) const {
        switch (stopBits) {
            case QSerialPort::OneStop: return 1; // First condition
            case QSerialPort::OneAndHalfStop: return 2; // Second condition
            case QSerialPort::TwoStop: return 3; // Third condition
            default:
                throw std::invalid_argument("Invalid StopBits value");
        }
    }

    uint8_t mapParity(QSerialPort::Parity parity) const {
        switch (parity) {
            case QSerialPort::NoParity: return 1; // First condition
            case QSerialPort::EvenParity: return 2; // Second condition
            case QSerialPort::OddParity: return 3; // Third condition
            default:
                throw std::invalid_argument("Invalid Parity value");
        }
    }

    uint8_t mapFlowControl(QSerialPort::FlowControl flowControl) const {
        switch (flowControl) {
            case QSerialPort::NoFlowControl: return 1; // First condition
            case QSerialPort::HardwareControl: return 2; // Second condition
            default:
                throw std::invalid_argument("Invalid FlowControl value");
        }
    }
};

#endif // UARTSETTINGS_H
