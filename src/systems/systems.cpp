#include "systems/systems.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "components/components.hpp"
#include "core/factory.hpp"
#include "core/upgrades.hpp"

namespace vk {

namespace {
constexpr float kTwoPi = 6.2831853f;
}

// ---------------------------------------------------------------- input

void updateInput(World& w, float /*dt*/) {
    float dx = 0.0f, dy = 0.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dy += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dx -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1.0f;

    if (dx != 0.0f && dy != 0.0f) {
        constexpr float kInvSqrt2 = 0.70710678f;
        dx *= kInvSqrt2; dy *= kInvSqrt2;
    }

    w.registry.view<PlayerTag, Velocity, Speed>().each(
        [&](auto, Velocity& vel, const Speed& speed) {
            vel.x = dx * speed.value;
            vel.y = dy * speed.value;
        });
}

// ---------------------------------------------------------------- movement

void updateMovement(World& w, float dt) {
    w.registry.view<Position, Velocity>(entt::exclude<Inactive>).each(
        [&](auto, Position& pos, const Velocity& vel) {
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
        });
}

// ---------------------------------------------------------------- spawner

namespace {

const EnemyTypeConfig& pickEnemyType(World& w) {
    const auto& types = w.config.enemyTypes;
    float total = 0.0f;
    for (const auto& t : types) total += (t.spawnWeight > 0.0f ? t.spawnWeight : 0.0f);
    if (total <= 0.0f) return types.front();

    float r = randRange(w.state.rngState, 0.0f, total);
    for (const auto& t : types) {
        r -= (t.spawnWeight > 0.0f ? t.spawnWeight : 0.0f);
        if (r <= 0.0f) return t;
    }
    return types.back();
}

} // namespace

void spawnEnemies(World& w, float dt) {
    w.state.spawnTimer -= dt;
    if (w.state.spawnTimer > 0.0f) return;
    w.state.spawnTimer = w.config.spawner.interval;

    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    const auto& ppos = w.registry.get<Position>(pe);

    if (w.enemies.active() >= static_cast<std::size_t>(w.config.spawner.maxEnemies)) return;

    const auto e = w.enemies.acquire();
    if (e == entt::null) return;

    const float angle = randRange(w.state.rngState, 0.0f, kTwoPi);
    const float dist  = w.config.spawner.distance;
    const float x     = ppos.x + std::cos(angle) * dist;
    const float y     = ppos.y + std::sin(angle) * dist;

    configureEnemy(w.registry, e, x, y, pickEnemyType(w));
}

// ---------------------------------------------------------------- AI

void chasePlayer(World& w, float /*dt*/) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    const auto& ppos = w.registry.get<Position>(pe);

    w.registry.view<EnemyTag, Position, Velocity, Speed>(entt::exclude<Inactive>).each(
        [&](auto, const Position& pos, Velocity& vel, const Speed& speed) {
            const float dx = ppos.x - pos.x;
            const float dy = ppos.y - pos.y;
            const float len2 = dx * dx + dy * dy;
            if (len2 < 1e-4f) { vel.x = vel.y = 0.0f; return; }
            const float invLen = 1.0f / std::sqrt(len2);
            vel.x = dx * invLen * speed.value;
            vel.y = dy * invLen * speed.value;
        });
}

// ---------------------------------------------------------------- spatial

void rebuildSpatial(World& w) {
    w.enemySpatial.clear();
    w.registry.view<EnemyTag, Position>(entt::exclude<Inactive>).each(
        [&](auto e, const Position& pos) {
            w.enemySpatial.insert(e, pos.x, pos.y);
        });
}

// ---------------------------------------------------------------- weapons

