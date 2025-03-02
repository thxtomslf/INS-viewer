#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QList>
#include <stdexcept>

class ObjectNavigator : public QWidget {
    Q_OBJECT

public:
    explicit ObjectNavigator(QWidget* parent = nullptr)
        : QWidget(parent), currentIndex(0) {
        // Создаем кнопки
        prevButton = new QPushButton(this);
        prevButton->setText("<=");

        nextButton = new QPushButton(this);
        nextButton->setText("=>");

        // Подключаем слоты к нажатиям кнопок
        connect(prevButton, &QPushButton::clicked, this, &ObjectNavigator::goPrevious);
        connect(nextButton, &QPushButton::clicked, this, &ObjectNavigator::goNext);

        // Создаем и настраиваем макет
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->addWidget(prevButton);
        layout->addWidget(nextButton);
        setLayout(layout);

        // Обновляем состояние кнопок
        updateButtons();
    }

    void setObjects(const QList<QWidget*>& newObjects) {
        if (newObjects.isEmpty()) {
            throw std::invalid_argument("List of objects cannot be empty");
        }

        objects = newObjects;
        currentIndex = 0;
        updateObjectStates();
        emit objectChanged(objects[currentIndex]);
    }

signals:
    void objectChanged(QWidget* currentObject);

private slots:
    void goPrevious() {
        if (currentIndex > 0) {
            --currentIndex;
            updateObjectStates();
            emit objectChanged(objects[currentIndex]);
            updateButtons();
        }
    }

    void goNext() {
        if (currentIndex < objects.size() - 1) {
            ++currentIndex;
            updateObjectStates();
            emit objectChanged(objects[currentIndex]);
            updateButtons();
        }
    }

private:
    void updateButtons() {
        prevButton->setDisabled(currentIndex == 0);
        nextButton->setDisabled(currentIndex == objects.size() - 1);
    }

    void updateObjectStates() {
        for (int i = 0; i < objects.size(); ++i) {
            objects[i]->setVisible(i == currentIndex);
        }
    }

    QList<QWidget*> objects;
    int currentIndex;
    QPushButton* prevButton;
    QPushButton* nextButton;
};

#endif // NAVIGATOR_H
