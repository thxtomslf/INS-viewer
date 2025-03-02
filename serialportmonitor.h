#ifndef SERIALPORTMONITOR_H
#define SERIALPORTMONITOR_H

#include <QObject>
#include <QSet>
#include <QSerialPortInfo>
#include <QTimer>

class SerialPortMonitor : public QObject {
    Q_OBJECT

public:
    explicit SerialPortMonitor(QObject *parent = nullptr);

signals:
    void newPortDetected(const QString &portName);
    void portDeleted(const QString &portName);

private slots:
    void checkPorts();

private:
    QSet<QString> getAvailablePorts();

    QTimer *timer;
    QSet<QString> previousPorts;
};

#endif // SERIALPORTMONITOR_H
