#pragma once

#include <entt/entt.hpp>

#include "components/components.hpp"
#include "core/config.hpp"

namespace vk {

inline entt::entity createPlayer(entt::registry& registry,
                                 const PlayerConfig&  pcfg,
                                 const CombatConfig&  ccfg,
                                 const WeaponConfig&  wcfg,
                                 const AuraConfig&    acfg,
                                 const XPConfig&      xcfg) {
    const auto e = registry.create();
    registry.emplace<Position>     (e, pcfg.startX, pcfg.startY);
    registry.emplace<Velocity>     (e, 0.0f, 0.0f);
    registry.emplace<Speed>        (e, pcfg.speed);
    registry.emplace<RenderCircle> (e, pcfg.radius, pcfg.color);
    registry.emplace<Health>       (e, ccfg.playerHp, ccfg.playerHp);
    registry.emplace<PickupRadius> (e, pcfg.pickupRadius);
    registry.emplace<PlayerTag>    (e);

    XP xp;
    xp.level   = 1;
    xp.current = 0.0f;
    xp.needed  = static_cast<float>(xpNeededForLevel(xcfg, 1));
    registry.emplace<XP>(e, xp);

    Weapon w;
    w.cooldown           = wcfg.cooldown;
    w.timer              = 0.0f;
    w.range              = wcfg.range;
    w.projectileSpeed    = wcfg.projectileSpeed;
    w.projectileDamage   = wcfg.projectileDamage;
    w.projectileLifetime = wcfg.projectileLifetime;
    w.projectileRadius   = wcfg.projectileRadius;
    w.projectileColor    = wcfg.projectileColor;
    w.projectileCount    = wcfg.projectileCount;
    w.projectileSpread   = wcfg.projectileSpread;
    registry.emplace<Weapon>(e, w);

    AuraWeapon aura;
    aura.radius       = acfg.baseRadius;
    aura.damage       = acfg.baseDamage;
    aura.tickInterval = acfg.tickInterval;
    aura.timer        = 0.0f;
    aura.color        = acfg.color;
    registry.emplace<AuraWeapon>(e, aura);

    return e;
}

inline void configureEnemy(entt::registry& r, entt::entity e,
                           float x, float y,
                           const EnemyTypeConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<Speed>        (e, cfg.speed);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<Health>       (e, cfg.hp, cfg.hp);
    r.replace<ContactDamage>(e, cfg.contactDamage);
    r.replace<XPValue>      (e, cfg.xpValue);
}

inline void configureProjectile(entt::registry& r, entt::entity e,
                                float x, float y, float vx, float vy,
                                const Weapon& w) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, vx, vy);
    r.replace<RenderCircle> (e, w.projectileRadius, w.projectileColor);
    r.replace<Damage>       (e, w.projectileDamage);
    r.replace<Lifetime>     (e, w.projectileLifetime);
}

inline void configureXPOrb(entt::registry& r, entt::entity e,
                           float x, float y, float value,
                           const XPConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<RenderCircle> (e, cfg.orbRadius, cfg.orbColor);
    r.replace<XPOrb>        (e, value);
}

}