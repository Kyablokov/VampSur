#include "core/world.hpp"

#include <vector>

#include "core/factory.hpp"

namespace vk {

World::World(GameConfig cfg)
    : config(std::move(cfg))
    , enemies(registry,
              static_cast<std::size_t>(config.spawner.maxEnemies),
              [](entt::registry& r, entt::entity e) {
                  r.emplace<Position>(e);
                  r.emplace<Velocity>(e);
                  r.emplace<Speed>(e);
                  r.emplace<RenderCircle>(e);
                  r.emplace<Health>(e);
                  r.emplace<ContactDamage>(e);
              })
    , projectiles(registry,
                  static_cast<std::size_t>(config.weapon.poolCapacity),
                  [](entt::registry& r, entt::entity e) {
                      r.emplace<Position>(e);
                      r.emplace<Velocity>(e);
                      r.emplace<RenderCircle>(e);
                      r.emplace<Damage>(e);
                      r.emplace<Lifetime>(e);
                  })
    , enemySpatial(64.0f)
{
}

void World::spawnPlayer() {
    createPlayer(registry, config.player, config.combat, config.weapon);
}

void World::reset() {
    // Возвращаем всех активных врагов в пул
    std::vector<entt::entity> toRelease;
    registry.view<EnemyTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) enemies.release(e);

    // Возвращаем все снаряды в пул
    toRelease.clear();
    registry.view<ProjectileTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) projectiles.release(e);

    // Восстанавливаем игрока
    auto pv = registry.view<PlayerTag, Health>();
    if (pv.begin() != pv.end()) {
        const auto pe = *pv.begin();
        auto& hp = pv.get<Health>(pe);
        hp.current = hp.max;
        registry.remove<Invulnerability>(pe);
        if (registry.all_of<Weapon>(pe)) {
            registry.get<Weapon>(pe).timer = 0.0f;
        }
        if (registry.all_of<Position>(pe)) {
            auto& pos = registry.get<Position>(pe);
            pos.x = config.player.startX;
            pos.y = config.player.startY;
        }
        if (registry.all_of<Velocity>(pe)) {
            registry.get<Velocity>(pe) = {};
        }
    } else {
        // Игрока почему-то нет — создаём заново
        spawnPlayer();
    }

    enemySpatial.clear();
    state = GameState{};
}

} // namespace vk