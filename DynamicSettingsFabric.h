#ifndef DYNAMICSETTINGSFABRIC_H
#define DYNAMICSETTINGSFABRIC_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "DynamicSetting.h"

class DynamicSettingsFabric
{
public:
    DynamicSettingsFabric() = default;

    template <typename T>
    std::shared_ptr<DynamicSetting<T>> createSetting(const std::string &name, const T &value, std::function<bool(const T&)> validator = nullptr) {
        auto setting = std::make_shared<DynamicSetting<T>>(value, validator);
        settings_[name] = setting;
        return setting;
    }

    template <typename T>
    std::shared_ptr<DynamicSetting<T>> getSetting(const std::string &name) const {
        auto it = settings_.find(name);
        if (it != settings_.end()) {
            return std::static_pointer_cast<DynamicSetting<T>>(it->second);
        }
        return nullptr;
    }

    std::vector<std::string> getSettingNames() const {
        std::vector<std::string> names;
        for (const auto &pair : settings_) {
            names.push_back(pair.first);
        }
        return names;
    }

private:
    std::map<std::string, std::shared_ptr<void>> settings_;
};

#endif // DYNAMICSETTINGSFABRIC_H
