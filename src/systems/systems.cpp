#include "systems/systems.hpp"

#include <cmath>
#include <vector>

#include "components/components.hpp"
#include "core/factory.hpp"   // ← добавить эту строку


namespace vk {

void updateInput(entt::registry& registry, float /*dt*/) {
    float dx = 0.0f, dy = 0.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dy += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dx -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1.0f;

    if (dx != 0.0f && dy != 0.0f) {
        constexpr float kInvSqrt2 = 0.70710678f;
        dx *= kInvSqrt2; dy *= kInvSqrt2;
    }

    registry.view<PlayerTag, Velocity, Speed>().each(
        [&](auto, Velocity& vel, const Speed& speed) {
            vel.x = dx * speed.value;
            vel.y = dy * speed.value;
        });
}

void updateMovement(entt::registry& registry, float dt) {
    registry.view<Position, Velocity>().each(
        [&](auto, Position& pos, const Velocity& vel) {
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
        });
}

void spawnEnemies(entt::registry& registry, GameState& state,
                  const GameConfig& config, float dt) {
    if (state.gameOver) return;

    state.spawnTimer -= dt;
    if (state.spawnTimer > 0.0f) return;
    state.spawnTimer = config.spawner.interval;

    auto playerView = registry.view<PlayerTag, Position>();
    if (playerView.begin() == playerView.end()) return;
    const auto& playerPos = registry.get<Position>(*playerView.begin());

    const auto alive = registry.view<EnemyTag>().size();
    if (alive >= static_cast<std::size_t>(config.spawner.maxEnemies)) return;

    const float angle = randRange(state.rngState, 0.0f, 6.2831853f);
    const float dist  = config.spawner.distance;
    const float x     = playerPos.x + std::cos(angle) * dist;
    const float y     = playerPos.y + std::sin(angle) * dist;

    createEnemy(registry, x, y, config.enemy);
}

void chasePlayer(entt::registry& registry, float /*dt*/) {
    auto playerView = registry.view<PlayerTag, Position>();
    if (playerView.begin() == playerView.end()) return;
    const auto& playerPos = registry.get<Position>(*playerView.begin());

    registry.view<EnemyTag, Position, Velocity, Speed>().each(
        [&](auto, const Position& pos, Velocity& vel, const Speed& speed) {
            const float dx = playerPos.x - pos.x;
            const float dy = playerPos.y - pos.y;
            const float len2 = dx * dx + dy * dy;
            if (len2 < 1e-4f) { vel.x = vel.y = 0.0f; return; }
            const float invLen = 1.0f / std::sqrt(len2);
            vel.x = dx * invLen * speed.value;
            vel.y = dy * invLen * speed.value;
        });
}

void resolveCombat(entt::registry& registry, GameState& state,
                   const GameConfig& config, float dt) {
    // Тикаем неуязвимость
    registry.view<Invulnerability>().each(
        [&](auto, Invulnerability& inv) { inv.remaining -= dt; });

    auto playerView = registry.view<PlayerTag, Position, Health, RenderCircle>();
    if (playerView.begin() == playerView.end()) return;
    const auto playerEntity = *playerView.begin();

    const bool isInvuln =
        registry.all_of<Invulnerability>(playerEntity) &&
        registry.get<Invulnerability>(playerEntity).remaining > 0.0f;
    if (isInvuln) return;

    const auto& playerPos = registry.get<Position>(playerEntity);
    const auto& playerRC  = registry.get<RenderCircle>(playerEntity);

    entt::entity hitEnemy = entt::null;
    float        damage   = 0.0f;

    registry.view<EnemyTag, Position, RenderCircle, ContactDamage>().each(
        [&](auto e, const Position& pos, const RenderCircle& rc, const ContactDamage& dmg) {
            if (hitEnemy != entt::null) return;
            const float dx = pos.x - playerPos.x;
            const float dy = pos.y - playerPos.y;
            const float r  = playerRC.radius + rc.radius;
            if (dx * dx + dy * dy <= r * r) {
                hitEnemy = e;
                damage   = dmg.value;
            }
        });

    if (hitEnemy == entt::null) return;

    registry.destroy(hitEnemy);
    auto& hp = registry.get<Health>(playerEntity);
    hp.current -= damage;
    registry.emplace_or_replace<Invulnerability>(playerEntity, config.combat.playerInvulnTime);

    if (hp.current <= 0.0f) {
        hp.current  = 0.0f;
        state.gameOver = true;
    }
}

void renderCircles(entt::registry& registry) {
    registry.view<Position, RenderCircle>().each(
        [&](auto, const Position& pos, const RenderCircle& rc) {
            DrawCircleV({ pos.x, pos.y }, rc.radius, rc.color);
        });
}

}