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

// Автоматическое оружие игрока
struct Weapon {
    float cooldown          = 0.6f;
    float timer             = 0.0f;
    float range             = 550.0f;
    float projectileSpeed   = 620.0f;
    float projectileDamage  = 10.0f;
    float projectileLifetime= 1.5f;
    float projectileRadius  = 4.0f;
    Color projectileColor   = {255, 220, 90, 255};
};

struct PlayerTag     {};
struct EnemyTag      {};
struct ProjectileTag {};

// Маркер «объект в пуле, не активен».
// Все системы обязаны исключать Inactive через entt::exclude<Inactive>.
struct Inactive {};

}