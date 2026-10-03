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

const std::vector<Upgrade>& upgradePool();

void rollUpgrades(World& w);
void chooseUpgrade(World& w, int slot);

}