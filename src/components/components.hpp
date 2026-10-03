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

struct XPOrb        { float value = 1.0f; };
struct XP           { int level = 1; float current = 0.0f; float needed = 5.0f; };
struct PickupRadius { float value = 120.0f; };

struct PlayerTag     {};
struct EnemyTag      {};
struct ProjectileTag {};
struct XPOrbTag      {};

struct Inactive {};

}