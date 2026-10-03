#include <raylib.h>
#include <entt/entt.hpp>

#include <cmath>

#include "components/components.hpp"
#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/world.hpp"
#include "systems/systems.hpp"

using namespace vk;

namespace {

void drawGrid(float gridSize, float halfRange, Color color) {
    for (float x = -halfRange; x <= halfRange; x += gridSize)
        DrawLineV({ x, -halfRange }, { x, halfRange }, color);
    for (float y = -halfRange; y <= halfRange; y += gridSize)
        DrawLineV({ -halfRange, y }, { halfRange, y }, color);
}

void drawWorldMarkers(float spacing, float range, Vector2 center, Color color) {
    const int step = static_cast<int>(spacing);
    const int minX = static_cast<int>(std::floor((center.x - range) / step)) * step;
    const int maxX = static_cast<int>(std::ceil ((center.x + range) / step)) * step;
    const int minY = static_cast<int>(std::floor((center.y - range) / step)) * step;
    const int maxY = static_cast<int>(std::ceil ((center.y + range) / step)) * step;

    for (int x = minX; x <= maxX; x += step) {
        for (int y = minY; y <= maxY; y += step) {
            const int hx = (x * 73856093) ^ (y * 19349663);
            const int hy = (x * 83492791) ^ (y * 29943829);
            const float ox = static_cast<float>(((hx % 40) + 40) % 40 - 20);
            const float oy = static_cast<float>(((hy % 40) + 40) % 40 - 20);
            DrawCircleV({ x + ox, y + oy }, 2.0f, color);
        }
    }
}

void drawHUD(const World& w) {
    const int minutes = static_cast<int>(w.state.timeSeconds) / 60;
    const int seconds = static_cast<int>(w.state.timeSeconds) % 60;
    DrawText(TextFormat("%02d:%02d", minutes, seconds),
             w.config.window.width / 2 - 40, 20, 34, RAYWHITE);

    DrawText(TextFormat("Enemies: %zu / %zu",
                        w.enemies.active(), w.enemies.capacity()),
             10, 82, 18, ORANGE);
    DrawText(TextFormat("Projectiles: %zu / %zu",
                        w.projectiles.active(), w.projectiles.capacity()),
             10, 104, 16, ORANGE);

    auto pv = w.registry.view<PlayerTag, Health>();
    if (pv.begin() != pv.end()) {
        const auto& hp = pv.get<Health>(*pv.begin());
        const float frac = (hp.max > 0.0f) ? (hp.current / hp.max) : 0.0f;
        DrawRectangle(10, 130, 202, 22, Fade(BLACK, 0.6f));
        DrawRectangle(11, 131, static_cast<int>(200 * frac), 20, Fade(RED, 0.9f));
        DrawText(TextFormat("HP %.0f / %.0f", hp.current, hp.max),
                 16, 134, 16, RAYWHITE);
    }
}

} // namespace

int main() {
    GameConfig config = loadGameConfig();

    InitWindow(config.window.width, config.window.height, config.window.title.c_str());
    SetTargetFPS(config.window.targetFps);

    World world(config);
    world.spawnPlayer();

    Camera2D camera{};
    camera.zoom   = 1.0f;
    camera.offset = {
        static_cast<float>(config.window.width)  / 2.0f,
        static_cast<float>(config.window.height) / 2.0f
    };
    camera.target = { config.player.startX, config.player.startY };

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();

        if (world.state.gameOver) {
            if (IsKeyPressed(KEY_SPACE)) world.reset();
        } else {
            world.state.timeSeconds += dt;

            updateInput      (world, dt);
            spawnEnemies     (world, dt);
            chasePlayer      (world, dt);
            updateMovement   (world, dt);

            rebuildSpatial   (world);       // актуальные позиции врагов
            updateWeapons    (world, dt);   // спавн снарядов (с velocity на след. кадр)
            updateProjectiles(world, dt);   // тик лайфтаймов
            resolveProjectileHits(world);   // столкновения снаряд↔враг
            resolveContactDamage (world, dt);
        }

        // Камера
        float playerX = 0.0f, playerY = 0.0f;
        world.registry.view<PlayerTag, Position>().each(
            [&](auto, const Position& pos) {
                playerX = pos.x; playerY = pos.y;
                camera.target = { pos.x, pos.y };
            });

        BeginDrawing();
        ClearBackground(config.world.backgroundColor);

        BeginMode2D(camera);
            drawGrid(static_cast<float>(config.world.gridSize), 5000.0f, config.world.gridColor);
            drawWorldMarkers(160.0f, 900.0f, { playerX, playerY }, Color{ 90, 90, 110, 255 });
            renderCircles(world.registry);
        EndMode2D();

        DrawFPS(10, 10);
        DrawText("WASD / arrows - move", 10, 34, 18, LIGHTGRAY);
        DrawText(TextFormat("pos: %.1f, %.1f", playerX, playerY), 10, 58, 18, LIME);
        drawHUD(world);

        if (world.state.gameOver) {
            DrawRectangle(0, 0, config.window.width, config.window.height, Fade(BLACK, 0.65f));
            DrawText("GAME OVER",
                     config.window.width / 2 - 130,
                     config.window.height / 2 - 40, 52, RED);
            DrawText("Press SPACE to restart",
                     config.window.width / 2 - 130,
                     config.window.height / 2 + 30, 24, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}