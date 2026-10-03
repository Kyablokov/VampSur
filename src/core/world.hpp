#pragma once

#include <entt/entt.hpp>

#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/pools.hpp"
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

    explicit World(GameConfig cfg);

    void reset();
    void spawnPlayer();
};

} // namespace vk