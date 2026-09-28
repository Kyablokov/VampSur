#pragma once

#include <raylib.h>

namespace vk {

struct Position     { float x = 0.0f, y = 0.0f; };
struct Velocity     { float x = 0.0f, y = 0.0f; };
struct Speed        { float value = 0.0f; };
struct RenderCircle { float radius = 10.0f; Color color = WHITE; };

struct PlayerTag {};   // маркер: это игрок
struct EnemyTag  {};   // пригодится на шаге 2

}