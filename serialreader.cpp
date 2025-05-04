#include "serialreader.h"

#include <QDebug>
#include <QThread>


SerialReaderWriter::SerialReaderWriter(QObject *parent) : QObject(parent),
    serialPort(new QSerialPort(this))
{
}

SerialReaderWriter::~SerialReaderWriter()
{
    closeSerialPort();
    delete serialPort;
}

bool SerialReaderWriter::openSerialPort(const QString &portName,
                                        QSerialPort::BaudRate baudRate,
                                        QSerialPort::DataBits dataBits,
                                        QSerialPort::Parity parity,
                                        QSerialPort::StopBits stopBits,
                                        QSerialPort::FlowControl flowControl)
{
    portName_ = portName;
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setParity(parity);
    serialPort->setStopBits(stopBits);
    serialPort->setFlowControl(flowControl);

    if (serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Connected to UART with settings:";
        qDebug() << "Port:" << portName;
        qDebug() << "Baud rate:" << baudRate;
        qDebug() << "Data bits:" << dataBits;
        qDebug() << "Parity:" << parity;
        qDebug() << "Stop bits:" << stopBits;
        qDebug() << "Flow control:" << flowControl;
        lastError.clear();
        return true;
    } else {
        lastError = serialPort->errorString();
        qDebug() << "Failed to open UART port:" << serialPort->errorString();
        return false;
    }
}

void SerialReaderWriter::closeSerialPort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

QString SerialReaderWriter::getLastError() const
{
    return lastError;
}
