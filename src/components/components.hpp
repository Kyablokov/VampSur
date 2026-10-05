#pragma once

#include <raylib.h>

namespace vk {

struct Position     { float x = 0.0f, y = 0.0f; };
struct Velocity     { float x = 0.0f, y = 0.0f; };
struct Speed        { float value = 0.0f; };
struct RenderCircle { float radius = 10.0f; Color color = WHITE; };

struct Health         { float current = 1.0f; float max = 1.0f; };
struct ContactDamage  { float value = 0.0f; };
struct Invulnerability{ float remaining = 0.0f; };
struct Damage         { float value = 0.0f; };
struct Lifetime       { float remaining = 0.0f; };
struct XPValue        { float value = 1.0f; };

struct Weapon {
    float cooldown           = 0.6f;
    float timer              = 0.0f;
    float range              = 550.0f;
    float projectileSpeed    = 620.0f;
    float projectileDamage   = 10.0f;
    float projectileLifetime = 1.5f;
    float projectileRadius   = 4.0f;
    Color projectileColor    = {255, 220, 90, 255};
    int   projectileCount    = 1;
    float projectileSpread   = 0.15f;
};

struct AuraWeapon {
    float radius       = 90.0f;
    float damage       = 6.0f;
    float tickInterval = 0.5f;
    float timer        = 0.0f;
    Color color        = {255, 200, 100, 60};
};

struct OrbitWeapon {
    int   count            = 2;
    float radius           = 70.0f;
    float damage           = 8.0f;
    float angularSpeed     = 3.0f;
    float projectileRadius = 6.0f;
    float hitCooldown      = 0.4f;
    float currentAngle     = 0.0f;
    Color color            = {180, 120, 255, 255};
};

struct LightningWeapon {
    float cooldown     = 2.0f;
    float timer        = 0.0f;
    float range        = 480.0f;
    float damage       = 25.0f;
    int   targets      = 2;
    float chainRadius  = 220.0f;
    float boltLifetime = 0.15f;
    Color color        = {200, 240, 255, 255};
};

struct OrbitHitCooldown { float remaining = 0.0f; };

struct HitFlash {
    float remaining = 0.0f;
    float maxTime   = 0.08f;
};

struct ScreenShake {
    float remaining = 0.0f;
    float maxTime   = 0.15f;
    float intensity = 0.0f;   // пиковое смещение камеры в пикселях
};

// Визуальный эффект — не ECS-компонент, лежит в World::lightningBolts
struct LightningBolt {
    Vector2 from       = {};
    Vector2 to         = {};
    float   remaining  = 0.0f;
    float   maxLife    = 0.15f;
    Color   color      = WHITE;
};

struct XPOrb        { float value = 1.0f; };
struct XP           { int level = 1; float current = 0.0f; float needed = 5.0f; };
struct PickupRadius { float value = 120.0f; };

struct PlayerTag     {};
struct EnemyTag      {};
struct ProjectileTag {};
struct XPOrbTag      {};

struct Inactive {};
struct BossTag {};
struct RangedAttack {
    float cooldown         = 1.8f;
    float timer            = 0.0f;
    float keepDistance     = 380.0f;
    float minDistance      = 220.0f;
    float projectileSpeed  = 340.0f;
    float projectileDamage = 12.0f;
    float projectileLifetime = 3.0f;
    float projectileRadius = 7.0f;
    Color projectileColor  = {120, 200, 255, 255};
};

struct EnemyProjectileTag {};
struct EnemyProjectile {
    float damage = 12.0f;
};

struct MagnetOrbTag {};
struct MagnetOrb    { float pullDuration = 4.0f; };

struct HealOrbTag {};
struct HealOrb   { float amount = 15.0f; };

struct ChestTag {};
struct Chest    { int upgradesRemaining = 3; };

struct CoinTag {};
struct Coin    { int value = 1; };
}