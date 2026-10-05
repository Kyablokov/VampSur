#pragma once

#include <entt/entt.hpp>

#include <cmath>
#include <unordered_map>

#include "components/components.hpp"
#include "core/config.hpp"

namespace vk {

inline entt::entity createPlayer(entt::registry& registry,
                                 const PlayerConfig&    pcfg,
                                 const CombatConfig&    ccfg,
                                 const WeaponConfig&    wcfg,
                                 const AuraConfig&      acfg,
                                 const OrbitConfig&     ocfg,
                                 const LightningConfig& lcfg,
                                 const XPConfig&        xcfg,
                                 const std::unordered_map<std::string, int>& bonuses) {
    const auto e = registry.create();
    registry.emplace<Position>     (e, pcfg.startX, pcfg.startY);
    registry.emplace<Velocity>     (e, 0.0f, 0.0f);
    registry.emplace<Speed>        (e, pcfg.speed);
    registry.emplace<RenderCircle> (e, pcfg.radius, pcfg.color);
    registry.emplace<Health>       (e, ccfg.playerHp, ccfg.playerHp);
    registry.emplace<PickupRadius> (e, pcfg.pickupRadius);
    registry.emplace<PlayerTag>    (e);
    registry.emplace<ScreenShake>(e);


    XP xp;
    xp.level   = 1;
    xp.current = 0.0f;
    xp.needed  = static_cast<float>(xpNeededForLevel(xcfg, 1));
    registry.emplace<XP>(e, xp);

    Weapon w;
    w.cooldown           = wcfg.cooldown;
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
    aura.color        = acfg.color;
    registry.emplace<AuraWeapon>(e, aura);

    OrbitWeapon orbit;
    orbit.count            = ocfg.baseCount;
    orbit.radius           = ocfg.baseRadius;
    orbit.damage           = ocfg.baseDamage;
    orbit.angularSpeed     = ocfg.angularSpeed;
    orbit.projectileRadius = ocfg.projectileRadius;
    orbit.hitCooldown      = ocfg.hitCooldown;
    orbit.color            = ocfg.color;
    registry.emplace<OrbitWeapon>(e, orbit);

    LightningWeapon lightning;
    lightning.cooldown     = lcfg.cooldown;
    lightning.range        = lcfg.range;
    lightning.damage       = lcfg.damage;
    lightning.targets      = lcfg.targets;
    lightning.chainRadius  = lcfg.chainRadius;
    lightning.boltLifetime = lcfg.boltLifetime;
    lightning.color        = lcfg.color;
    registry.emplace<LightningWeapon>(e, lightning);


        // Применяем постоянные бонусы из магазина
    auto getLevel = [&](const char* id) {
        auto it = bonuses.find(id);
        return it == bonuses.end() ? 0 : it->second;
    };

    const int dmgLvl   = getLevel("perm_damage");
    const int hpLvl    = getLevel("perm_hp");
    const int spdLvl   = getLevel("perm_speed");
    const int pickLvl  = getLevel("perm_pickup");
    const int fireLvl  = getLevel("perm_fire_rate");
    const int xpLvl    = getLevel("perm_xp");

    if (dmgLvl > 0) {
        auto& w = registry.get<Weapon>(e);
        w.projectileDamage *= (1.0f + 0.05f * dmgLvl);
    }
    if (hpLvl > 0) {
        auto& h = registry.get<Health>(e);
        h.max     += 10.0f * hpLvl;
        h.current  = h.max;
    }
    if (spdLvl > 0) {
        registry.get<Speed>(e).value *= (1.0f + 0.03f * spdLvl);
    }
    if (pickLvl > 0) {
        registry.get<PickupRadius>(e).value *= (1.0f + 0.05f * pickLvl);
    }
    if (fireLvl > 0) {
        auto& w = registry.get<Weapon>(e);
        w.cooldown = std::max(0.05f, w.cooldown * std::pow(0.97f, static_cast<float>(fireLvl)));
    }
    if (xpLvl > 0) {
        auto& xp = registry.get<XP>(e);
        xp.needed *= std::pow(0.95f, static_cast<float>(xpLvl));
    }

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
    r.replace<OrbitHitCooldown>(e, 0.0f);
    r.remove<BossTag>(e);   // на случай, если сущность была боссом в прошлой жизни
    r.remove<RangedAttack>(e);   // если был — убираем
}


inline void configureRangedBoss(entt::registry& r, entt::entity e,
                                float x, float y,
                                const RangedBossConfig& cfg,
                                float hpScale) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<Speed>        (e, cfg.speed);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<Health>       (e, cfg.hp * hpScale, cfg.hp * hpScale);
    r.replace<ContactDamage>(e, cfg.contactDamage);
    r.replace<XPValue>      (e, cfg.xpValue);
    r.replace<OrbitHitCooldown>(e, 0.0f);

    RangedAttack attack;
    attack.cooldown           = cfg.attackCooldown;
    attack.timer              = 0.0f;
    attack.keepDistance       = cfg.keepDistance;
    attack.minDistance        = cfg.minDistance;
    attack.projectileSpeed    = cfg.projectileSpeed;
    attack.projectileDamage   = cfg.projectileDamage;
    attack.projectileLifetime = cfg.projectileLifetime;
    attack.projectileRadius   = cfg.projectileRadius;
    attack.projectileColor    = cfg.projectileColor;
    r.emplace_or_replace<RangedAttack>(e, attack);
    // BossTag + RangedAttack = ranged boss
    r.emplace_or_replace<BossTag>(e);
}

inline void configureEnemyProjectile(entt::registry& r, entt::entity e,
                                     float x, float y, float vx, float vy,
                                     const RangedAttack& attack) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, vx, vy);
    r.replace<RenderCircle> (e, attack.projectileRadius, attack.projectileColor);
    r.replace<Lifetime>     (e, attack.projectileLifetime);
    r.replace<EnemyProjectile>(e, attack.projectileDamage);
}


inline void configureChest(entt::registry& r, entt::entity e,
                           float x, float y, const ChestConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<Chest>        (e, cfg.upgradesPerChest);
}

inline void configureHealOrb(entt::registry& r, entt::entity e,
                             float x, float y, const HealOrbConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<HealOrb>      (e, cfg.healAmount);
}


inline void configureCoin(entt::registry& r, entt::entity e,
                          float x, float y, int value, const GoldConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<RenderCircle> (e, cfg.coinRadius, cfg.coinColor);
    r.replace<Coin>         (e, value);
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

inline void configureMagnetOrb(entt::registry& r, entt::entity e,
                               float x, float y, const MagnetConfig& cfg) {
    r.replace<Position>     (e, x, y);
    r.replace<Velocity>     (e, 0.0f, 0.0f);
    r.replace<RenderCircle> (e, cfg.radius, cfg.color);
    r.replace<Lifetime>     (e, cfg.lifetime);
    r.replace<MagnetOrb>    (e, cfg.pullDuration);
}

}