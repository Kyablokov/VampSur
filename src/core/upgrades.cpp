#include "core/upgrades.hpp"

#include <algorithm>

#include "core/world.hpp"

namespace vk {

namespace {

entt::entity player(World& w) { return findPlayer(w.registry); }

void modifyWeapon(World& w, auto&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<Weapon>(pe)) {
        fn(w.registry.get<Weapon>(pe));
    }
}

void modifyAura(World& w, auto&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<AuraWeapon>(pe)) {
        fn(w.registry.get<AuraWeapon>(pe));
    }
}

} // namespace

const std::vector<Upgrade>& upgradePool() {
    static const std::vector<Upgrade> pool = {
        // --- Projectile weapon ---
        { "damage", "+25% Damage",
          "Projectiles deal 25% more damage.",
          [](World& w) { modifyWeapon(w, [](Weapon& wp) { wp.projectileDamage *= 1.25f; }); } },
        { "fire_rate", "+20% Fire Rate",
          "Weapon cooldown reduced by 20%.",
          [](World& w) { modifyWeapon(w, [](Weapon& wp) { wp.cooldown = std::max(0.05f, wp.cooldown * 0.8f); }); } },
        { "proj_speed", "+20% Projectile Speed",
          "Projectiles travel faster.",
          [](World& w) { modifyWeapon(w, [](Weapon& wp) { wp.projectileSpeed *= 1.2f; }); } },
        { "proj_lifetime", "+20% Projectile Range",
          "Projectiles live 20% longer.",
          [](World& w) { modifyWeapon(w, [](Weapon& wp) { wp.projectileLifetime *= 1.2f; }); } },
        { "multishot", "+1 Projectile",
          "Fire an additional projectile per shot.",
          [](World& w) { modifyWeapon(w, [](Weapon& wp) { wp.projectileCount += 1; }); } },

        // --- Player ---
        { "move_speed", "+15% Move Speed",
          "You run 15% faster.",
          [](World& w) {
              if (auto pe = player(w); pe != entt::null && w.registry.all_of<Speed>(pe))
                  w.registry.get<Speed>(pe).value *= 1.15f;
          } },
        { "max_hp", "+20 Max HP & Full Heal",
          "Increases maximum health and fully restores it.",
          [](World& w) {
              if (auto pe = player(w); pe != entt::null && w.registry.all_of<Health>(pe)) {
                  auto& hp = w.registry.get<Health>(pe);
                  hp.max += 20.0f;
                  hp.current = hp.max;
              }
          } },
        { "pickup_radius", "+50% Pickup Radius",
          "Experience orbs fly to you from farther away.",
          [](World& w) {
              if (auto pe = player(w); pe != entt::null && w.registry.all_of<PickupRadius>(pe))
                  w.registry.get<PickupRadius>(pe).value *= 1.5f;
          } },

        // --- Aura ---
        { "aura_radius", "+25% Aura Radius",
          "The damage aura reaches farther.",
          [](World& w) { modifyAura(w, [](AuraWeapon& a) { a.radius *= 1.25f; }); } },
        { "aura_damage", "+30% Aura Damage",
          "The damage aura hits harder.",
          [](World& w) { modifyAura(w, [](AuraWeapon& a) { a.damage *= 1.3f; }); } },
        { "aura_tick", "+20% Aura Speed",
          "The damage aura ticks 20% more often.",
          [](World& w) { modifyAura(w, [](AuraWeapon& a) { a.tickInterval = std::max(0.1f, a.tickInterval * 0.8f); }); } },
    };
    return pool;
}

void rollUpgrades(World& w) {
    const auto& pool = upgradePool();
    const int n = static_cast<int>(pool.size());

    w.state.upgradeOffer = { -1, -1, -1 };
    if (n == 0) return;

    const int count = std::min(3, n);
    for (int i = 0; i < count; ++i) {
        int idx = -1;
        bool dup = false;
        do {
            idx = randInt(w.state.rngState, 0, n);
            dup = false;
            for (int j = 0; j < i; ++j) {
                if (w.state.upgradeOffer[j] == idx) { dup = true; break; }
            }
        } while (dup);
        w.state.upgradeOffer[i] = idx;
    }
}

void chooseUpgrade(World& w, int slot) {
    if (slot < 0 || slot >= 3) return;
    const int idx = w.state.upgradeOffer[slot];
    if (idx < 0) return;

    const auto& pool = upgradePool();
    if (idx >= static_cast<int>(pool.size())) return;

    const auto& up = pool[idx];
    up.apply(w);
    w.state.takenUpgrades[up.id] += 1;

    w.state.upgradeOffer = { -1, -1, -1 };
    w.state.mode = GameMode::Playing;
}

} // namespace vk