void updateWeapons(World& w, float dt) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<Weapon, Position>(pe)) return;

    auto& weapon = w.registry.get<Weapon>(pe);
    auto& ppos   = w.registry.get<Position>(pe);

    weapon.timer -= dt;
    if (weapon.timer > 0.0f) return;
    weapon.timer = weapon.cooldown;

    std::vector<entt::entity> candidates;
    candidates.reserve(64);
    w.enemySpatial.query(ppos.x, ppos.y, weapon.range, candidates);

    entt::entity target = entt::null;
    float bestD2 = weapon.range * weapon.range;
    for (auto e : candidates) {
        if (!w.registry.valid(e)) continue;
        if (w.registry.all_of<Inactive>(e)) continue;
        const auto& ep = w.registry.get<Position>(e);
        const float dx = ep.x - ppos.x;
        const float dy = ep.y - ppos.y;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) { bestD2 = d2; target = e; }
    }
    if (target == entt::null) return;

    const auto& tp = w.registry.get<Position>(target);
    const float baseAngle = std::atan2(tp.y - ppos.y, tp.x - ppos.x);

    const int shots = (weapon.projectileCount < 1) ? 1 : weapon.projectileCount;
    const float spreadStep  = weapon.projectileSpread;
    const float startOffset = -spreadStep * (shots - 1) * 0.5f;

    for (int i = 0; i < shots; ++i) {
        const float ang = baseAngle + startOffset + spreadStep * i;
        const float ax = std::cos(ang);
        const float ay = std::sin(ang);

        const auto proj = w.projectiles.acquire();
        if (proj == entt::null) break;
        configureProjectile(w.registry, proj,
                            ppos.x, ppos.y,
                            ax * weapon.projectileSpeed,
                            ay * weapon.projectileSpeed,
                            weapon);
    }
}

// ---------------------------------------------------------------- projectiles

void updateProjectiles(World& w, float dt) {
    std::vector<entt::entity> expired;
    w.registry.view<ProjectileTag, Lifetime>(entt::exclude<Inactive>).each(
        [&](auto e, Lifetime& lt) {
            lt.remaining -= dt;
            if (lt.remaining <= 0.0f) expired.push_back(e);
        });
    for (auto e : expired) w.projectiles.release(e);
}

void resolveProjectileHits(World& w) {
    std::vector<entt::entity> candidates;
    std::vector<entt::entity> hitProjectiles;
    std::vector<entt::entity> killedEnemies;

    w.registry.view<ProjectileTag, Position, RenderCircle, Damage>(entt::exclude<Inactive>).each(
        [&](auto proj, const Position& pp, const RenderCircle& prc, const Damage& dmg) {
            candidates.clear();
            w.enemySpatial.query(pp.x, pp.y, prc.radius + 64.0f, candidates);

            for (auto e : candidates) {
                if (!w.registry.valid(e)) continue;
                if (w.registry.all_of<Inactive>(e)) continue;
                const auto& ep  = w.registry.get<Position>(e);
                const auto& erc = w.registry.get<RenderCircle>(e);
                const float dx = ep.x - pp.x;
                const float dy = ep.y - pp.y;
                const float r  = prc.radius + erc.radius;
                if (dx * dx + dy * dy > r * r) continue;

                auto& hp = w.registry.get<Health>(e);
                hp.current -= dmg.value;
                if (hp.current <= 0.0f) killedEnemies.push_back(e);

                hitProjectiles.push_back(proj);
                break;
            }
        });

    for (auto e : killedEnemies) {
        w.state.stats.kills += 1;   // ← добавить
        const auto& pos = w.registry.get<Position>(e);
        const float value = w.registry.get<XPValue>(e).value;
        if (auto orb = w.xpOrbs.acquire(); orb != entt::null) {
            configureXPOrb(w.registry, orb, pos.x, pos.y, value, w.config.xp);
        }
        w.enemies.release(e);
    }
    for (auto p : hitProjectiles) w.projectiles.release(p);
}

// ---------------------------------------------------------------- contact

