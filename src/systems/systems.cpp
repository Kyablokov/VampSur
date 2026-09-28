#include "systems/systems.hpp"

#include "components/components.hpp"

namespace vk {

void updateInput(entt::registry& registry, float /*dt*/) {
    float dx = 0.0f;
    float dy = 0.0f;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dy -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dy += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dx -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1.0f;

    // Нормализация диагонали, чтобы игрок не двигался быстрее по диагонали
    if (dx != 0.0f && dy != 0.0f) {
        constexpr float kInvSqrt2 = 0.70710678f;
        dx *= kInvSqrt2;
        dy *= kInvSqrt2;
    }

    auto view = registry.view<PlayerTag, Velocity, Speed>();
    for (auto [entity, vel, speed] : view.each()) {
        vel.x = dx * speed.value;
        vel.y = dy * speed.value;
    }
}

void updateMovement(entt::registry& registry, float dt) {
    auto view = registry.view<Position, Velocity>();
    for (auto [entity, pos, vel] : view.each()) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    }
}

void renderCircles(entt::registry& registry) {
    auto view = registry.view<Position, RenderCircle>();
    for (auto [entity, pos, rc] : view.each()) {
        DrawCircleV({ pos.x, pos.y }, rc.radius, rc.color);
    }
}


}