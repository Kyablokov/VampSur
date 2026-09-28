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

std::string findConfigPath() {
    const char* candidates[] = {
        "assets/config/game.json",
        "../assets/config/game.json",
    };
    for (auto* c : candidates) {
        std::ifstream f(c);
        if (f.is_open()) return c;
    }
#ifdef ASSETS_DIR
    std::string p = std::string(ASSETS_DIR) + "/config/game.json";
    std::ifstream f(p);
    if (f.is_open()) return p;
#endif
    return "assets/config/game.json";
}

} // namespace

GameConfig loadGameConfig(const std::string& path) {
    GameConfig config;
    const std::string actualPath = path.empty() ? findConfigPath() : path;
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
        config.player.speed  = p.value("speed",   config.player.speed);
        config.player.radius = p.value("radius",  config.player.radius);
        config.player.startX = p.value("start_x", config.player.startX);
        config.player.startY = p.value("start_y", config.player.startY);
        if (p.contains("color")) config.player.color = parseColor(p["color"], config.player.color);
    }
    if (j.contains("enemy")) {
        const auto& e = j["enemy"];
        config.enemy.speed         = e.value("speed",          config.enemy.speed);
        config.enemy.radius        = e.value("radius",         config.enemy.radius);
        config.enemy.hp            = e.value("hp",             config.enemy.hp);
        config.enemy.contactDamage = e.value("contact_damage", config.enemy.contactDamage);
        if (e.contains("color")) config.enemy.color = parseColor(e["color"], config.enemy.color);
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
        { "color", colorToJson(c.player.color) },
    };
    j["enemy"] = {
        { "speed", c.enemy.speed }, { "radius", c.enemy.radius },
        { "hp", c.enemy.hp }, { "contact_damage", c.enemy.contactDamage },
        { "color", colorToJson(c.enemy.color) },
    };
    j["spawner"] = {
        { "interval", c.spawner.interval },
        { "distance", c.spawner.distance },
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

    std::ofstream out(path);
    if (!out.is_open()) { TraceLog(LOG_ERROR, "Cannot write config to '%s'", path.c_str()); return; }
    out << j.dump(2) << std::endl;
}

} // namespace vk