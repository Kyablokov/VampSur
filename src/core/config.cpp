#include "core/config.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace vk {

namespace {

Color parseColor(const json& j, Color fallback) {
    if (!j.is_array() || j.size() < 3) return fallback;
    auto clampi = [](int v) { return static_cast<unsigned char>(v < 0 ? 0 : (v > 255 ? 255 : v)); };
    return Color{
        clampi(j[0].get<int>()),
        clampi(j[1].get<int>()),
        clampi(j[2].get<int>()),
        clampi(j.size() >= 4 ? j[3].get<int>() : 255)
    };
}

json colorToJson(Color c) { return json::array({ c.r, c.g, c.b, c.a }); }

std::string findInAssets(const char* filename) {
    std::string name = filename;
    const std::string candidates[] = {
        "assets/config/" + name,
        "../assets/config/" + name,
    };
    for (const auto& c : candidates) {
        std::ifstream f(c);
        if (f.is_open()) return c;
    }
#ifdef ASSETS_DIR
    std::string p = std::string(ASSETS_DIR) + "/config/" + name;
    std::ifstream f(p);
    if (f.is_open()) return p;
#endif
    return "assets/config/" + name;
}

EnemyTypeConfig parseEnemyType(const json& e) {
    EnemyTypeConfig cfg;
    cfg.id            = e.value("id",             cfg.id);
    cfg.speed         = e.value("speed",          cfg.speed);
    cfg.radius        = e.value("radius",         cfg.radius);
    cfg.hp            = e.value("hp",             cfg.hp);
    cfg.contactDamage = e.value("contact_damage", cfg.contactDamage);
    cfg.xpValue       = e.value("xp_value",       cfg.xpValue);
    cfg.spawnWeight   = e.value("spawn_weight",   cfg.spawnWeight);
    cfg.unlockTimeSec = e.value("unlock_time_sec", cfg.unlockTimeSec);
    if (e.contains("color")) cfg.color = parseColor(e["color"], cfg.color);
    return cfg;
}

} // namespace

std::vector<UpgradeConfig> loadUpgradesConfig(const std::string& path) {
    std::vector<UpgradeConfig> out;

    std::ifstream file(path);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "Upgrades config not found at '%s'", path.c_str());
        return out;
    }

    json j;
    try { file >> j; }
    catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "Failed to parse upgrades '%s': %s", path.c_str(), e.what());
        return out;
    }

    if (!j.contains("upgrades") || !j["upgrades"].is_array()) {
        TraceLog(LOG_WARNING, "Upgrades config missing 'upgrades' array");
        return out;
    }

    for (const auto& u : j["upgrades"]) {
        UpgradeConfig cfg;
        cfg.id          = u.value("id",          "");
        cfg.name        = u.value("name",        "");
        cfg.description = u.value("description", "");
        cfg.weight      = u.value("weight",      1.0f);
        cfg.rarity      = u.value("rarity",      "common");

        if (u.contains("effect") && u["effect"].is_object()) {
            const auto& eff = u["effect"];
            cfg.effectType  = eff.value("type",  "");
            cfg.effectValue = eff.value("value", 0.0f);
        }

        if (cfg.id.empty() || cfg.effectType.empty()) {
            TraceLog(LOG_WARNING, "Skipping malformed upgrade entry");
            continue;
        }
        out.push_back(std::move(cfg));
    }

    TraceLog(LOG_INFO, "Loaded %zu upgrades from '%s'", out.size(), path.c_str());
    return out;
}

