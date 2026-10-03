#pragma once

#include <entt/entt.hpp>

#include "components/components.hpp"
#include "core/config.hpp"

namespace vk {

inline entt::entity createPlayer(entt::registry& registry,
                                 const PlayerConfig&  pcfg,
                                 const CombatConfig&  ccfg,
                                 const WeaponConfig&  wcfg) {
    const auto e = registry.create();
    registry.emplace<Position>     (e, pcfg.startX, pcfg.startY);
    registry.emplace<Velocity>     (e, 0.0f, 0.0f);
    registry.emplace<Speed>        (e, pcfg.speed);
    registry.emplace<RenderCircle> (e, pcfg.radius, pcfg.color);
    registry.emplace<Health>       (e, ccfg.playerHp, ccfg.playerHp);
    registry.emplace<PlayerTag>    (e);

    Weapon w;
    w.cooldown           = wcfg.cooldown;
    w.timer              = 0.0f;
    w.range              = wcfg.range;
    w.projectileSpeed    = wcfg.projectileSpeed;
    w.projectileDamage   = wcfg.projectileDamage;
    w.projectileLifetime = wcfg.projectileLifetime;
    w.projectileRadius   = wcfg.projectileRadius;
    w.projectileColor    = wcfg.projectileColor;
    registry.emplace<Weapon>(e, w);
    return e;
}

// Заполняет компоненты сущности врага из пула. Тег EnemyTag и Inactive уже стоят.
inline void configureEnemy(entt::registry& r, entt::entity e,
                           float x, float y, const EnemyConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<Speed>        (e, cfg.speed);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<Health>       (e, cfg.hp, cfg.hp);
    r.replace<ContactDamage>(e, cfg.contactDamage);
}

// Заполняет компоненты снаряда из пула.
inline void configureProjectile(entt::registry& r, entt::entity e,
                                float x, float y, float vx, float vy,
                                const Weapon& w) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, vx, vy);
    r.replace<RenderCircle> (e, w.projectileRadius, w.projectileColor);
    r.replace<Damage>       (e, w.projectileDamage);
    r.replace<Lifetime>     (e, w.projectileLifetime);
}

} // namespace vk