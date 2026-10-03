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
                  r.emplace<XPValue>(e);
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
    , xpOrbs(registry,
             static_cast<std::size_t>(config.xp.poolCapacity),
             [](entt::registry& r, entt::entity e) {
                 r.emplace<Position>(e);
                 r.emplace<Velocity>(e);
                 r.emplace<RenderCircle>(e);
                 r.emplace<XPOrb>(e);
             })
    , enemySpatial(64.0f)
{
}

void World::spawnPlayer() {
    createPlayer(registry, config.player, config.combat,
                 config.weapon, config.aura, config.xp);
}

void World::reset() {
    std::vector<entt::entity> toRelease;

    registry.view<EnemyTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) enemies.release(e);

    toRelease.clear();
    registry.view<ProjectileTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) projectiles.release(e);

    toRelease.clear();
    registry.view<XPOrbTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) xpOrbs.release(e);

    if (auto pe = findPlayer(registry); pe != entt::null) {
        registry.destroy(pe);
    }
    spawnPlayer();

    enemySpatial.clear();
    state = GameState{};
}

} // namespace vk