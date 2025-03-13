#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

SettingsWidget::SettingsWidget(
    const std::vector<DynamicSettingsFabric<int>> &numericFabrics,
    const std::vector<DynamicSettingsFabric<bool>> &booleanFabrics,
    QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingsWidget), numericFabrics(numericFabrics), booleanFabrics(booleanFabrics)
{   
    ui->setupUi(this);
    formLayout = new QFormLayout(this);
    setLayout(formLayout);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    // Clear existing layout
    QLayoutItem *item;
    while ((item = formLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    lineEdits_.clear();
    checkBoxes_.clear();

    // Recreate layout contents for numeric settings
    for (const auto &fabric : numericFabrics) {
        // Add group name label
        QLabel *groupLabel = new QLabel(QString::fromStdString(fabric.getGroupName()), this);
        QFont font = groupLabel->font();
        font.setBold(true);
        groupLabel->setFont(font);
        groupLabel->setStyleSheet("border: 1px solid white;"); // Add white border
        formLayout->addRow(groupLabel);

        for (const auto &name : fabric.getSettingNames()) {
            QString qName = QString::fromStdString(name);
            QLabel *label = new QLabel(qName, this);
            QLineEdit *lineEdit = new QLineEdit(this);

            auto setting = fabric.getSetting(qName.toStdString());
            if (setting) {
                lineEdit->setText(QString::number(setting->get()));
            }

            formLayout->addRow(label, lineEdit);
            lineEdits_[qName] = lineEdit;
        }
    }

    // Recreate layout contents for boolean settings
    for (const auto &fabric : booleanFabrics) {
        // Add group name label
        QLabel *groupLabel = new QLabel(QString::fromStdString(fabric.getGroupName()), this);
        QFont font = groupLabel->font();
        font.setBold(true);
        groupLabel->setFont(font);
        groupLabel->setStyleSheet("border: 1px solid white;"); // Add white border
        formLayout->addRow(groupLabel);

        for (const auto &name : fabric.getSettingNames()) {
            QString qName = QString::fromStdString(name);
            QLabel *label = new QLabel(qName, this);
            QCheckBox *checkBox = new QCheckBox(this);

            auto setting = fabric.getSetting(qName.toStdString());
            if (setting) {
                checkBox->setChecked(setting->get());
            }

            formLayout->addRow(label, checkBox);
            checkBoxes_[qName] = checkBox;
        }
    }

    QPushButton *submitButton = new QPushButton("Сохранить", this);
    connect(submitButton, &QPushButton::clicked, this, &SettingsWidget::applySettings);
    formLayout->addRow(submitButton);
}

void SettingsWidget::applySettings()
{
    for (auto it = lineEdits_.begin(); it != lineEdits_.end(); ++it) {
        const QString &name = it.key();
        QLineEdit *lineEdit = it.value();
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok) {
            for (const auto &fabric : numericFabrics) {
                auto setting = fabric.getSetting(name.toStdString());
                if (setting) {
                    try {
                        setting->set(value);
                    } catch (const std::invalid_argument &e) {
                        qDebug() << "Invalid value for setting" << name << ":" << e.what();
                    }
                    break;
                }
            }
        }
    }

    for (auto it = checkBoxes_.begin(); it != checkBoxes_.end(); ++it) {
        const QString &name = it.key();
        QCheckBox *checkBox = it.value();
        bool value = checkBox->isChecked();
        for (const auto &fabric : booleanFabrics) {
            auto setting = fabric.getSetting(name.toStdString());
            if (setting) {
                setting->set(value);
                break;
            }
        }
    }
}
