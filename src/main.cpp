#include <raylib.h>
#include <entt/entt.hpp>

#include <cmath>
#include <string>

#include "components/components.hpp"
#include "core/config.hpp"
#include "core/game_state.hpp"
#include "core/particles.hpp"
#include "core/upgrades.hpp"
#include "core/world.hpp"
#include "systems/systems.hpp"
#include "core/audio.hpp"
#include "core/save.hpp"   // если ещё нет (для perkCost)

using namespace vk;

namespace {

// ---------------------------------------------------------------- world render

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

// ---------------------------------------------------------------- helpers

std::string formatTime(float seconds) {
    const int total = static_cast<int>(seconds);
    const int m = total / 60;
    const int s = total % 60;
    return TextFormat("%02d:%02d", m, s);
}

int playerLevel(const World& w) {
    auto pv = w.registry.view<PlayerTag, XP>();
    if (pv.begin() == pv.end()) return 1;
    return pv.get<XP>(*pv.begin()).level;
}

// ---------------------------------------------------------------- HUD

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

void drawBuildPanel(const World& w) {
    auto pv = w.registry.view<PlayerTag, Weapon, AuraWeapon, OrbitWeapon, LightningWeapon, Speed>();
    if (pv.begin() == pv.end()) return;

    const auto e = *pv.begin();
    const auto& wc = w.registry.get<Weapon>(e);
    const auto& ac = w.registry.get<AuraWeapon>(e);
    const auto& oc = w.registry.get<OrbitWeapon>(e);
    const auto& lc = w.registry.get<LightningWeapon>(e);
    const auto& sp = w.registry.get<Speed>(e);

    const int sw = w.config.window.width;
    const int sh = w.config.window.height;
    const int x = 10;
    int y = sh - 150;

    DrawRectangle(x - 4, y - 4, 280, 144, Fade(BLACK, 0.55f));

    DrawText("BUILD", x, y, 16, LIGHTGRAY);
    y += 20;

    DrawText(TextFormat("Player:    SPD %5.1f", sp.value),
             x, y, 14, Color{ 120, 240, 160, 255 });
    y += 18;

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

    DrawText(TextFormat("Kills: %d", w.state.stats.kills), 10, 82, 18, ORANGE);
    DrawText(TextFormat("Enemies: %zu / %zu", w.enemies.active(), w.enemies.capacity()),
             10, 104, 16, ORANGE);
    DrawText(TextFormat("Projectiles: %zu / %zu", w.projectiles.active(), w.projectiles.capacity()),
             10, 122, 16, ORANGE);
    DrawText(TextFormat("XP orbs: %zu / %zu", w.xpOrbs.active(), w.xpOrbs.capacity()),
             10, 140, 16, ORANGE);
    DrawText(TextFormat("Particles: %zu / %zu", w.particles.activeCount(), w.particles.capacity()),
             10, 158, 16, ORANGE);
    DrawText(TextFormat("Heal orbs: %zu / %zu", w.healOrbs.active(), w.healOrbs.capacity()),
         10, 176, 16, ORANGE);
    DrawText(TextFormat("Enemy bullets: %zu / %zu",
                    w.enemyProjectiles.active(), w.enemyProjectiles.capacity()),
         10, 176, 16, ORANGE);
        

    auto pv = w.registry.view<PlayerTag, Health>();
    if (pv.begin() != pv.end()) {
        const auto& hp = pv.get<Health>(*pv.begin());
        const float frac = (hp.max > 0.0f) ? (hp.current / hp.max) : 0.0f;
        DrawRectangle(10, 184, 202, 22, Fade(BLACK, 0.6f));
        DrawRectangle(11, 185, static_cast<int>(200 * frac), 20, Fade(RED, 0.9f));
        DrawText(TextFormat("HP %.0f / %.0f", hp.current, hp.max),
                 16, 202, 16, RAYWHITE);
    }

    if (w.state.magnetTimer > 0.0f) {
        DrawText(TextFormat("MAGNET: %.1fs", w.state.magnetTimer),
                 10, 236, 18, Color{ 255, 100, 200, 255 });
    }

    
    const auto& sd = w.saveData;
    const char* goldText = TextFormat("Gold: %d", sd.gold);
    const int goldW = MeasureText(goldText, 22);
    DrawText(goldText, w.config.window.width - goldW - 20, 60, 22,
             Color{ 255, 220, 80, 255 });


    DrawText("[ESC] pause", 10, 260, 16, Color{ 140, 150, 170, 255 });
}

// ---------------------------------------------------------------- upgrade cards

Rectangle cardRect(int slot, int screenW, int screenH) {
    constexpr float cardW = 340.0f;
    constexpr float cardH = 400.0f;
    constexpr float gap   = 40.0f;
    const float total = cardW * 3 + gap * 2;
    const float startX = (screenW - total) * 0.5f;
    const float y = (screenH - cardH) * 0.5f + 40.0f;
    return { startX + slot * (cardW + gap), y, cardW, cardH };
}

Rectangle shopButtonRect(int sw, int sh) {
    return { sw/2.0f - 200.0f, sh/2.0f + 120.0f, 400.0f, 42.0f };
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

    const char* title = w.state.fromChest ? "CHEST!" : "LEVEL UP!";
    const Color titleColor = w.state.fromChest ? Color{ 255, 200, 80, 255 } : GOLD;
    DrawText(title, sw/2 - MeasureText(title, 48)/2, 80, 48, titleColor); 

    if (w.state.pendingUpgrades >= 1) {
        const char* counter = TextFormat("Picks left: %d", w.state.pendingUpgrades);
        DrawText(counter,
                sw/2 - MeasureText(counter, 20)/2,
                140, 20, Color{ 220, 220, 240, 255 });
    }

    const auto& pool = w.config.upgrades;
    const Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; ++i) {
        const int idx = w.state.upgradeOffer[i];
        if (idx < 0 || idx >= static_cast<int>(pool.size())) continue;

        const auto& up = pool[idx];
        const Rectangle r = cardRect(i, sw, sh);
        const bool hovered = CheckCollisionPointRec(mouse, r);

        const RarityStyle style = rarityStyle(up.rarity);
        const Color bg = hovered ? Color{
            static_cast<unsigned char>(style.bg.r + 15),
            static_cast<unsigned char>(style.bg.g + 15),
            static_cast<unsigned char>(style.bg.b + 15),
            255 } : style.bg;
        const Color border = hovered ? style.border : Fade(style.border, 0.65f);

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2.0f, border);