void resolveContactDamage(World& w, float dt) {
    w.registry.view<Invulnerability>().each(
        [&](auto, Invulnerability& inv) { inv.remaining -= dt; });

    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<Position, Health, RenderCircle>(pe)) return;

    const bool isInvuln = w.registry.all_of<Invulnerability>(pe) &&
                          w.registry.get<Invulnerability>(pe).remaining > 0.0f;
    if (isInvuln) return;

    const auto& ppos = w.registry.get<Position>(pe);
    const auto& prc  = w.registry.get<RenderCircle>(pe);

    entt::entity hitEnemy = entt::null;
    float        damage   = 0.0f;

    w.registry.view<EnemyTag, Position, RenderCircle, ContactDamage>(entt::exclude<Inactive>).each(
        [&](auto e, const Position& pos, const RenderCircle& rc, const ContactDamage& dmg) {
            if (hitEnemy != entt::null) return;
            const float dx = pos.x - ppos.x;
            const float dy = pos.y - ppos.y;
            const float r  = prc.radius + rc.radius;
            if (dx * dx + dy * dy <= r * r) {
                hitEnemy = e;
                damage   = dmg.value;
            }
        });

    if (hitEnemy == entt::null) return;

    w.enemies.release(hitEnemy);

    auto& hp = w.registry.get<Health>(pe);
    w.state.stats.damageTaken += damage;
    hp.current -= damage;
    w.registry.emplace_or_replace<Invulnerability>(pe, w.config.combat.playerInvulnTime);

    if (hp.current <= 0.0f) {
        hp.current = 0.0f;
        w.state.mode = GameMode::GameOver;
    }
}

// ---------------------------------------------------------------- aura

void updateAura(World& w, float dt) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<AuraWeapon, Position>(pe)) return;

    auto& aura = w.registry.get<AuraWeapon>(pe);
    const auto& ppos = w.registry.get<Position>(pe);

    aura.timer -= dt;
    if (aura.timer > 0.0f) return;
    aura.timer = aura.tickInterval;

    std::vector<entt::entity> killed;

    w.registry.view<EnemyTag, Position, RenderCircle, Health>(entt::exclude<Inactive>).each(
        [&](auto e, const Position& pos, const RenderCircle& rc, Health& hp) {
            const float dx = pos.x - ppos.x;
            const float dy = pos.y - ppos.y;
            const float r  = aura.radius + rc.radius;
            if (dx * dx + dy * dy > r * r) return;

            hp.current -= aura.damage;
            if (hp.current <= 0.0f) killed.push_back(e);
        });

    for (auto e : killed) {
        w.state.stats.kills += 1;   // ← добавить
        const auto& pos = w.registry.get<Position>(e);
        const float value = w.registry.get<XPValue>(e).value;
        if (auto orb = w.xpOrbs.acquire(); orb != entt::null) {
            configureXPOrb(w.registry, orb, pos.x, pos.y, value, w.config.xp);
        }
        w.enemies.release(e);
    }
}

// ---------------------------------------------------------------- orbit

namespace {

inline void computeOrbitPositions(const OrbitWeapon& orbit, const Position& ppos,
                                  std::array<Vector2, 32>& out) {
    const int n = std::min(orbit.count, 32);
    if (n <= 0) return;
    const float step = kTwoPi / static_cast<float>(n);
    for (int i = 0; i < n; ++i) {
        const float a = orbit.currentAngle + step * static_cast<float>(i);
        out[i] = {
            ppos.x + std::cos(a) * orbit.radius,
            ppos.y + std::sin(a) * orbit.radius
        };
    }
}

} // namespace

void updateOrbit(World& w, float dt) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<OrbitWeapon>(pe)) return;

    auto& orbit = w.registry.get<OrbitWeapon>(pe);
    orbit.currentAngle += orbit.angularSpeed * dt;
    if (orbit.currentAngle >= kTwoPi) orbit.currentAngle -= kTwoPi;
    if (orbit.currentAngle < 0.0f)    orbit.currentAngle += kTwoPi;
}

