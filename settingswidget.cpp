#include "settingswidget.h"
#include "ui_settingswidget.h"
#include <QFormLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

SettingsWidget::SettingsWidget(const DynamicSettingsFabric &fabric, QWidget *parent)
    : QWidget(parent), ui(new Ui::SettingsWidget), fabric_(fabric)
{
    ui->setupUi(this);

    QFormLayout *formLayout = new QFormLayout(this);

    for (const auto &name : fabric.getSettingNames()) {
        QString qName = QString::fromStdString(name);
        QLabel *label = new QLabel(qName, this);
        QLineEdit *lineEdit = new QLineEdit(this);

        // Получаем текущее значение настройки и устанавливаем его как текст поля ввода
        auto setting = fabric_.getSetting<int>(qName.toStdString()); // Предполагаем, что все настройки типа int
        if (setting) {
            lineEdit->setText(QString::number(setting->get()));
        }

        // Добавляем QLabel и QLineEdit в макет формы
        formLayout->addRow(label, lineEdit);

        lineEdits_[qName] = lineEdit;
    }

    QPushButton *submitButton = new QPushButton("Сохранить", this);
    connect(submitButton, &QPushButton::clicked, this, &SettingsWidget::applySettings);
    formLayout->addRow(submitButton);

    setLayout(formLayout);
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::applySettings()
{
    for (auto it = lineEdits_.begin(); it != lineEdits_.end(); ++it) {
        const QString &name = it.key();
        QLineEdit *lineEdit = it.value();
        bool ok;
        int value = lineEdit->text().toInt(&ok);
        if (ok) {
            auto setting = fabric_.getSetting<int>(name.toStdString());
            if (setting) {
                try {
                    setting->set(value);
                } catch (const std::invalid_argument &e) {
                    qDebug() << "Invalid value for setting" << name << ":" << e.what();
                }
            }
        }
    }
}
