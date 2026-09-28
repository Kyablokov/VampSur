#include <raylib.h>
#include <entt/entt.hpp>

#include "components/components.hpp"
#include "core/config.hpp"
#include "core/factory.hpp"
#include "systems/systems.hpp"

using namespace vk;

namespace {

// Декоративная сетка вокруг начала координат (пока не бесконечная)
void drawGrid(float gridSize, float halfRange, Color color) {
    for (float x = -halfRange; x <= halfRange; x += gridSize) {
        DrawLineV({ x, -halfRange }, { x, halfRange }, color);
    }
    for (float y = -halfRange; y <= halfRange; y += gridSize) {
        DrawLineV({ -halfRange, y }, { halfRange, y }, color);
    }
}

// Детерминированные точки-маркеры в мире — визуальная опора движения.
// Расположение зависит только от координат, поэтому точки "стоят" в мире,
// а мимо них проплывает камера.
void drawWorldMarkers(float spacing, int range, Color color) {
    const int step = static_cast<int>(spacing);
    for (int x = -range; x <= range; x += step) {
        for (int y = -range; y <= range; y += step) {
            // Псевдослучайное, но детерминированное смещение
            const int hx = (x * 73856093) ^ (y * 19349663);
            const int hy = (x * 83492791) ^ (y * 29943829);
            const float ox = static_cast<float>(hx % 40) - 20.0f;
            const float oy = static_cast<float>(hy % 40) - 20.0f;

            const Vector2 p{
                static_cast<float>(x) + ox,
                static_cast<float>(y) + oy
            };
            DrawCircleV(p, 2.0f, color);
        }
    }
}

} // namespace

int main() {
    // --- Конфиг ---
    GameConfig config = loadGameConfig();

    InitWindow(config.window.width, config.window.height, config.window.title.c_str());
    SetTargetFPS(config.window.targetFps);

    // --- ECS ---
    entt::registry registry;
    createPlayer(registry, config.player);

    // --- Камера ---
    Camera2D camera{};
    camera.zoom   = 1.0f;
    camera.offset = {
        static_cast<float>(config.window.width)  / 2.0f,
        static_cast<float>(config.window.height) / 2.0f
    };
    camera.target = { config.player.startX, config.player.startY };

    // --- Главный цикл ---
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        // Логика
        updateInput(registry, dt);
        updateMovement(registry, dt);

        // Камера следует за игроком + запоминаем позицию для HUD
        float playerX = 0.0f;
        float playerY = 0.0f;
        registry.view<PlayerTag, Position>().each(
            [&](auto, const Position& pos) {
                playerX = pos.x;
                playerY = pos.y;
                camera.target = { pos.x, pos.y };
            });

        // --- Отрисовка ---
        BeginDrawing();
        ClearBackground(config.world.backgroundColor);

        BeginMode2D(camera);
            drawGrid(static_cast<float>(config.world.gridSize), 5000.0f, config.world.gridColor);
            drawWorldMarkers(160.0f, 5000, Color{ 90, 90, 110, 255 });
            renderCircles(registry);
        EndMode2D();

        // HUD
        DrawFPS(10, 10);
        DrawText("WASD / arrows - move", 10, 34, 18, LIGHTGRAY);
        DrawText(TextFormat("pos: %.1f, %.1f", playerX, playerY), 10, 58, 18, LIME);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}