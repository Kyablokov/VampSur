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

struct WorldConfig {
    int   gridSize        = 64;
    Color backgroundColor = {18, 18, 24, 255};
    Color gridColor       = {32, 32, 40, 255};
};

struct GameConfig {
    WindowConfig window;
    PlayerConfig player;
    WorldConfig  world;
};

// Загружает конфиг. Если path пустой — ищет game.json автоматически.
// При любой ошибке возвращает значения по умолчанию (структуры выше).
GameConfig loadGameConfig(const std::string& path = "");

// Сохраняет текущий конфиг (пригодится позже — например, для сохранений прогресса).
void saveGameConfig(const GameConfig& config, const std::string& path);

}