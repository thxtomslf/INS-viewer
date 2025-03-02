#ifndef UARTWIDGET_H
#define UARTWIDGET_H

#include "inscommandprocessor.h"
#include "routablewidget.cpp"
#include "serialportmonitor.h"
#include "serialreader.h"

#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class UartWidget; }
QT_END_NAMESPACE

class UartWidget : public RoutableWidget
{
    Q_OBJECT

public:
    explicit UartWidget(InsCommandProcessor *serial, QWidget *parent = nullptr);
    ~UartWidget();

    void onPageHide() override;

    void onPageShow(Page page) override;

    void loadSetupUartPage();
    void loadConfigureUartPage();

    void reconfigureUart();

private slots:
    void onConnectButtonClicked();

private:
    void setEnabledComboBoxes(bool value);

    void prepareUartParams();
private:
    InsCommandProcessor *serialReaderWriter;

    Ui::UartWidget *ui;
    SerialPortMonitor *serailPortMonitor = 0;

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
};

#endif // UARTWIDGET_H
