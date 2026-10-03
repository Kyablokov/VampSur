#pragma once

#include <entt/entt.hpp>

#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/pools.hpp"
#include "core/spatial_hash.hpp"
#include "components/components.hpp"

namespace vk {

// Всё состояние мира в одном месте — чтобы системы принимали один аргумент.
struct World {
    entt::registry registry;
    GameConfig     config;
    GameState      state;

    EntityPool<EnemyTag>      enemies;
    EntityPool<ProjectileTag> projectiles;

    SpatialHash enemySpatial;

    explicit World(GameConfig cfg);

    // Сброс партии: все пулы — в исходное, игрок восстановлен, таймеры в 0.
    void reset();

    // Создаёт игрока (один раз при инициализации).
    void spawnPlayer();
};

} // namespace vk