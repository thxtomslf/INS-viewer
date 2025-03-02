#include "serialportmonitor.h"

#include <QDebug>

SerialPortMonitor::SerialPortMonitor(QObject *parent) : QObject(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SerialPortMonitor::checkPorts);
    timer->start(1000); // проверка каждую секунду
    previousPorts = getAvailablePorts();
}

void SerialPortMonitor::checkPorts() {
    QSet<QString> currentPorts = getAvailablePorts();
    for (const QString &portName : currentPorts) {
        if (!previousPorts.contains(portName)) {
            emit newPortDetected(portName);
            qDebug() << "New port detected:" << portName;
        }
    }

    for (const QString &portName : previousPorts) {
        if (!currentPorts.contains(portName)) {
            emit portDeleted(portName);
            qDebug() << "Port deleted:" << portName;
        }
    }

    previousPorts = currentPorts;
}

QSet<QString> SerialPortMonitor::getAvailablePorts() {
    QSet<QString> ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        ports.insert(info.portName());
    }
    return ports;
}
