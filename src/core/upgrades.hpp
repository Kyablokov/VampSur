#pragma once

#include <functional>
#include <string>
#include <vector>

namespace vk {

struct World;

struct Upgrade {
    std::string id;
    std::string name;
    std::string description;
    std::function<void(World&)> apply;
};

// Пул всех доступных апгрейдов. Порядок стабилен.
const std::vector<Upgrade>& upgradePool();

// Заполняет w.state.upgradeOffer тремя уникальными индексами из пула.
// При size() < 3 — сколько есть.
void rollUpgrades(World& w);

// Применяет апгрейд из слота [0..2] и возвращает mode в Playing.
void chooseUpgrade(World& w, int slot);

}