#pragma once

#include <string>
#include <vector>
#include <raylib.h>

namespace vk {

struct WindowConfig {
    int         width     = 1280;
    int         height    = 720;
    int         targetFps = 60;
    std::string title     = "Vampire Like";
};

struct PlayerConfig {
    float speed        = 220.0f;
    float radius       = 14.0f;
    float startX       = 0.0f;
    float startY       = 0.0f;
    Color color        = {80, 200, 120, 255};
    float pickupRadius = 120.0f;
};

struct EnemyTypeConfig {
    std::string id            = "grunt";
    float       speed         = 90.0f;
    float       radius        = 10.0f;
    Color       color         = {200, 60, 60, 255};
    float       hp            = 20.0f;
    float       contactDamage = 8.0f;
    float       xpValue       = 1.0f;
    float       spawnWeight   = 1.0f;
    float       unlockTimeSec = 0.0f;   // ← новое
};
struct MagnetConfig {
    float radius       = 8.0f;
    Color color        = {255, 100, 200, 255};
    float lifetime     = 20.0f;
    float pullDuration = 4.0f;
    int   poolCapacity = 16;
};

struct WeaponConfig {
    float cooldown           = 0.6f;
    float range              = 550.0f;
    float projectileSpeed    = 620.0f;
    float projectileDamage   = 10.0f;
    float projectileLifetime = 1.5f;
    float projectileRadius   = 4.0f;
    Color projectileColor    = {255, 220, 90, 255};
    int   projectileCount    = 1;
    float projectileSpread   = 0.15f;
    int   poolCapacity       = 400;
};

struct AuraConfig {
    float baseRadius   = 90.0f;
    float baseDamage   = 6.0f;
    float tickInterval = 0.5f;
    Color color        = {255, 200, 100, 60};
};

struct OrbitConfig {
    int   baseCount        = 2;
    float baseRadius       = 70.0f;
    float baseDamage       = 8.0f;
    float angularSpeed     = 3.0f;
    float projectileRadius = 6.0f;
    float hitCooldown      = 0.4f;
    Color color            = {180, 120, 255, 255};
};

struct LightningConfig {
    float cooldown     = 2.0f;
    float range        = 480.0f;
    float damage       = 25.0f;
    int   targets      = 2;
    float chainRadius  = 220.0f;
    float boltLifetime = 0.15f;
    Color color        = {200, 240, 255, 255};
};

struct BossConfig {
    float interval            = 60.0f;
    float radius              = 34.0f;
    float speed               = 45.0f;
    Color color               = {255, 40, 100, 255};
    float hp                  = 800.0f;
    float contactDamage       = 25.0f;
    float xpValue             = 50.0f;
    float hpGrowthPerMinute   = 0.5f;
};

struct RangedBossConfig {
    float radius              = 30.0f;
    float speed               = 70.0f;
    Color color               = {80, 180, 255, 255};
    float hp                  = 600.0f;
    float contactDamage       = 15.0f;
    float xpValue             = 50.0f;
    float hpGrowthPerMinute   = 0.5f;
    float keepDistance        = 380.0f;
    float minDistance         = 220.0f;
    float attackCooldown      = 1.8f;
    float projectileSpeed     = 340.0f;
    float projectileDamage    = 12.0f;
    float projectileLifetime  = 3.0f;
    float projectileRadius    = 7.0f;
    Color projectileColor     = {120, 200, 255, 255};
    int   poolCapacity        = 64;
};
struct DifficultyConfig {
    float enemyHpGrowthPerMinute      = 0.33f;
    float spawnIntervalDecayPerMinute = 0.05f;
    float spawnIntervalMin            = 0.15f;
};
struct XPConfig {
    float orbRadius         = 5.0f;
    Color orbColor          = {100, 220, 255, 255};
    float magnetSpeed       = 480.0f;
    int   poolCapacity      = 800;
    int   baseRequirement   = 5;
    int   requirementGrowth = 4;
};

struct SpawnerConfig {
    float interval   = 0.7f;
    float distance   = 700.0f;
    int   maxEnemies = 500;
};
struct AudioConfig {
    float masterVolume        = 0.7f;
    float sfxVolume           = 1.0f;
    float musicVolume         = 0.35f;
    float killSfxMinInterval  = 0.03f;
    float hitSfxMinInterval   = 0.02f;
};
struct CombatConfig {
    float playerHp         = 70.0f;
    float playerInvulnTime = 0.6f;
};

struct WorldConfig {
    int   gridSize        = 64;
    Color backgroundColor = {18, 18, 24, 255};
    Color gridColor       = {55, 55, 75, 255};
};

struct UpgradeConfig {
    std::string id;
    std::string name;
    std::string description;
    float       weight      = 1.0f;
    std::string rarity      = "common";   // common | rare | epic | legendary
    std::string effectType;
    float       effectValue = 0.0f;
};

struct EffectsConfig {
    int   hitParticles          = 4;
    int   deathParticles        = 12;
    int   muzzleParticles       = 3;
    int   particlePool          = 1024;
    float particleDrag          = 6.0f;
    int   damageNumbersPool     = 64;
    float damageNumbersLifetime = 0.7f;
    float shakeOnShoot          = 1.5f;
    float shakeOnHit            = 5.0f;
    float shakeOnBossDeath      = 14.0f;
    float hitFlashDuration      = 0.08f;
};

struct HealOrbConfig {
    float healAmount  = 15.0f;
    float dropChance  = 0.05f;
    float radius      = 6.0f;
    Color color       = {100, 255, 120, 255};
    int   poolCapacity = 32;
};

struct ChestConfig {
    int   upgradesPerChest = 3;
    float radius           = 14.0f;
    Color color            = {255, 200, 80, 255};
    int   poolCapacity     = 4;
};

struct GoldConfig {
    float dropChance   = 0.15f;
    int   bossDropMin  = 5;
    int   bossDropMax  = 10;
    float coinRadius   = 5.0f;
    Color coinColor    = {255, 220, 80, 255};
    int   poolCapacity = 256;
    float magnetSpeed  = 500.0f;
};

struct ShopItemConfig {
    std::string id;
    std::string name;
    std::string description;
    int   baseCost   = 50;
    float costGrowth = 1.5f;
    int   maxLevel   = 10;
};

struct GameConfig {
    WindowConfig                 window;
    PlayerConfig                 player;
    std::vector<EnemyTypeConfig> enemyTypes;
    WeaponConfig                 weapon;
    AuraConfig                   aura;
    OrbitConfig                  orbit;
    LightningConfig              lightning;
    XPConfig                     xp;
    SpawnerConfig                spawner;
    CombatConfig                 combat;
    WorldConfig                  world;
    std::vector<UpgradeConfig>   upgrades;
    BossConfig       boss;
    DifficultyConfig difficulty;
    MagnetConfig magnet;
    EffectsConfig effects;
    AudioConfig audio;
    HealOrbConfig healOrb;
    ChestConfig chest;
    RangedBossConfig rangedBoss;
    GoldConfig                 gold;
    std::vector<ShopItemConfig> shopItems;


};


GameConfig loadGameConfig(const std::string& path = "");
void       saveGameConfig(const GameConfig& config, const std::string& path);
std::vector<UpgradeConfig> loadUpgradesConfig(const std::string& path);
std::vector<ShopItemConfig> loadShopConfig(const std::string& path);

inline int xpNeededForLevel(const XPConfig& cfg, int level) {
    return cfg.baseRequirement + (level - 1) * cfg.requirementGrowth;
}

}