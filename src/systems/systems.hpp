#pragma once

#include <entt/entt.hpp>
#include <raylib.h>

namespace vk {

// Управление: WASD/стрелки → Velocity игрока
void updateInput(entt::registry& registry, float dt);

// Движение: Position += Velocity * dt
void updateMovement(entt::registry& registry, float dt);

// Рендер: все сущности с Position + RenderCircle
void renderCircles(entt::registry& registry);

// Декоративная сетка вокруг начала координат

}