        DrawText(TextFormat("[%d]", i + 1),
                 static_cast<int>(r.x) + 16, static_cast<int>(r.y) + 14,
                 22, Color{ 180, 200, 230, 255 });

        const int badgeW = MeasureText(style.label, 14);
        DrawText(style.label,
                 static_cast<int>(r.x + r.width) - badgeW - 16,
                 static_cast<int>(r.y) + 20, 14, style.badge);

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

// ---------------------------------------------------------------- overlays

void drawPausedOverlay(const World& w) {
    const int sw = w.config.window.width;
    const int sh = w.config.window.height;

    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.55f));

    const char* title = "PAUSED";
    DrawText(title, sw/2 - MeasureText(title, 56)/2, sh/2 - 80, 56, RAYWHITE);

    const char* hint = "Press ESC or P to resume";
    DrawText(hint, sw/2 - MeasureText(hint, 22)/2, sh/2 + 10, 22, Color{ 200, 210, 230, 255 });

    const std::string stats = TextFormat("Time %s   Level %d   Kills %d",
                                         formatTime(w.state.timeSeconds).c_str(),
                                         playerLevel(w),
                                         w.state.stats.kills);
    DrawText(stats.c_str(), sw/2 - MeasureText(stats.c_str(), 18)/2, sh/2 + 60, 18, GOLD);
}

