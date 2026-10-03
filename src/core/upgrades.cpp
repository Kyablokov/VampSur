#include "core/upgrades.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "core/world.hpp"

namespace vk {

namespace {

entt::entity player(World& w) { return findPlayer(w.registry); }

template <typename Fn>
void modifyWeapon(World& w, Fn&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<Weapon>(pe))
        fn(w.registry.get<Weapon>(pe));
}

template <typename Fn>
void modifyAura(World& w, Fn&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<AuraWeapon>(pe))
        fn(w.registry.get<AuraWeapon>(pe));
}

template <typename Fn>
void modifyOrbit(World& w, Fn&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<OrbitWeapon>(pe))
        fn(w.registry.get<OrbitWeapon>(pe));
}

void modifyHealth(World& w, auto&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<Health>(pe))
        fn(w.registry.get<Health>(pe));
}

void modifySpeed(World& w, auto&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<Speed>(pe))
        fn(w.registry.get<Speed>(pe));
}

void modifyPickup(World& w, auto&& fn) {
    if (auto pe = player(w); pe != entt::null && w.registry.all_of<PickupRadius>(pe))
        fn(w.registry.get<PickupRadius>(pe));
}

} // namespace

void applyUpgradeEffect(World& w, const std::string& type, float value) {
    // ---- weapon ----
    if (type == "weapon_damage_mul") {
        modifyWeapon(w, [v = value](Weapon& wp) { wp.projectileDamage *= v; });
    } else if (type == "weapon_cooldown_mul") {
        modifyWeapon(w, [v = value](Weapon& wp) { wp.cooldown = std::max(0.05f, wp.cooldown * v); });
    } else if (type == "weapon_projectile_speed_mul") {
        modifyWeapon(w, [v = value](Weapon& wp) { wp.projectileSpeed *= v; });
    } else if (type == "weapon_projectile_lifetime_mul") {
        modifyWeapon(w, [v = value](Weapon& wp) { wp.projectileLifetime *= v; });
    } else if (type == "weapon_projectile_count_add") {
        modifyWeapon(w, [v = value](Weapon& wp) {
            wp.projectileCount = std::max(1, wp.projectileCount + static_cast<int>(v));
        });
    }
    // ---- player ----
    else if (type == "player_speed_mul") {
        modifySpeed(w, [v = value](Speed& s) { s.value *= v; });
    } else if (type == "player_max_hp_add") {
        modifyHealth(w, [v = value](Health& h) { h.max += v; h.current = h.max; });
    } else if (type == "player_pickup_radius_mul") {
        modifyPickup(w, [v = value](PickupRadius& p) { p.value *= v; });
    }
    // ---- aura ----
    else if (type == "aura_radius_mul") {
        modifyAura(w, [v = value](AuraWeapon& a) { a.radius *= v; });
    } else if (type == "aura_damage_mul") {
        modifyAura(w, [v = value](AuraWeapon& a) { a.damage *= v; });
    } else if (type == "aura_tick_mul") {
        modifyAura(w, [v = value](AuraWeapon& a) { a.tickInterval = std::max(0.1f, a.tickInterval * v); });
    }
    // ---- orbit ----
    else if (type == "orbit_count_add") {
        modifyOrbit(w, [v = value](OrbitWeapon& o) {
            o.count = std::min(32, o.count + static_cast<int>(v));
        });
    } else if (type == "orbit_radius_mul") {
        modifyOrbit(w, [v = value](OrbitWeapon& o) { o.radius *= v; });
    } else if (type == "orbit_damage_mul") {
        modifyOrbit(w, [v = value](OrbitWeapon& o) { o.damage *= v; });
    } else if (type == "orbit_speed_mul") {
        modifyOrbit(w, [v = value](OrbitWeapon& o) { o.angularSpeed *= v; });
    }
    // ---- fallback ----
    else {
        TraceLog(LOG_WARNING, "Unknown upgrade effect: %s", type.c_str());
    }
}

void rollUpgrades(World& w) {
    const auto& pool = w.config.upgrades;
    const int n = static_cast<int>(pool.size());

    w.state.upgradeOffer = { -1, -1, -1 };
    if (n == 0) return;

    const int count = std::min(3, n);

    auto alreadyPicked = [&](int slot, int candidate) {
        for (int j = 0; j < slot; ++j) if (w.state.upgradeOffer[j] == candidate) return true;
        return false;
    };

    for (int i = 0; i < count; ++i) {
        // Считаем суммарный вес среди ещё не выбранных слотов
        float total = 0.0f;
        for (int j = 0; j < n; ++j) {
            if (alreadyPicked(i, j)) continue;
            total += std::max(0.0f, pool[j].weight);
        }
        if (total <= 0.0f) {
            // Все веса нули — берём первый невыбранный
            for (int j = 0; j < n; ++j) {
                if (!alreadyPicked(i, j)) { w.state.upgradeOffer[i] = j; break; }
            }
            continue;
        }

        float r = randRange(w.state.rngState, 0.0f, total);
        int chosen = -1;
        for (int j = 0; j < n; ++j) {
            if (alreadyPicked(i, j)) continue;
            r -= std::max(0.0f, pool[j].weight);
            if (r <= 0.0f) { chosen = j; break; }
        }
        if (chosen < 0) {
            for (int j = n - 1; j >= 0; --j) {
                if (!alreadyPicked(i, j)) { chosen = j; break; }
            }
        }
        w.state.upgradeOffer[i] = chosen;
    }
}

void chooseUpgrade(World& w, int slot) {
    if (slot < 0 || slot >= 3) return;
    const int idx = w.state.upgradeOffer[slot];
    if (idx < 0) return;

    const auto& pool = w.config.upgrades;
    if (idx >= static_cast<int>(pool.size())) return;

    const auto& up = pool[idx];
    applyUpgradeEffect(w, up.effectType, up.effectValue);
    w.state.takenUpgrades[up.id] += 1;

    w.state.upgradeOffer = { -1, -1, -1 };
    w.state.mode = GameMode::Playing;
}

} // namespace vk