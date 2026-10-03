#include <raylib.h>
#include <entt/entt.hpp>

#include <cmath>

#include "components/components.hpp"
#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/upgrades.hpp"
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

void drawXPBar(const World& w) {
    auto pv = w.registry.view<PlayerTag, XP>();
    if (pv.begin() == pv.end()) return;
    const auto& xp = pv.get<XP>(*pv.begin());

    const float margin = 20.0f;
    const float barW   = static_cast<float>(w.config.window.width) - margin * 2.0f;
    const float barH   = 14.0f;
    const float frac   = (xp.needed > 0.0f) ? (xp.current / xp.needed) : 0.0f;

    DrawRectangle(static_cast<int>(margin), 10, static_cast<int>(barW), static_cast<int>(barH), Fade(BLACK, 0.6f));
    DrawRectangle(static_cast<int>(margin) + 1, 11,
                  static_cast<int>((barW - 2) * frac), static_cast<int>(barH) - 2,
                  Color{ 100, 220, 255, 255 });

    DrawText(TextFormat("Lv %d", xp.level),
             static_cast<int>(margin) + 6, 30, 16, Color{ 180, 230, 255, 255 });
}

void drawUpgradeHUD(const World& w) {
    if (w.state.takenUpgrades.empty()) return;

    const int sw = w.config.window.width;
    const int x  = sw - 240;
    int y = 30;

    DrawText("Upgrades:", x, y, 18, LIGHTGRAY);
    y += 24;

    for (const auto& up : w.config.upgrades) {
        auto it = w.state.takenUpgrades.find(up.id);
        if (it == w.state.takenUpgrades.end() || it->second <= 0) continue;

        DrawText(TextFormat("%s  x%d", up.name.c_str(), it->second), x, y, 16,
                 Color{ 200, 210, 230, 255 });
        y += 20;
    }
}

// Панель текущего «билда» — цифры по оружию
void drawBuildPanel(const World& w) {
    auto pv = w.registry.view<PlayerTag, Weapon, AuraWeapon, OrbitWeapon, LightningWeapon>();
    if (pv.begin() == pv.end()) return;

    const auto e = *pv.begin();
    const auto& wc = w.registry.get<Weapon>(e);
    const auto& ac = w.registry.get<AuraWeapon>(e);
    const auto& oc = w.registry.get<OrbitWeapon>(e);
    const auto& lc = w.registry.get<LightningWeapon>(e);

    const int sw = w.config.window.width;
    const int sh = w.config.window.height;
    const int x = 10;
    int y = sh - 130;

    DrawRectangle(x - 4, y - 4, 260, 124, Fade(BLACK, 0.55f));

    DrawText("BUILD", x, y, 16, LIGHTGRAY);
    y += 20;

    DrawText(TextFormat("Cannon:    DMG %5.1f  CD %.2f  N %d",
                        wc.projectileDamage, wc.cooldown, wc.projectileCount),
             x, y, 14, Color{ 255, 230, 120, 255 });
    y += 18;

    DrawText(TextFormat("Aura:      DMG %5.1f  R %5.1f",
                        ac.damage, ac.radius),
             x, y, 14, Color{ 255, 200, 100, 255 });
    y += 18;

    DrawText(TextFormat("Orbit:     DMG %5.1f  N %d  R %5.1f",
                        oc.damage, oc.count, oc.radius),
             x, y, 14, Color{ 180, 140, 255, 255 });
    y += 18;

    DrawText(TextFormat("Lightning: DMG %5.1f  CD %.2f  T %d",
                        lc.damage, lc.cooldown, lc.targets),
             x, y, 14, Color{ 180, 220, 255, 255 });
}

void drawHUD(const World& w) {
    const int minutes = static_cast<int>(w.state.timeSeconds) / 60;
    const int seconds = static_cast<int>(w.state.timeSeconds) % 60;
    DrawText(TextFormat("%02d:%02d", minutes, seconds),
             w.config.window.width / 2 - 40, 30, 34, RAYWHITE);

    DrawText(TextFormat("Enemies: %zu / %zu", w.enemies.active(), w.enemies.capacity()),
             10, 82, 18, ORANGE);
    DrawText(TextFormat("Projectiles: %zu / %zu", w.projectiles.active(), w.projectiles.capacity()),
             10, 104, 16, ORANGE);
    DrawText(TextFormat("XP orbs: %zu / %zu", w.xpOrbs.active(), w.xpOrbs.capacity()),
             10, 122, 16, ORANGE);

    auto pv = w.registry.view<PlayerTag, Health>();
    if (pv.begin() != pv.end()) {
        const auto& hp = pv.get<Health>(*pv.begin());
        const float frac = (hp.max > 0.0f) ? (hp.current / hp.max) : 0.0f;
        DrawRectangle(10, 148, 202, 22, Fade(BLACK, 0.6f));
        DrawRectangle(11, 149, static_cast<int>(200 * frac), 20, Fade(RED, 0.9f));
        DrawText(TextFormat("HP %.0f / %.0f", hp.current, hp.max),
                 16, 152, 16, RAYWHITE);
    }
}