void drawDeathScreen(const World& w) {
    const int sw = w.config.window.width;
    const int sh = w.config.window.height;

    DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.82f));

    const char* title = "GAME OVER";
    DrawText(title, sw/2 - MeasureText(title, 52)/2, 100, 52, RED);

    const int colLabelX = sw/2 - 240;
    const int colValueX = sw/2 + 20;
    int y = 210;

    DrawText("THIS RUN", colValueX + 100, y, 20, Color{ 200, 210, 230, 255 });
    DrawText("BEST",     colValueX + 260, y, 20, GOLD);
    y += 32;

    auto row = [&](const char* label, const char* run, const char* best, bool newBest) {
        DrawText(label, colLabelX, y, 22, LIGHTGRAY);
        DrawText(run,   colValueX + 100, y, 22, RAYWHITE);
        DrawText(best,  colValueX + 260, y, 22, newBest ? GOLD : Color{ 200, 210, 230, 255 });
        if (newBest) DrawText("NEW!", colValueX + 360, y, 22, GOLD);
        y += 34;
    };

    const auto& sd = w.saveData;

    const std::string timeRun  = formatTime(sd.lastTime);
    const std::string timeBest = formatTime(sd.bestTime);
    row("Time",  timeRun.c_str(), timeBest.c_str(), sd.newBestTime);

    const std::string lvlRun  = TextFormat("%d", sd.lastLevel);
    const std::string lvlBest = TextFormat("%d", sd.bestLevel);
    row("Level", lvlRun.c_str(), lvlBest.c_str(), sd.newBestLevel);

    const std::string killRun  = TextFormat("%d", sd.lastKills);
    const std::string killBest = TextFormat("%d", sd.bestKills);
    row("Kills", killRun.c_str(), killBest.c_str(), sd.newBestKills);

    y += 20;
    DrawText(TextFormat("Damage taken this run: %.0f", w.state.stats.damageTaken),
             colLabelX, y, 18, Color{ 220, 160, 160, 255 });
    y += 30;
    DrawText(TextFormat("Total runs: %d", sd.totalRuns),
             colLabelX, y, 18, Color{ 160, 170, 190, 255 });

    const char* hint = "Press SPACE / ENTER or click to return to menu";
    DrawText(hint, sw/2 - MeasureText(hint, 22)/2, sh - 80, 22, RAYWHITE);
}

// ---------------------------------------------------------------- main menu

Rectangle playButtonRect(int sw, int sh) {
    return { sw/2.0f - 200.0f, sh/2.0f - 20.0f, 400.0f, 60.0f };
}

Rectangle resetButtonRect(int sw, int sh) {
    return { sw/2.0f - 200.0f, sh/2.0f + 60.0f, 400.0f, 42.0f };
}




void drawShopScreen(World& w) {
    const int sw = w.config.window.width;
    const int sh = w.config.window.height;

    ClearBackground(Color{ 15, 15, 25, 255 });

    const char* title = "SHOP";
    DrawText(title, sw/2 - MeasureText(title, 56)/2, 40, 56, GOLD);

    // Золото сверху
    const char* goldText = TextFormat("Gold: %d", w.saveData.gold);
    DrawText(goldText, sw/2 - MeasureText(goldText, 24)/2, 110, 24,
             Color{ 255, 220, 80, 255 });

    // Перки
    const auto& items = w.config.shopItems;
    const Vector2 mouse = GetMousePosition();
    int y = 170;

    for (const auto& item : items) {
        int currentLvl = 0;
        auto it = w.saveData.permanentBonuses.find(item.id);
        if (it != w.saveData.permanentBonuses.end()) currentLvl = it->second;

        const int cost = perkCost(item, currentLvl);
        const bool maxed = (currentLvl >= item.maxLevel);
        const bool canBuy = !maxed && (w.saveData.gold >= cost);

        const Rectangle r = { sw/2.0f - 400.0f, static_cast<float>(y), 800.0f, 60.0f };
        const bool hov = CheckCollisionPointRec(mouse, r);

        // Фон
        Color bg = Color{ 25, 25, 40, 255 };
        if (maxed) bg = Color{ 20, 30, 25, 255 };
        else if (!canBuy) bg = Color{ 30, 20, 20, 255 };
        else if (hov) bg = Color{ 40, 40, 65, 255 };

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 1.5f, Color{ 70, 70, 110, 255 });

        // Название + описание
        DrawText(item.name.c_str(),
                 static_cast<int>(r.x) + 20, static_cast<int>(r.y) + 10, 22, RAYWHITE);
        DrawText(item.description.c_str(),
                 static_cast<int>(r.x) + 20, static_cast<int>(r.y) + 36, 15,
                 Color{ 180, 190, 210, 255 });

        // Уровень
        const std::string lvlText = TextFormat("Lv %d / %d", currentLvl, item.maxLevel);
        DrawText(lvlText.c_str(),
                 static_cast<int>(r.x + r.width) - 280, static_cast<int>(r.y) + 20,
                 20, Color{ 200, 210, 230, 255 });

        // Кнопка Buy
        const Rectangle buyBtn = {
            r.x + r.width - 180.0f, r.y + 10.0f, 160.0f, 40.0f
        };

        if (maxed) {
            DrawRectangleRec(buyBtn, Color{ 30, 50, 35, 255 });
            DrawRectangleLinesEx(buyBtn, 1.5f, Color{ 80, 180, 100, 255 });
            const char* txt = "MAXED";
            DrawText(txt,
                     static_cast<int>(buyBtn.x + buyBtn.width/2 - MeasureText(txt, 20)/2),
                     static_cast<int>(buyBtn.y + 10), 20, Color{ 120, 220, 140, 255 });
        } else {
            const Color btnColor = canBuy
                ? (CheckCollisionPointRec(mouse, buyBtn)
                    ? Color{ 80, 160, 80, 255 } : Color{ 50, 110, 50, 255 })
                : Color{ 60, 40, 40, 255 };

            DrawRectangleRec(buyBtn, btnColor);
            DrawRectangleLinesEx(buyBtn, 1.5f,
                                 canBuy ? Color{ 100, 220, 100, 255 } : Color{ 100, 60, 60, 255 });

            const char* txt = TextFormat("Buy: %d", cost);
            DrawText(txt,
                     static_cast<int>(buyBtn.x + buyBtn.width/2 - MeasureText(txt, 20)/2),
                     static_cast<int>(buyBtn.y + 10), 20,
                     canBuy ? RAYWHITE : Color{ 160, 120, 120, 255 });

            if (canBuy && hov &&
                CheckCollisionPointRec(mouse, buyBtn) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                w.saveData.gold -= cost;
                w.saveData.permanentBonuses[item.id] = currentLvl + 1;
                writeSaveFile(w.saveData);
                w.audio.play("levelup", 1.3f);
            }
        }

        y += 70;
    }

    // Назад
    const char* back = "Press ESC / ENTER to return";
    DrawText(back, sw/2 - MeasureText(back, 20)/2, sh - 60, 20,
             Color{ 180, 190, 210, 255 });
}

