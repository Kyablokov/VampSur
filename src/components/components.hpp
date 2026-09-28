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

struct PlayerTag {};
struct EnemyTag  {};

}