Rectangle cardRect(int slot, int screenW, int screenH) {
    constexpr float cardW = 340.0f;
    constexpr float cardH = 400.0f;
    constexpr float gap   = 40.0f;
    const float total = cardW * 3 + gap * 2;
    const float startX = (screenW - total) * 0.5f;
    const float y = (screenH - cardH) * 0.5f + 40.0f;
    return { startX + slot * (cardW + gap), y, cardW, cardH };
}

struct RarityStyle {
    Color border;
    Color bg;
    Color badge;
    const char* label;
};

RarityStyle rarityStyle(const std::string& r) {
    if (r == "legendary") return { {255, 180, 50, 255},  {40, 30, 10, 255}, {255, 180, 50, 255},  "LEGENDARY" };
    if (r == "epic")      return { {180, 90, 240, 255},  {34, 20, 50, 255}, {180, 90, 240, 255},  "EPIC" };
    if (r == "rare")      return { {80, 160, 255, 255},  {20, 30, 50, 255}, {80, 160, 255, 255},  "RARE" };
    return                       { {80, 90, 110, 255},   {26, 30, 42, 255}, {160, 170, 190, 255}, "COMMON" };
}

void drawUpgradeScreen(World& w) {
    const int sw = w.config.window.width;
    const int sh = w.config.window.height;

    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.78f));

    const char* title = "LEVEL UP!";
    DrawText(title, sw/2 - MeasureText(title, 48)/2, 80, 48, GOLD);

    const auto& pool = w.config.upgrades;
    const Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; ++i) {
        const int idx = w.state.upgradeOffer[i];
        if (idx < 0 || idx >= static_cast<int>(pool.size())) continue;

        const auto& up = pool[idx];
        const Rectangle r = cardRect(i, sw, sh);
        const bool hovered = CheckCollisionPointRec(mouse, r);

        const RarityStyle style = rarityStyle(up.rarity);
        const Color bg     = hovered ? Color{
            static_cast<unsigned char>(style.bg.r + 15),
            static_cast<unsigned char>(style.bg.g + 15),
            static_cast<unsigned char>(style.bg.b + 15),
            255 } : style.bg;
        const Color border = hovered ? style.border : Fade(style.border, 0.65f);

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2.0f, border);

        // Верхняя строка: [1] и бейдж редкости
        DrawText(TextFormat("[%d]", i + 1),
                 static_cast<int>(r.x) + 16, static_cast<int>(r.y) + 14,
                 22, Color{ 180, 200, 230, 255 });

        const int badgeW = MeasureText(style.label, 14);
        DrawText(style.label,
                 static_cast<int>(r.x + r.width) - badgeW - 16,
                 static_cast<int>(r.y) + 20,
                 14, style.badge);

        const int nameY = static_cast<int>(r.y) + 70;
        DrawText(up.name.c_str(),
                 static_cast<int>(r.x) + 20, nameY, 26, RAYWHITE);

        DrawText(up.description.c_str(),
                 static_cast<int>(r.x) + 20, nameY + 55, 18, Color{ 190, 200, 220, 255 });

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            chooseUpgrade(w, i);
            return;
        }
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

        switch (world.state.mode) {
            case GameMode::Playing: {
                world.state.timeSeconds += dt;

                updateInput      (world, dt);
                spawnEnemies     (world, dt);
                chasePlayer      (world, dt);
                updateXPMagnet   (world, dt);
                updateMovement   (world, dt);

                rebuildSpatial   (world);
                updateWeapons    (world, dt);
                updateProjectiles(world, dt);
                updateAura       (world, dt);
                updateOrbit      (world, dt);
                updateLightning  (world, dt);
                resolveProjectileHits(world);
                resolveOrbitHits     (world, dt);
                resolveContactDamage (world, dt);
                resolveXPPickup      (world);
                checkLevelUp         (world);
                break;
            }
            case GameMode::Upgrading: {
                if (IsKeyPressed(KEY_ONE))   chooseUpgrade(world, 0);
                if (IsKeyPressed(KEY_TWO))   chooseUpgrade(world, 1);
                if (IsKeyPressed(KEY_THREE)) chooseUpgrade(world, 2);
                break;
            }
            case GameMode::GameOver: {
                if (IsKeyPressed(KEY_SPACE)) world.reset();
                break;
            }
        }

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
            renderAura(world);
            renderOrbit(world);
            renderCircles(world.registry);
            renderLightning(world);   // поверх всего, чтобы линии были видны
        EndMode2D();

        DrawFPS(10, 10);
        DrawText("WASD / arrows - move", 10, 200, 18, LIGHTGRAY);
        DrawText(TextFormat("pos: %.1f, %.1f", playerX, playerY), 10, 222, 18, LIME);
        drawXPBar(world);
        drawHUD(world);
        drawUpgradeHUD(world);
        drawBuildPanel(world);

        if (world.state.mode == GameMode::Upgrading) {
            drawUpgradeScreen(world);
        } else if (world.state.mode == GameMode::GameOver) {
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