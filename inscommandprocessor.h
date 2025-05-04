#ifndef INSCOMMANDPROCESSOR_H
#define INSCOMMANDPROCESSOR_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <vector>
#include <functional>
#include <qtimer.h>

#include "dynamiccircularbuffer.h"
#include "serialreader.h"

class InsCommandProcessor : public SerialReaderWriter
{
    Q_OBJECT

public:
    enum ResponseType {
        Accepted = 0x01,
        Rejected = 0x02,
        CRC_FAIL = 0x03,
        BAD_RESPONSE = 0x04
    };

    explicit InsCommandProcessor(QObject *parent = nullptr);
    ~InsCommandProcessor();

    virtual bool openSerialPort(const QString &portName, QSerialPort::BaudRate baudRate,
                       QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
                       QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl) override;
    virtual void closeSerialPort() override;

    bool isConnected() const { return serialPort && serialPort->isOpen(); }

    void readData(const std::function<void(const QByteArray&)> &callback);
    void interrupt();
    void reconfigureUart(QSerialPort::BaudRate baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::FlowControl flowControl, QSerialPort::StopBits stopBits);

    void setSpeed(QSerialPort::BaudRate baudRate);

public:
    int getFrequency();

signals:
    void connectionStatusChanged(bool connected);
    void stopped();

private slots:
    void handleReadyRead();
    void updateCounter();

private:
    std::function<void(const QByteArray&)> EMPTY_CALLBACK = [this](const QByteArray &data) {};
    QString responseTypeToString(ResponseType type) const;
    bool validateCRC(const QByteArray &message, uint8_t crc) const;

    const int BUFFER_SIZE = 2048 * 10;
    std::function<void(const QByteArray&)> responseCallback_;
    DynamicCircularBuffer buffer_;
    int tail;
    int head;

    bool addByteToBuffer(uint8_t byte);
    QByteArray getCompleteMessage();

    int messagesCount = 0;
    int frequency = 0;
    QTimer timer;
};


#endif // INSCOMMANDPROCESSOR_H
