#include "core/world.hpp"

#include <algorithm>
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
                  r.emplace<OrbitHitCooldown>(e);
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
    , magnetOrbs(registry,
             static_cast<std::size_t>(config.magnet.poolCapacity),
             [](entt::registry& r, entt::entity e) {
                 r.emplace<Position>(e);
                 r.emplace<Velocity>(e);
                 r.emplace<RenderCircle>(e);
                 r.emplace<Lifetime>(e);
                 r.emplace<MagnetOrb>(e);
             })
    , enemySpatial(64.0f)
{
    particles.reserve(static_cast<std::size_t>(config.effects.particlePool));
    damageNumbers.reserve(static_cast<std::size_t>(config.effects.damageNumbersPool));
    lightningBolts.reserve(64);
    saveData = loadSaveFile();
    
}

void World::spawnPlayer() {
    createPlayer(registry, config.player, config.combat,
                 config.weapon, config.aura, config.orbit,
                 config.lightning, config.xp);
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

    toRelease.clear();
    registry.view<MagnetOrbTag>(entt::exclude<Inactive>).each(
        [&](auto e) { toRelease.push_back(e); });
    for (auto e : toRelease) magnetOrbs.release(e);

    if (auto pe = findPlayer(registry); pe != entt::null) {
        registry.destroy(pe);
    }
    spawnPlayer();

    enemySpatial.clear();
    lightningBolts.clear();
    particles.clear();
    damageNumbers.clear();
    state = GameState{};
}

void World::finalizeRun() {
    if (state.runSaved) return;

    int level = 1;
    if (auto pe = findPlayer(registry); pe != entt::null && registry.all_of<XP>(pe)) {
        level = registry.get<XP>(pe).level;
    }

    saveData.lastTime  = state.timeSeconds;
    saveData.lastLevel = level;
    saveData.lastKills = state.stats.kills;

    saveData.newBestTime  = (saveData.lastTime  > saveData.bestTime);
    saveData.newBestLevel = (saveData.lastLevel > saveData.bestLevel);
    saveData.newBestKills = (saveData.lastKills > saveData.bestKills);

    if (saveData.newBestTime)  saveData.bestTime  = saveData.lastTime;
    if (saveData.newBestLevel) saveData.bestLevel = saveData.lastLevel;
    if (saveData.newBestKills) saveData.bestKills = saveData.lastKills;

    saveData.totalRuns++;

    writeSaveFile(saveData);
    state.runSaved = true;
}

void World::resetSaveFile() {
    saveData = SaveData{};
    writeSaveFile(saveData);
}

} // namespace vk