void resolveOrbitHits(World& w, float dt) {
    w.registry.view<OrbitHitCooldown>().each(
        [&](auto, OrbitHitCooldown& c) {
            if (c.remaining > 0.0f) c.remaining -= dt;
        });

    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<OrbitWeapon, Position>(pe)) return;

    const auto& orbit = w.registry.get<OrbitWeapon>(pe);
    const auto& ppos  = w.registry.get<Position>(pe);

    if (orbit.count <= 0) return;

    std::array<Vector2, 32> positions{};
    computeOrbitPositions(orbit, ppos, positions);
    const int n = std::min(orbit.count, 32);

    std::vector<entt::entity> killed;

    w.registry.view<EnemyTag, Position, RenderCircle, Health, OrbitHitCooldown>(
        entt::exclude<Inactive>).each(
        [&](auto e, const Position& ep, const RenderCircle& erc,
            Health& hp, OrbitHitCooldown& cd) {
            if (cd.remaining > 0.0f) return;

            for (int i = 0; i < n; ++i) {
                const float dx = ep.x - positions[i].x;
                const float dy = ep.y - positions[i].y;
                const float r  = orbit.projectileRadius + erc.radius;
                if (dx * dx + dy * dy > r * r) continue;

                hp.current -= orbit.damage;
                cd.remaining = orbit.hitCooldown;
                if (hp.current <= 0.0f) killed.push_back(e);
                return;
            }
        });

    for (auto e : killed) {
        w.state.stats.kills += 1;   // ← добавить
        const auto& pos = w.registry.get<Position>(e);
        const float value = w.registry.get<XPValue>(e).value;
        if (auto orb = w.xpOrbs.acquire(); orb != entt::null) {
            configureXPOrb(w.registry, orb, pos.x, pos.y, value, w.config.xp);
        }
        w.enemies.release(e);
    }
}

// ---------------------------------------------------------------- lightning

void updateLightning(World& w, float dt) {
    // Тикаем все активные эффекты, удаляем истёкшие
    for (auto& bolt : w.lightningBolts) bolt.remaining -= dt;
    w.lightningBolts.erase(
        std::remove_if(w.lightningBolts.begin(), w.lightningBolts.end(),
                       [](const LightningBolt& b) { return b.remaining <= 0.0f; }),
        w.lightningBolts.end());

    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<LightningWeapon, Position>(pe)) return;

    auto& lightning = w.registry.get<LightningWeapon>(pe);
    const auto& ppos = w.registry.get<Position>(pe);

    lightning.timer -= dt;
    if (lightning.timer > 0.0f) return;
    lightning.timer = lightning.cooldown;

    if (lightning.targets <= 0) return;

    // Накапливаем список поражённых
    std::vector<entt::entity> struck;
    std::vector<entt::entity> killed;

    // Первая цель: ближайший враг в радиусе
    Vector2 cursor = { ppos.x, ppos.y };
    float searchRadius = lightning.range;

    for (int step = 0; step < lightning.targets; ++step) {
        entt::entity best = entt::null;
        float bestD2 = searchRadius * searchRadius;

        w.registry.view<EnemyTag, Position>(entt::exclude<Inactive>).each(
            [&](auto e, const Position& ep) {
                // Уже поражён в этой цепочке?
                for (auto s : struck) if (s == e) return;

                const float dx = ep.x - cursor.x;
                const float dy = ep.y - cursor.y;
                const float d2 = dx * dx + dy * dy;
                if (d2 < bestD2) { bestD2 = d2; best = e; }
            });

        if (best == entt::null) break;

        const auto& bp = w.registry.get<Position>(best);

        // Записываем визуальный эффект
        LightningBolt bolt;
        bolt.from        = cursor;
        bolt.to          = { bp.x, bp.y };
        bolt.remaining   = lightning.boltLifetime;
        bolt.maxLife     = lightning.boltLifetime;
        bolt.color       = lightning.color;
        w.lightningBolts.push_back(bolt);

        // Наносим урон
        if (w.registry.all_of<Health>(best)) {
            auto& hp = w.registry.get<Health>(best);
            hp.current -= lightning.damage;
            if (hp.current <= 0.0f) killed.push_back(best);
        }

        struck.push_back(best);
        cursor = { bp.x, bp.y };
        searchRadius = lightning.chainRadius;
    }

    // Дроп XP и освобождение
    for (auto e : killed) {
        w.state.stats.kills += 1;   // ← добавить
        const auto& pos = w.registry.get<Position>(e);
        const float value = w.registry.get<XPValue>(e).value;
        if (auto orb = w.xpOrbs.acquire(); orb != entt::null) {
            configureXPOrb(w.registry, orb, pos.x, pos.y, value, w.config.xp);
        }
        w.enemies.release(e);
    }
}

// ---------------------------------------------------------------- XP

