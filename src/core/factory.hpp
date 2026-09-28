#pragma once

#include <entt/entt.hpp>

#include "components/components.hpp"
#include "core/config.hpp"

namespace vk {

inline entt::entity createPlayer(entt::registry& registry,
                                 const PlayerConfig& pcfg,
                                 const CombatConfig& ccfg) {
    const auto e = registry.create();
    registry.emplace<Position>     (e, pcfg.startX, pcfg.startY);
    registry.emplace<Velocity>     (e, 0.0f, 0.0f);
    registry.emplace<Speed>        (e, pcfg.speed);
    registry.emplace<RenderCircle> (e, pcfg.radius, pcfg.color);
    registry.emplace<Health>       (e, ccfg.playerHp, ccfg.playerHp);
    registry.emplace<PlayerTag>    (e);
    return e;
}

inline entt::entity createEnemy(entt::registry& registry,
                                float x, float y,
                                const EnemyConfig& cfg) {
    const auto e = registry.create();
    registry.emplace<Position>     (e, x, y);
    registry.emplace<Velocity>     (e, 0.0f, 0.0f);
    registry.emplace<Speed>        (e, cfg.speed);
    registry.emplace<RenderCircle> (e, cfg.radius, cfg.color);
    registry.emplace<Health>       (e, cfg.hp, cfg.hp);
    registry.emplace<ContactDamage>(e, cfg.contactDamage);
    registry.emplace<EnemyTag>     (e);
    return e;
}

}