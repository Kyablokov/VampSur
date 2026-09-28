#pragma once

#include <string>
#include <raylib.h>

namespace vk {

struct WindowConfig {
    int         width     = 1280;
    int         height    = 720;
    int         targetFps = 60;
    std::string title     = "Vampire Like";
};

struct PlayerConfig {
    float speed  = 220.0f;
    float radius = 14.0f;
    float startX = 0.0f;
    float startY = 0.0f;
    Color color  = {80, 200, 120, 255};
};

struct EnemyConfig {
    float speed         = 90.0f;
    float radius        = 10.0f;
    Color color         = {200, 60, 60, 255};
    float hp            = 10.0f;
    float contactDamage = 8.0f;
};

struct SpawnerConfig {
    float interval   = 0.7f;
    float distance   = 700.0f;
    int   maxEnemies = 500;
};

struct CombatConfig {
    float playerHp         = 100.0f;
    float playerInvulnTime = 0.6f;
};

struct WorldConfig {
    int   gridSize        = 64;
    Color backgroundColor = {18, 18, 24, 255};
    Color gridColor       = {55, 55, 75, 255};
};

struct GameConfig {
    WindowConfig  window;
    PlayerConfig  player;
    EnemyConfig   enemy;
    SpawnerConfig spawner;
    CombatConfig  combat;
    WorldConfig   world;
};

GameConfig loadGameConfig(const std::string& path = "");
void       saveGameConfig(const GameConfig& config, const std::string& path);

}