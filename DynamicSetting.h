#ifndef DYNAMICSETTING_H
#define DYNAMICSETTING_H

#include <QObject>
#include <functional>
#include <stdexcept>

template <typename T>
class DynamicSetting
{

public:
    // Конструктор, принимающий значение указанного типа
    explicit DynamicSetting(const T &value, std::function<bool(const T&)> validator = nullptr)
        : value_(value), validator_(validator) {}

    // Метод для получения значения
    T get() const {
        return value_;
    }

    void setOnUpdateCallback(std::function<void(const T&)> onUpdate) {
        this->onUpdate_.append(onUpdate);
    }

    // Метод для установки значения с валидацией
    void set(const T &value) {
        if (validator_ && !validator_(value)) {
            throw std::invalid_argument("Invalid value");
        }
        value_ = value;
        if (!onUpdate_.empty()) {
            for (auto func : onUpdate_) {
                func(value);
            }
        }
    }

private:
    T value_;
    std::function<bool(const T&)> validator_;

    QList<std::function<void(const T&)>> onUpdate_;
};

#endif // DYNAMICSETTING_H
