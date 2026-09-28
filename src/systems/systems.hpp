#pragma once

#include <entt/entt.hpp>
#include <raylib.h>

#include "core/config.hpp"
#include "core/game_state.hpp"

namespace vk {

void updateInput  (entt::registry& registry, float dt);
void updateMovement(entt::registry& registry, float dt);

void spawnEnemies (entt::registry& registry, GameState& state,
                   const GameConfig& config, float dt);

void chasePlayer  (entt::registry& registry, float dt);

void resolveCombat(entt::registry& registry, GameState& state,
                   const GameConfig& config, float dt);

void renderCircles(entt::registry& registry);

}