void drawMainMenu(const World& w) {
    const int sw = w.config.window.width;
    const int sh = w.config.window.height;

    ClearBackground(Color{ 12, 12, 20, 255 });

    const char* title = "VAMPIRE LIKE";
    DrawText(title, sw/2 - MeasureText(title, 64)/2, 70, 64, Color{ 220, 80, 80, 255 });

    const char* sub = "survive as long as you can";
    DrawText(sub, sw/2 - MeasureText(sub, 22)/2, 150, 22, Color{ 140, 150, 170, 255 });

    int y = 220;
    DrawText("BEST RUNS", sw/2 - 100, y, 22, GOLD);
    y += 40;

    const std::string t = formatTime(w.saveData.bestTime);
    DrawText(TextFormat("Time:       %s", t.c_str()), sw/2 - 100, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Level:      %d", w.saveData.bestLevel), sw/2 - 100, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Kills:      %d", w.saveData.bestKills), sw/2 - 100, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Total runs: %d", w.saveData.totalRuns), sw/2 - 100, y, 18,
             Color{ 160, 170, 190, 255 });

    const Rectangle playBtn  = playButtonRect(sw, sh);
    const Rectangle resetBtn = resetButtonRect(sw, sh);

    const Vector2 mouse = GetMousePosition();

    {
        const bool hov = CheckCollisionPointRec(mouse, playBtn);
        DrawRectangleRec(playBtn, hov ? Color{ 60, 160, 80, 255 } : Color{ 40, 100, 55, 255 });
        DrawRectangleLinesEx(playBtn, 2.0f, hov ? LIME : Color{ 80, 180, 100, 255 });
        const char* txt = "PLAY";
        DrawText(txt,
                 static_cast<int>(playBtn.x + playBtn.width/2 - MeasureText(txt, 28)/2),
                 static_cast<int>(playBtn.y + 14), 28, RAYWHITE);
    }

    {
        const bool hov = CheckCollisionPointRec(mouse, resetBtn);
        DrawRectangleRec(resetBtn, hov ? Color{ 100, 40, 40, 255 } : Color{ 55, 25, 25, 255 });
        DrawRectangleLinesEx(resetBtn, 1.5f,
                             hov ? Color{ 240, 100, 100, 255 } : Color{ 140, 60, 60, 255 });
        const char* txt = "Reset Save (R)";
        DrawText(txt,
                 static_cast<int>(resetBtn.x + resetBtn.width/2 - MeasureText(txt, 18)/2),
                 static_cast<int>(resetBtn.y + 12), 18, Color{ 220, 180, 180, 255 });
    }

    {
        const Rectangle shopBtn = shopButtonRect(sw, sh);
        const bool hov = CheckCollisionPointRec(mouse, shopBtn);
        DrawRectangleRec(shopBtn, hov ? Color{ 60, 60, 120, 255 } : Color{ 35, 35, 70, 255 });
        DrawRectangleLinesEx(shopBtn, 1.5f,
                            hov ? Color{ 140, 140, 240, 255 } : Color{ 80, 80, 160, 255 });
        const char* txt = "Shop (S)";
        DrawText(txt,
                static_cast<int>(shopBtn.x + shopBtn.width/2 - MeasureText(txt, 18)/2),
                static_cast<int>(shopBtn.y + 12), 18, Color{ 200, 200, 240, 255 });
    }

    DrawText("Press ENTER to play",
             sw/2 - MeasureText("Press ENTER to play", 18)/2,
             sh - 60, 18, Color{ 140, 150, 170, 255 });
}

