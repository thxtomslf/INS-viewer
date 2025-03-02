#ifndef SERIALREADERWRITER_H
#define SERIALREADERWRITER_H

#include <QSerialPort>
#include <QObject>

class SerialReaderWriter : public QObject
{
    Q_OBJECT

public:
    explicit SerialReaderWriter(QObject *parent = nullptr);
    ~SerialReaderWriter();

    bool openSerialPort(const QString &portName,
                        QSerialPort::BaudRate baudRate,
                        QSerialPort::DataBits dataBits,
                        QSerialPort::Parity parity,
                        QSerialPort::StopBits stopBits,
                        QSerialPort::FlowControl flowControl);

    void closeSerialPort();
    QString getLastError() const;

protected:
    QSerialPort *serialPort;

private:
    QString lastError;
};

#endif // SERIALREADERWRITER_H