GameConfig loadGameConfig(const std::string& path) {
    GameConfig config;
    config.enemyTypes.push_back(EnemyTypeConfig{});

    const std::string actualPath = path.empty() ? findInAssets("game.json") : path;
    std::ifstream file(actualPath);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, "Config not found at '%s', using defaults", actualPath.c_str());
        return config;
    }

    json j;
    try { file >> j; }
    catch (const std::exception& e) {
        TraceLog(LOG_ERROR, "Failed to parse config '%s': %s", actualPath.c_str(), e.what());
        return config;
    }

    if (j.contains("window")) {
        const auto& w = j["window"];
        config.window.width     = w.value("width",      config.window.width);
        config.window.height    = w.value("height",     config.window.height);
        config.window.targetFps = w.value("target_fps", config.window.targetFps);
        config.window.title     = w.value("title",      config.window.title);
    }
    if (j.contains("player")) {
        const auto& p = j["player"];
        config.player.speed        = p.value("speed",         config.player.speed);
        config.player.radius       = p.value("radius",        config.player.radius);
        config.player.startX       = p.value("start_x",       config.player.startX);
        config.player.startY       = p.value("start_y",       config.player.startY);
        config.player.pickupRadius = p.value("pickup_radius", config.player.pickupRadius);
        if (p.contains("color")) config.player.color = parseColor(p["color"], config.player.color);
    }
    if (j.contains("enemy_types") && j["enemy_types"].is_array()) {
        config.enemyTypes.clear();
        for (const auto& e : j["enemy_types"]) config.enemyTypes.push_back(parseEnemyType(e));
        if (config.enemyTypes.empty()) config.enemyTypes.push_back(EnemyTypeConfig{});
    }
    if (j.contains("weapon")) {
        const auto& w = j["weapon"];
        config.weapon.cooldown           = w.value("cooldown",            config.weapon.cooldown);
        config.weapon.range              = w.value("range",               config.weapon.range);
        config.weapon.projectileSpeed    = w.value("projectile_speed",    config.weapon.projectileSpeed);
        config.weapon.projectileDamage   = w.value("projectile_damage",   config.weapon.projectileDamage);
        config.weapon.projectileLifetime = w.value("projectile_lifetime", config.weapon.projectileLifetime);
        config.weapon.projectileRadius   = w.value("projectile_radius",   config.weapon.projectileRadius);
        config.weapon.projectileCount    = w.value("projectile_count",    config.weapon.projectileCount);
        config.weapon.projectileSpread   = w.value("projectile_spread",   config.weapon.projectileSpread);
        config.weapon.poolCapacity       = w.value("pool_capacity",       config.weapon.poolCapacity);
        if (w.contains("projectile_color")) config.weapon.projectileColor = parseColor(w["projectile_color"], config.weapon.projectileColor);
    }
    if (j.contains("aura")) {
        const auto& a = j["aura"];
        config.aura.baseRadius   = a.value("base_radius",   config.aura.baseRadius);
        config.aura.baseDamage   = a.value("base_damage",   config.aura.baseDamage);
        config.aura.tickInterval = a.value("tick_interval", config.aura.tickInterval);
        if (a.contains("color")) config.aura.color = parseColor(a["color"], config.aura.color);
    }
    if (j.contains("orbit")) {
        const auto& o = j["orbit"];
        config.orbit.baseCount        = o.value("base_count",        config.orbit.baseCount);
        config.orbit.baseRadius       = o.value("base_radius",       config.orbit.baseRadius);
        config.orbit.baseDamage       = o.value("base_damage",       config.orbit.baseDamage);
        config.orbit.angularSpeed     = o.value("angular_speed",     config.orbit.angularSpeed);
        config.orbit.projectileRadius = o.value("projectile_radius", config.orbit.projectileRadius);
        config.orbit.hitCooldown      = o.value("hit_cooldown",      config.orbit.hitCooldown);
        if (o.contains("color")) config.orbit.color = parseColor(o["color"], config.orbit.color);
    }
    if (j.contains("lightning")) {
        const auto& l = j["lightning"];
        config.lightning.cooldown     = l.value("cooldown",      config.lightning.cooldown);
        config.lightning.range        = l.value("range",         config.lightning.range);
        config.lightning.damage       = l.value("damage",        config.lightning.damage);
        config.lightning.targets      = l.value("targets",       config.lightning.targets);
        config.lightning.chainRadius  = l.value("chain_radius",  config.lightning.chainRadius);
        config.lightning.boltLifetime = l.value("bolt_lifetime", config.lightning.boltLifetime);
        if (l.contains("color")) config.lightning.color = parseColor(l["color"], config.lightning.color);
    }
    if (j.contains("xp")) {
        const auto& x = j["xp"];
        config.xp.orbRadius         = x.value("orb_radius",         config.xp.orbRadius);
        config.xp.magnetSpeed       = x.value("magnet_speed",       config.xp.magnetSpeed);
        config.xp.poolCapacity      = x.value("pool_capacity",      config.xp.poolCapacity);
        config.xp.baseRequirement   = x.value("base_requirement",   config.xp.baseRequirement);
        config.xp.requirementGrowth = x.value("requirement_growth", config.xp.requirementGrowth);
        if (x.contains("orb_color")) config.xp.orbColor = parseColor(x["orb_color"], config.xp.orbColor);
    }
    if (j.contains("spawner")) {
        const auto& s = j["spawner"];
        config.spawner.interval   = s.value("interval",    config.spawner.interval);
        config.spawner.distance   = s.value("distance",    config.spawner.distance);
        config.spawner.maxEnemies = s.value("max_enemies", config.spawner.maxEnemies);
    }
    if (j.contains("combat")) {
        const auto& c = j["combat"];
        config.combat.playerHp         = c.value("player_hp",          config.combat.playerHp);
        config.combat.playerInvulnTime = c.value("player_invuln_time", config.combat.playerInvulnTime);
    }
    if (j.contains("world")) {
        const auto& wd = j["world"];
        config.world.gridSize = wd.value("grid_size", config.world.gridSize);
        if (wd.contains("background_color")) config.world.backgroundColor = parseColor(wd["background_color"], config.world.backgroundColor);
        if (wd.contains("grid_color"))       config.world.gridColor       = parseColor(wd["grid_color"],       config.world.gridColor);
    }
    if (j.contains("boss")) {
    const auto& b = j["boss"];
    config.boss.interval          = b.value("interval",              config.boss.interval);
    config.boss.radius            = b.value("radius",                config.boss.radius);
    config.boss.speed             = b.value("speed",                 config.boss.speed);
    config.boss.hp                = b.value("hp",                    config.boss.hp);
    config.boss.contactDamage     = b.value("contact_damage",        config.boss.contactDamage);
    config.boss.xpValue           = b.value("xp_value",              config.boss.xpValue);
    config.boss.hpGrowthPerMinute = b.value("hp_growth_per_minute",  config.boss.hpGrowthPerMinute);
    if (b.contains("color")) config.boss.color = parseColor(b["color"], config.boss.color);
    }
    if (j.contains("difficulty")) {
        const auto& d = j["difficulty"];
        config.difficulty.enemyHpGrowthPerMinute      = d.value("enemy_hp_growth_per_minute",      config.difficulty.enemyHpGrowthPerMinute);
        config.difficulty.spawnIntervalDecayPerMinute = d.value("spawn_interval_decay_per_minute", config.difficulty.spawnIntervalDecayPerMinute);
        config.difficulty.spawnIntervalMin            = d.value("spawn_interval_min",              config.difficulty.spawnIntervalMin);
    }
    if (j.contains("magnet")) {
        const auto& m = j["magnet"];
        config.magnet.radius       = m.value("radius",        config.magnet.radius);
        config.magnet.lifetime     = m.value("lifetime",      config.magnet.lifetime);
        config.magnet.pullDuration = m.value("pull_duration", config.magnet.pullDuration);
        config.magnet.poolCapacity = m.value("pool_capacity", config.magnet.poolCapacity);
        if (m.contains("color")) config.magnet.color = parseColor(m["color"], config.magnet.color);
    }
    if (j.contains("effects")) {
        const auto& e = j["effects"];
        config.effects.hitParticles          = e.value("hit_particles",           config.effects.hitParticles);
        config.effects.deathParticles        = e.value("death_particles",         config.effects.deathParticles);
        config.effects.muzzleParticles       = e.value("muzzle_particles",        config.effects.muzzleParticles);
        config.effects.particlePool          = e.value("particle_pool",           config.effects.particlePool);
        config.effects.particleDrag          = e.value("particle_drag",           config.effects.particleDrag);
        config.effects.damageNumbersPool     = e.value("damage_numbers_pool",     config.effects.damageNumbersPool);
        config.effects.damageNumbersLifetime = e.value("damage_numbers_lifetime", config.effects.damageNumbersLifetime);
        config.effects.shakeOnShoot          = e.value("shake_on_shoot",          config.effects.shakeOnShoot);
        config.effects.shakeOnHit            = e.value("shake_on_hit",            config.effects.shakeOnHit);
        config.effects.shakeOnBossDeath      = e.value("shake_on_boss_death",     config.effects.shakeOnBossDeath);
        config.effects.hitFlashDuration      = e.value("hit_flash_duration",      config.effects.hitFlashDuration);
    }
    if (j.contains("audio")) {
        const auto& a = j["audio"];
        config.audio.masterVolume       = a.value("master_volume",          config.audio.masterVolume);
        config.audio.sfxVolume          = a.value("sfx_volume",             config.audio.sfxVolume);
        config.audio.musicVolume        = a.value("music_volume",           config.audio.musicVolume);
        config.audio.killSfxMinInterval = a.value("kill_sfx_min_interval",  config.audio.killSfxMinInterval);
        config.audio.hitSfxMinInterval  = a.value("hit_sfx_min_interval",   config.audio.hitSfxMinInterval);
    }
    if (j.contains("heal_orb")) {
        const auto& h = j["heal_orb"];
        config.healOrb.healAmount   = h.value("heal_amount",   config.healOrb.healAmount);
        config.healOrb.dropChance   = h.value("drop_chance",   config.healOrb.dropChance);
        config.healOrb.radius       = h.value("radius",        config.healOrb.radius);
        config.healOrb.poolCapacity = h.value("pool_capacity", config.healOrb.poolCapacity);
        if (h.contains("color")) config.healOrb.color = parseColor(h["color"], config.healOrb.color);
    }
    if (j.contains("chest")) {
        const auto& c = j["chest"];
        config.chest.upgradesPerChest = c.value("upgrades_per_chest", config.chest.upgradesPerChest);
        config.chest.radius           = c.value("radius",             config.chest.radius);
        config.chest.poolCapacity     = c.value("pool_capacity",      config.chest.poolCapacity);
        if (c.contains("color")) config.chest.color = parseColor(c["color"], config.chest.color);
    }
    config.upgrades = loadUpgradesConfig(findInAssets("upgrades.json"));

    TraceLog(LOG_INFO, "Config loaded from '%s'", actualPath.c_str());
    return config;
}

