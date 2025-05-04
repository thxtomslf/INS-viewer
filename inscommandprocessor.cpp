#include "inscommandprocessor.h"
#include "comand/command.h"
#include "comand/uartsettings.h"
#include <QDebug>
#include <QException>
#include <QMessageBox>
#include <comand/EmtyData.h>

InsCommandProcessor::InsCommandProcessor(QObject *parent)
    : SerialReaderWriter(parent),
      tail(0),
      head(0),
      buffer_(BUFFER_SIZE)
{
    connect(serialPort, &QSerialPort::readyRead, this, &InsCommandProcessor::handleReadyRead);

    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &InsCommandProcessor::updateCounter);
    timer.start();
}

InsCommandProcessor::~InsCommandProcessor()
{

}

void InsCommandProcessor::readData(const std::function<void(const QByteArray&)> &callback)
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port is not open.";
        return;
    }

    responseCallback_ = callback;
    Command<EmptyData> command(CommandType::GetData);

    QByteArray commandData = command.toByteArray();
    qDebug() << "GetData: " << commandData;
    int writtenBytes = serialPort->write(commandData);
    if (writtenBytes < commandData.size()) {
        qDebug() << "Failed to send full command";
        throw new QException();
    }
}

void InsCommandProcessor::interrupt()
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port is not open.";
        return;
    }
    responseCallback_ = EMPTY_CALLBACK;
    Command<EmptyData> command(CommandType::Stop);

    QByteArray commandData = command.toByteArray();
    qDebug() << "Stop command data: " << commandData;
    int writtenBytes = serialPort->write(commandData);
    if (writtenBytes < commandData.size()) {
        qDebug() << "Failed to send full command";
        throw new QException();
    }
}

void InsCommandProcessor::reconfigureUart(QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::FlowControl flowControl, QSerialPort::StopBits stopBits)
{
    if (!serialPort->isOpen()) {
        qDebug() << "Serial port is not open.";
        return;
    }
    responseCallback_ = EMPTY_CALLBACK;

    UartSettings uartSettings(baudRate, dataBits, parity, flowControl, stopBits);
    Command<UartSettings> command(CommandType::ReconfigureUart);
    command.setBody(uartSettings);

    QByteArray commandData = command.toByteArray();
    qDebug() << "Reconfigure UART command data: " << commandData;
    int writtenBytes = serialPort->write(commandData);
    if (writtenBytes < commandData.size()) {
        qDebug() << "Failed to send full command";
        throw new QException();
    }

    emit stopped();
}

void InsCommandProcessor::updateCounter() {
    frequency = messagesCount;
    messagesCount = 0;
}

int InsCommandProcessor::getFrequency() {
    return frequency;
}

void InsCommandProcessor::handleReadyRead()
{
    QByteArray incomingData = serialPort->read(BUFFER_SIZE / 10);
    if (!responseCallback_) {
        return;
    }

    buffer_.append(incomingData);

    while (buffer_.size() >= 3) { // Минимальный размер для чтения заголовка
        QByteArray header = buffer_.read(3); // Читаем первые три байта

        uint8_t startByte = static_cast<uint8_t>(header.at(0));
        if (startByte != Command<EmptyData>::START_BYTE) {
            buffer_.pop(1); // Удаляем первый байт и продолжаем
            continue;
        }

        uint8_t responseType = static_cast<uint8_t>(header.at(1));
        uint8_t messageLength = static_cast<uint8_t>(header.at(2));

        // Проверяем, достаточно ли данных в буфере для полного сообщения
        if (buffer_.size() < 3 + messageLength + 1) {
            break;
        }

        // Если сообщение корректно, вызываем колбэк и удаляем данные из буфера
        responseCallback_(buffer_.pop(3 + messageLength + 1));
        messagesCount += 1;
        switch (responseType) {
        case Accepted:
            qDebug() << "Accepted";
            break;
        case Rejected:
            qDebug() << "Rejected";
            break;
        case CRC_FAIL:
            qDebug() << "CRC_FAIL";
            break;
        }
    }
}

QString InsCommandProcessor::responseTypeToString(ResponseType type) const
{
    switch (type) {
        case Accepted: return "Accepted";
        case Rejected: return "Rejected";
        case CRC_FAIL: return "CRC Failed";
        case BAD_RESPONSE: return "Bad Response";
        default: return "Unknown Response";
    }
}

bool InsCommandProcessor::validateCRC(const QByteArray &message, uint8_t expectedCrc) const
{
    uint8_t polynomial = 0x07;
    uint8_t crc = 0x00;for (size_t i = 0; i < message.size(); i++) {
        crc ^= message[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }

    return expectedCrc == crc;
}

bool InsCommandProcessor::openSerialPort(const QString &portName, QSerialPort::BaudRate baudRate,
                                       QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
                                       QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl)
{
    bool result = SerialReaderWriter::openSerialPort(portName, baudRate, dataBits, parity, stopBits, flowControl);
    emit connectionStatusChanged(result);
    return result;
}

void InsCommandProcessor::closeSerialPort()
{
    SerialReaderWriter::closeSerialPort();
    emit connectionStatusChanged(false);
}
