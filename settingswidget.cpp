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

    // Group settings by group name while maintaining order
    std::map<std::string, std::vector<std::pair<QString, QWidget*>>> groupedSettingsMap;

    // Collect numeric settings
    for (const auto &fabric : numericFabrics) {
        std::vector<std::pair<QString, QWidget*>> &settings = groupedSettingsMap[fabric.getGroupName()];
        for (const auto &name : fabric.getSettingNames()) {
            QString qName = QString::fromStdString(name);
            QLineEdit *lineEdit = new QLineEdit(this);

            auto setting = fabric.getSetting(qName.toStdString());
            if (setting) {
                lineEdit->setText(QString::number(setting->get()));
            }

            settings.emplace_back(qName, lineEdit);
            lineEdits_[buildSettingName(QString::fromStdString(fabric.getGroupName()), qName)] = lineEdit;
        }
    }

    // Collect boolean settings
    for (const auto &fabric : booleanFabrics) {
        std::vector<std::pair<QString, QWidget*>> &settings = groupedSettingsMap[fabric.getGroupName()];
        for (const auto &name : fabric.getSettingNames()) {
            QString qName = QString::fromStdString(name);
            QCheckBox *checkBox = new QCheckBox(this);

            auto setting = fabric.getSetting(qName.toStdString());
            if (setting) {
                checkBox->setChecked(setting->get());
            }

            settings.emplace_back(qName, checkBox);
            checkBoxes_[buildSettingName(QString::fromStdString(fabric.getGroupName()), qName)] = checkBox;
        }
    }

    // Add grouped settings to the layout
    for (const auto &group : groupedSettingsMap) {
        QLabel *groupLabel = new QLabel(QString::fromStdString(group.first), this);
        QFont font = groupLabel->font();
        font.setBold(true);
        groupLabel->setFont(font);
        groupLabel->setStyleSheet("border: 1px solid white;"); // Add white border
        formLayout->addRow(groupLabel);

        for (const auto &setting : group.second) {
            QLabel *label = new QLabel(setting.first, this);
            formLayout->addRow(label, setting.second);
        }
    }

    QPushButton *submitButton = new QPushButton("Сохранить", this);
    connect(submitButton, &QPushButton::clicked, this, &SettingsWidget::applySettings);
    formLayout->addRow(submitButton);
}

QString SettingsWidget::buildSettingName(const QString &groupName, const QString &settingName) {
    return groupName + "_" + settingName;
}

void SettingsWidget::applySettings()
{
    for (auto it = lineEdits_.begin(); it != lineEdits_.end(); ++it) {
        const QString &name = it.key();
        QLineEdit *lineEdit = it.value();
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok) {
            for (auto &fabric : numericFabrics) {
                if (fabric.getGroupName() != name.split("_").first().toStdString()) {
                    continue;
                }

                auto setting = fabric.getSetting(name.split("_").last().toStdString());
                if (setting) {
                    if (setting->get() != value) { // Check if the value has changed
                        try {
                            setting->set(value);
                        } catch (const std::invalid_argument &e) {
                            qDebug() << "Invalid value for setting" << name << ":" << e.what();
                        }
                    }
                    break;
                } else {
                    qDebug() << "Setting" << name.toUtf8() << "not found";
                }
            }
        }
    }

    for (auto it = checkBoxes_.begin(); it != checkBoxes_.end(); ++it) {
        const QString &name = it.key();
        QCheckBox *checkBox = it.value();
        bool value = checkBox->isChecked();
        for (auto &fabric : booleanFabrics) {
            if (fabric.getGroupName() != name.split("_").first().toStdString()) {
                continue;
            }

            auto setting = fabric.getSetting(name.split("_").last().toStdString());
            if (setting) {
                if (setting->get() != value) { // Check if the value has changed
                    setting->set(value);
                }
            } else {
                qDebug() << "Setting" << name.toUtf8() << "not found";
            }
        }
    }
}