bool handleMainMenuInput(World& w) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        return true;
    }
    if (IsKeyPressed(KEY_R)) {
        w.resetSaveFile();
    }
    if (IsKeyPressed(KEY_S)) return false;   // обрабатываем в Menu через прямую смену mode

    const int sw = w.config.window.width;
    const int sh = w.config.window.height;
    const Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, playButtonRect(sw, sh)) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }
    if (CheckCollisionPointRec(mouse, resetButtonRect(sw, sh)) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        w.resetSaveFile();
    }
    return false;
}

} // namespace

int main() {
    GameConfig config = loadGameConfig();

    InitWindow(config.window.width, config.window.height, config.window.title.c_str());
    SetTargetFPS(config.window.targetFps);
    SetExitKey(KEY_NULL);


    World world(config);
    world.audio.init(config.audio);
    world.audio.playMusic("music/bgm.ogg", true);

    world.spawnPlayer();
    world.state.mode = GameMode::MainMenu;

    Camera2D camera{};
    camera.zoom   = 1.0f;
    camera.offset = {
        static_cast<float>(config.window.width)  / 2.0f,
        static_cast<float>(config.window.height) / 2.0f
    };
    camera.target = { config.player.startX, config.player.startY };

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();


        // ----------------------------------------------------------------
        // Логика
        // ----------------------------------------------------------------
        
        world.audio.update();

        switch (world.state.mode) {
            case GameMode::MainMenu: {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                    world.reset();
                }
                if (IsKeyPressed(KEY_R)) {
                    world.resetSaveFile();
                }
                if (IsKeyPressed(KEY_S)) {
                    world.state.mode = GameMode::Shop;
                }
                // клик по кнопкам
                const int sw = world.config.window.width;
                const int sh = world.config.window.height;
                const Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, playButtonRect(sw, sh)) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    world.reset();
                }
                if (CheckCollisionPointRec(mouse, resetButtonRect(sw, sh)) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    world.resetSaveFile();
                }
                if (CheckCollisionPointRec(mouse, shopButtonRect(sw, sh)) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    world.state.mode = GameMode::Shop;
                }
                break;
            }

            case GameMode::Shop: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
                    IsKeyPressed(KEY_KP_ENTER)) {
                    world.state.mode = GameMode::MainMenu;
                }
                break;
            }

            case GameMode::Playing: {
                world.state.timeSeconds += dt;

                updateInput      (world, dt);
                spawnEnemies     (world, dt);
                spawnBosses      (world, dt);
                chasePlayer      (world, dt);
                updateXPMagnet   (world, dt);
                updateMagnets    (world, dt);
                updateHealOrbs(world, dt);
                updateCoins(world, dt);
                updateChests(world, dt);       // после updateHealOrbs
                updateRangedBosses(world, dt);
                updateEnemyProjectiles(world, dt);
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
                resolveMagnetPickup  (world);
                resolveHealPickup(world);
                resolveCoinPickup(world);
                resolveChestPickup(world);     // после resolveHealPickup
                resolveEnemyProjectileHits(world);
                checkLevelUp         (world);

                // Эффекты (шаг 12)
                world.particles.update(dt);
                world.damageNumbers.update(dt);
                updateHitFlashes (world, dt);
                updateScreenShake(world, dt);

                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
                    world.state.mode = GameMode::Paused;
                    world.audio.setMusicPaused(true);
                }
                
                break;
            }

            case GameMode::Upgrading: {
                if (IsKeyPressed(KEY_ONE))   chooseUpgrade(world, 0);
                if (IsKeyPressed(KEY_TWO))   chooseUpgrade(world, 1);
                if (IsKeyPressed(KEY_THREE)) chooseUpgrade(world, 2);
                if (IsKeyPressed(KEY_ESCAPE)) {
                    world.state.mode            = GameMode::Playing;
                    world.state.pendingUpgrades = 0;
                    world.state.fromChest       = false;
                }
                break;
            }

            case GameMode::Paused: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
                    world.state.mode = GameMode::Playing;
                    world.audio.setMusicPaused(false);
                }
                break;
            }

            case GameMode::GameOver: {
                world.finalizeRun();
                const bool restart =
                    IsKeyPressed(KEY_SPACE) ||
                    IsKeyPressed(KEY_ENTER) ||
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                if (restart) {
                    world.state.mode = GameMode::MainMenu;
                }
                break;
            }
        }

        // ----------------------------------------------------------------
        // Камера + Screen shake
        // ----------------------------------------------------------------
        float playerX = 0.0f, playerY = 0.0f;
        if (world.state.mode != GameMode::MainMenu) {
            world.registry.view<PlayerTag, Position>().each(
                [&](auto, const Position& pos) {
                    playerX = pos.x; playerY = pos.y;
                    camera.target = { pos.x, pos.y };
                });

            const float baseOffX = static_cast<float>(config.window.width)  / 2.0f;
            const float baseOffY = static_cast<float>(config.window.height) / 2.0f;

            float shakeMag = 0.0f;
            world.registry.view<ScreenShake>().each(
                [&](auto, const ScreenShake& s) {
                    if (s.remaining > 0.0f && s.maxTime > 0.0f) {
                        const float t = s.remaining / s.maxTime;
                        shakeMag = s.intensity * t;
                    }
                });

            if (shakeMag > 0.01f) {
                camera.offset.x = baseOffX + randRange(world.state.rngState, -shakeMag, shakeMag);
                camera.offset.y = baseOffY + randRange(world.state.rngState, -shakeMag, shakeMag);
            } else {
                camera.offset.x = baseOffX;
                camera.offset.y = baseOffY;
            }
        }

        // ----------------------------------------------------------------
        // Рендер
        // ----------------------------------------------------------------
        BeginDrawing();

        if (world.state.mode == GameMode::MainMenu) {
            drawMainMenu(world);
        }  else if (world.state.mode == GameMode::Shop) {
            drawShopScreen(world); 
        } else {
            ClearBackground(config.world.backgroundColor);

            BeginMode2D(camera);
                drawGrid(static_cast<float>(config.world.gridSize), 5000.0f, config.world.gridColor);
                drawWorldMarkers(160.0f, 900.0f, { playerX, playerY }, Color{ 90, 90, 110, 255 });
                renderAura(world);
                renderOrbit(world);
                renderCircles(world.registry);
                renderMagnets(world);
                renderHealOrbs(world);
                renderCoins(world);
                renderChests(world);
                renderBossHP(world);
                renderLightning(world);
                renderEnemyProjectiles(world);
                world.particles.render();        // ← частицы
                world.damageNumbers.render();    // ← цифры урона (поверх всего)
            EndMode2D();

            DrawFPS(10, 10);
            DrawText(TextFormat("pos: %.1f, %.1f", playerX, playerY), 10, 286, 16, LIME);
            drawXPBar(world);
            drawHUD(world);
            drawUpgradeHUD(world);
            drawBuildPanel(world);

            if (world.state.mode == GameMode::Upgrading) {
                drawUpgradeScreen(world);
            } else if (world.state.mode == GameMode::Paused) {
                drawPausedOverlay(world);
            } else if (world.state.mode == GameMode::GameOver) {
                drawDeathScreen(world);
            }
        }

        EndDrawing();
    }

    world.audio.shutdown();
    CloseWindow();
    return 0;
}