void saveGameConfig(const GameConfig& c, const std::string& path) {
    json j;
    j["window"] = {
        { "width", c.window.width }, { "height", c.window.height },
        { "target_fps", c.window.targetFps }, { "title", c.window.title },
    };
    j["player"] = {
        { "speed", c.player.speed }, { "radius", c.player.radius },
        { "start_x", c.player.startX }, { "start_y", c.player.startY },
        { "pickup_radius", c.player.pickupRadius },
        { "color", colorToJson(c.player.color) },
    };
    j["enemy_types"] = json::array();
    for (const auto& e : c.enemyTypes) {
        j["enemy_types"].push_back({
            { "id", e.id }, { "speed", e.speed }, { "radius", e.radius },
            { "color", colorToJson(e.color) }, { "hp", e.hp },
            { "contact_damage", e.contactDamage },
            { "xp_value", e.xpValue }, { "spawn_weight", e.spawnWeight },
            { "unlock_time_sec", e.unlockTimeSec }
        });
    }
    j["weapon"] = {
        { "cooldown", c.weapon.cooldown }, { "range", c.weapon.range },
        { "projectile_speed", c.weapon.projectileSpeed },
        { "projectile_damage", c.weapon.projectileDamage },
        { "projectile_lifetime", c.weapon.projectileLifetime },
        { "projectile_radius", c.weapon.projectileRadius },
        { "projectile_color", colorToJson(c.weapon.projectileColor) },
        { "projectile_count", c.weapon.projectileCount },
        { "projectile_spread", c.weapon.projectileSpread },
        { "pool_capacity", c.weapon.poolCapacity },
    };
    j["aura"] = {
        { "base_radius", c.aura.baseRadius }, { "base_damage", c.aura.baseDamage },
        { "tick_interval", c.aura.tickInterval }, { "color", colorToJson(c.aura.color) },
    };
    j["orbit"] = {
        { "base_count", c.orbit.baseCount }, { "base_radius", c.orbit.baseRadius },
        { "base_damage", c.orbit.baseDamage }, { "angular_speed", c.orbit.angularSpeed },
        { "projectile_radius", c.orbit.projectileRadius },
        { "hit_cooldown", c.orbit.hitCooldown },
        { "color", colorToJson(c.orbit.color) },
    };
    j["lightning"] = {
        { "cooldown", c.lightning.cooldown }, { "range", c.lightning.range },
        { "damage", c.lightning.damage }, { "targets", c.lightning.targets },
        { "chain_radius", c.lightning.chainRadius },
        { "bolt_lifetime", c.lightning.boltLifetime },
        { "color", colorToJson(c.lightning.color) },
    };
    j["xp"] = {
        { "orb_radius", c.xp.orbRadius }, { "orb_color", colorToJson(c.xp.orbColor) },
        { "magnet_speed", c.xp.magnetSpeed }, { "pool_capacity", c.xp.poolCapacity },
        { "base_requirement", c.xp.baseRequirement }, { "requirement_growth", c.xp.requirementGrowth },
    };
    j["spawner"] = {
        { "interval", c.spawner.interval }, { "distance", c.spawner.distance },
        { "max_enemies", c.spawner.maxEnemies },
    };
    j["combat"] = {
        { "player_hp", c.combat.playerHp },
        { "player_invuln_time", c.combat.playerInvulnTime },
    };
    j["world"] = {
        { "grid_size", c.world.gridSize },
        { "background_color", colorToJson(c.world.backgroundColor) },
        { "grid_color", colorToJson(c.world.gridColor) },
    };
    j["boss"] = {
    { "interval", c.boss.interval }, { "radius", c.boss.radius },
    { "speed", c.boss.speed }, { "color", colorToJson(c.boss.color) },
    { "hp", c.boss.hp }, { "contact_damage", c.boss.contactDamage },
    { "xp_value", c.boss.xpValue },
    { "hp_growth_per_minute", c.boss.hpGrowthPerMinute },
    };
    j["difficulty"] = {
        { "enemy_hp_growth_per_minute", c.difficulty.enemyHpGrowthPerMinute },
        { "spawn_interval_decay_per_minute", c.difficulty.spawnIntervalDecayPerMinute },
        { "spawn_interval_min", c.difficulty.spawnIntervalMin },
    };
    j["magnet"] = {
        { "radius", c.magnet.radius }, { "color", colorToJson(c.magnet.color) },
        { "lifetime", c.magnet.lifetime }, { "pull_duration", c.magnet.pullDuration },
        { "pool_capacity", c.magnet.poolCapacity },
    };
    j["effects"] = {
        { "hit_particles",           c.effects.hitParticles },
        { "death_particles",         c.effects.deathParticles },
        { "muzzle_particles",        c.effects.muzzleParticles },
        { "particle_pool",           c.effects.particlePool },
        { "particle_drag",           c.effects.particleDrag },
        { "damage_numbers_pool",     c.effects.damageNumbersPool },
        { "damage_numbers_lifetime", c.effects.damageNumbersLifetime },
        { "shake_on_shoot",          c.effects.shakeOnShoot },
        { "shake_on_hit",            c.effects.shakeOnHit },
        { "shake_on_boss_death",     c.effects.shakeOnBossDeath },
        { "hit_flash_duration",      c.effects.hitFlashDuration },
    };
    j["audio"] = {
        { "master_volume",          c.audio.masterVolume },
        { "sfx_volume",             c.audio.sfxVolume },
        { "music_volume",           c.audio.musicVolume },
        { "kill_sfx_min_interval",  c.audio.killSfxMinInterval },
        { "hit_sfx_min_interval",   c.audio.hitSfxMinInterval },
    };
    j["heal_orb"] = {
        { "heal_amount",   c.healOrb.healAmount },
        { "drop_chance",   c.healOrb.dropChance },
        { "radius",        c.healOrb.radius },
        { "color",         colorToJson(c.healOrb.color) },
        { "pool_capacity", c.healOrb.poolCapacity },
    };
    j["chest"] = {
        { "upgrades_per_chest", c.chest.upgradesPerChest },
        { "radius",             c.chest.radius },
        { "color",              colorToJson(c.chest.color) },
        { "pool_capacity",      c.chest.poolCapacity },
    };

    std::ofstream out(path);

    if (!out.is_open()) { TraceLog(LOG_ERROR, "Cannot write config to '%s'", path.c_str()); return; }
    out << j.dump(2) << std::endl;
}

} // namespace vk