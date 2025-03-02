#include "togglebutton.h"
#include "ui_togglebutton.h"

ToggleButton::ToggleButton(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToggleButton)
{
    ui->setupUi(this);

    ui->onButton->setDisabled(false);
    ui->offButton->setDisabled(true);

    connect(ui->onButton, SIGNAL(clicked()), this, SLOT(onClicked()));
    connect(ui->offButton, SIGNAL(clicked()), this, SLOT(offClicked()));
}

void ToggleButton::onClicked() {
    ui->onButton->setDisabled(true);
    ui->offButton->setDisabled(false);
    emit onSignal();
}

void ToggleButton::offClicked() {
    ui->onButton->setDisabled(false);
    ui->offButton->setDisabled(true);
    emit offSignal();
}

ToggleButton::~ToggleButton()
{
    delete ui;
}

void ToggleButton::clickOn()
{
    ui->onButton->click();
}

void ToggleButton::clickOff()
{
    ui->offButton->click();
}