void updateXPMagnet(World& w, float dt) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<Position, PickupRadius>(pe)) return;

    const auto& ppos = w.registry.get<Position>(pe);
    const float radius = w.registry.get<PickupRadius>(pe).value;
    const float r2 = radius * radius;
    const float speed = w.config.xp.magnetSpeed;

    w.registry.view<XPOrbTag, Position, Velocity>(entt::exclude<Inactive>).each(
        [&](auto, Position& pos, Velocity& vel) {
            const float dx = ppos.x - pos.x;
            const float dy = ppos.y - pos.y;
            const float d2 = dx * dx + dy * dy;
            if (d2 > r2) { vel.x = vel.y = 0.0f; return; }
            const float len = std::sqrt(d2);
            if (len < 1e-4f) { vel.x = vel.y = 0.0f; return; }
            const float t = 1.0f - (len / radius);
            const float sp = speed * (0.35f + 0.65f * t);
            vel.x = dx / len * sp;
            vel.y = dy / len * sp;
        });
}

void resolveXPPickup(World& w) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<Position, RenderCircle, XP>(pe)) return;

    const auto& ppos = w.registry.get<Position>(pe);
    const auto& prc  = w.registry.get<RenderCircle>(pe);
    auto& xp = w.registry.get<XP>(pe);

    std::vector<entt::entity> picked;
    w.registry.view<XPOrbTag, Position, RenderCircle, XPOrb>(entt::exclude<Inactive>).each(
        [&](auto e, const Position& pos, const RenderCircle& rc, const XPOrb& orb) {
            const float dx = pos.x - ppos.x;
            const float dy = pos.y - ppos.y;
            const float r = prc.radius + rc.radius + 2.0f;
            if (dx * dx + dy * dy <= r * r) {
                xp.current += orb.value;
                picked.push_back(e);
            }
        });

    for (auto e : picked) w.xpOrbs.release(e);
}

void checkLevelUp(World& w) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<XP>(pe)) return;

    auto& xp = w.registry.get<XP>(pe);
    if (xp.current < xp.needed) return;

    xp.current -= xp.needed;
    xp.level  += 1;
    xp.needed  = static_cast<float>(xpNeededForLevel(w.config.xp, xp.level));

    rollUpgrades(w);
    w.state.mode = GameMode::Upgrading;
}

// ---------------------------------------------------------------- render

void renderAura(World& w) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<AuraWeapon, Position>(pe)) return;

    const auto& aura = w.registry.get<AuraWeapon>(pe);
    const auto& pos  = w.registry.get<Position>(pe);

    DrawCircleV({ pos.x, pos.y }, aura.radius, Fade(aura.color, 0.35f));
    DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y),
                    aura.radius, Fade(aura.color, 0.9f));
}

void renderOrbit(World& w) {
    auto pe = findPlayer(w.registry);
    if (pe == entt::null) return;
    if (!w.registry.all_of<OrbitWeapon, Position>(pe)) return;

    const auto& orbit = w.registry.get<OrbitWeapon>(pe);
    const auto& ppos  = w.registry.get<Position>(pe);

    if (orbit.count <= 0) return;

    std::array<Vector2, 32> positions{};
    computeOrbitPositions(orbit, ppos, positions);
    const int n = std::min(orbit.count, 32);

    for (int i = 0; i < n; ++i) {
        DrawCircleV(positions[i], orbit.projectileRadius * 2.0f, Fade(orbit.color, 0.15f));
        DrawCircleV(positions[i], orbit.projectileRadius, orbit.color);
    }
}

void renderLightning(World& w) {
    for (const auto& b : w.lightningBolts) {
        const float t = (b.maxLife > 0.0f) ? (b.remaining / b.maxLife) : 0.0f;
        const Color c = Fade(b.color, t);
        // Основная линия + внешнее свечение
        DrawLineEx(b.from, b.to, 6.0f, Fade(b.color, t * 0.25f));
        DrawLineEx(b.from, b.to, 2.0f, c);
    }
}

void renderCircles(entt::registry& r) {
    r.view<Position, RenderCircle>(entt::exclude<Inactive>).each(
        [&](auto, const Position& pos, const RenderCircle& rc) {
            DrawCircleV({ pos.x, pos.y }, rc.radius, rc.color);
        });
}

} // namespace vk