#include "togglebutton.h"
#include "ui_togglebutton.h"

ToggleButton::ToggleButton(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToggleButton)
{
    ui->setupUi(this);

    // Set initial states
    ui->startButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(false);

    // Connect signals
    connect(ui->startButton, &QPushButton::clicked, this, &ToggleButton::onStartClicked);
    connect(ui->pauseButton, &QPushButton::clicked, this, &ToggleButton::onPauseClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &ToggleButton::onStopClicked);
}

ToggleButton::~ToggleButton()
{
    delete ui;
}

void ToggleButton::onStartClicked()
{
    ui->startButton->setEnabled(false);
    ui->pauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
    emit startSignal();
}

void ToggleButton::onPauseClicked()
{
    ui->startButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    emit pauseSignal();
}

void ToggleButton::onStopClicked()
{
    ui->startButton->setEnabled(true);
    ui->pauseButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    emit stopSignal();
}
