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

    virtual bool openSerialPort(const QString &portName,
                        QSerialPort::BaudRate baudRate,
                        QSerialPort::DataBits dataBits,
                        QSerialPort::Parity parity,
                        QSerialPort::StopBits stopBits,
                        QSerialPort::FlowControl flowControl);

    virtual void closeSerialPort();
    QString getLastError() const;
    QString getPortName() const { return portName_; }

protected:
    QSerialPort *serialPort;
    QString portName_;

private:
    QString lastError;
};

#endif // SERIALREADERWRITER_H
