#pragma once

#include <string>

namespace vk {

struct World;

// Пул всех доступных апгрейдов (из GameConfig::upgrades).
// Возвращает ссылку на конфиг из World.
// Вынесено как функция, чтобы HUD и логика имели единый источник.
void rollUpgrades(World& w);
void chooseUpgrade(World& w, int slot);

// Применяет эффект по строковому типу (dispatcher).
void applyUpgradeEffect(World& w, const std::string& type, float value);

// Начинает серию из N апгрейдов (чест или цепочка левелапов).
void beginUpgradeChain(World& w, int count, bool fromChest);

}