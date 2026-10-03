#pragma once

#include <vector>
#include <entt/entt.hpp>

#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/pools.hpp"
#include "core/save.hpp"
#include "core/spatial_hash.hpp"
#include "components/components.hpp"

namespace vk {

inline entt::entity findPlayer(entt::registry& r) {
    auto v = r.view<PlayerTag>();
    return v.begin() == v.end() ? entt::null : *v.begin();
}

struct World {
    entt::registry registry;
    GameConfig     config;
    GameState      state;

    EntityPool<EnemyTag>      enemies;
    EntityPool<ProjectileTag> projectiles;
    EntityPool<XPOrbTag>      xpOrbs;

    SpatialHash enemySpatial;

    std::vector<LightningBolt> lightningBolts;

    // Рекорды и статистика — живут между reset(), не обнуляются.
    SaveData saveData;

    explicit World(GameConfig cfg);

    void reset();
    void spawnPlayer();

    // Считает итог текущего прогона, обновляет saveData, пишет файл.
    // Идемпотентна: повторный вызов ничего не сделает.
    void finalizeRun();
};

}