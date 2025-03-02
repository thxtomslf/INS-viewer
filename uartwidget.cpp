#include "pagerouter.h"
#include "uartwidget.h"
#include "ui_uartwidget.h"

#include <QDebug>
#include <QFile>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QStandardItemModel>

UartWidget::UartWidget(InsCommandProcessor *serial, QWidget *parent) :
    RoutableWidget(parent),
    ui(new Ui::UartWidget),
    serialReaderWriter(serial)
{
    serailPortMonitor = new SerialPortMonitor();
    ui->setupUi(this);

    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->PortNameComboBox->addItem(port.portName());
    }

    // Заполнение параметров для UART
    ui->reconfigureUartButton->setVisible(false);
    ui->BaudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});
    ui->DataBitsComboBox->addItems({"7", "8"});
    ui->DataBitsComboBox->setCurrentIndex(1);
    ui->ParityComboBox->addItems({"None", "Even", "Odd"});
    ui->StopBitsComboBox->addItems({"1", "1.5", "2"});
    ui->FlowControlComboBox->addItems({"None", "RTS/CTS"});

    connect(ui->ConnectButton, &QPushButton::clicked, this, &UartWidget::onConnectButtonClicked);
    connect(ui->reconfigureUartButton, &QPushButton::clicked, this, &UartWidget::reconfigureUart);
    connect(serailPortMonitor, &SerialPortMonitor::newPortDetected, [=](const QString &portName) {
        qDebug() << "Signal received for new port:" << portName;
        ui->PortNameComboBox->addItem(portName);
    });
    connect(serailPortMonitor, &SerialPortMonitor::portDeleted, [=](const QString &portName) {
        qDebug() << "Signal received for deleted port:" << portName;
        int index = ui->PortNameComboBox->findText(portName); // Находим индекс элемента по значению
        if (index != -1) {
            ui->PortNameComboBox->removeItem(index); // Удаляем элемент по найденному индексу
            qDebug() << "Removed item:" << portName;
        } else {
            qDebug() << "Item not found:" << portName;
        }
    });


    QFile file(":/gui/darktheme.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString stylesheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(stylesheet);
    }
    qDebug() << file.errorString();
}

UartWidget::~UartWidget() {
    delete ui;
}

void UartWidget::setEnabledComboBoxes(bool value) {
    QStandardItemModel* model = ((QStandardItemModel*) ui->BaudRateComboBox->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        model->item(i)->setEnabled(value);
    }
    model = ((QStandardItemModel*) ui->DataBitsComboBox->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        model->item(i)->setEnabled(value);
    }

    model = ((QStandardItemModel*) ui->ParityComboBox->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        model->item(i)->setEnabled(value);
    }
    model = ((QStandardItemModel*) ui->StopBitsComboBox->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        model->item(i)->setEnabled(value);
    }
    model = ((QStandardItemModel*) ui->FlowControlComboBox->model());
    for (int i = 0; i < model->rowCount(); ++i) {
        model->item(i)->setEnabled(value);
    }
}

void UartWidget::loadSetupUartPage() {
    setEnabledComboBoxes(false);

    ui->PortNameComboBox->setVisible(true);
    ui->portNameLabel->setVisible(true);
    ui->reconfigureUartButton->setVisible(false);
    ui->ConnectButton->setVisible(true);
}

void UartWidget::loadConfigureUartPage() {
    setEnabledComboBoxes(true);

    ui->PortNameComboBox->setVisible(false);
    ui->portNameLabel->setVisible(false);
    ui->reconfigureUartButton->setVisible(true);
    ui->ConnectButton->setVisible(false);
}

void UartWidget::onPageHide() {

}

void UartWidget::prepareUartParams() {
    // Получение выбранных параметров
    QString baudRateStr = ui->BaudRateComboBox->currentText();
    QString dataBitsStr = ui->DataBitsComboBox->currentText();
    QString parityStr = ui->ParityComboBox->currentText();
    QString stopBitsStr = ui->StopBitsComboBox->currentText();
    QString flowControlStr = ui->FlowControlComboBox->currentText();

    if (baudRateStr == "9600") {
        baudRate = QSerialPort::Baud9600;
    } else if (baudRateStr == "19200") {
        baudRate = QSerialPort::Baud19200;
    } else if (baudRateStr == "38400") {
        baudRate = QSerialPort::Baud38400;
    } else if (baudRateStr == "57600") {
        baudRate = QSerialPort::Baud57600;
    } else if (baudRateStr == "115200") {
        baudRate = QSerialPort::Baud115200;
    }

    if (dataBitsStr == "5") {
        dataBits = QSerialPort::Data5;
    } else if (dataBitsStr == "6") {
        dataBits = QSerialPort::Data6;
    } else if (dataBitsStr == "7") {
        dataBits = QSerialPort::Data7;
    } else if (dataBitsStr == "8") {
        dataBits = QSerialPort::Data8;
    }

    if (parityStr == "None") {
        parity = QSerialPort::NoParity;
    } else if (parityStr == "Even") {
        parity = QSerialPort::EvenParity;
    } else if (parityStr == "Odd") {
        parity = QSerialPort::OddParity;
    }

    if (stopBitsStr == "1") {
        stopBits = QSerialPort::OneStop;
    } else if (stopBitsStr == "1.5") {
        stopBits = QSerialPort::OneAndHalfStop;
    } else if (stopBitsStr == "2") {
        stopBits = QSerialPort::TwoStop;
    }

    if (flowControlStr == "None") {
        flowControl = QSerialPort::NoFlowControl;
    } else if (flowControlStr == "RTS/CTS") {
        flowControl = QSerialPort::HardwareControl;
    }
}

void UartWidget::onPageShow(Page page) {
    if (page == Page::SetupUart) {
        loadSetupUartPage();
    } else if (page == Page::ConfigureUart) {
        loadConfigureUartPage();
    }
}


void UartWidget::reconfigureUart() {
    prepareUartParams();
    serialReaderWriter->reconfigureUart(baudRate, dataBits, parity, flowControl, stopBits);
    PageRouter::instance().navigateTo(Page::Graphics);
}

void UartWidget::onConnectButtonClicked() {
    // Установка параметров QSerialPort
    prepareUartParams();
    if (!serialReaderWriter->openSerialPort(ui->PortNameComboBox->currentText(), baudRate, dataBits, parity, stopBits, flowControl)) {
        QMessageBox::critical(this, "Connection Error", serialReaderWriter->getLastError());
    } else {
        PageRouter::instance().navigateTo(Page::Graphics);
    }
}
