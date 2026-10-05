#pragma once

#include <entt/entt.hpp>
#include <raylib.h>

#include "core/world.hpp"

namespace vk {

void updateInput      (World& w, float dt);
void updateMovement   (World& w, float dt);

void spawnEnemies     (World& w, float dt);
void chasePlayer      (World& w, float dt);
void rebuildSpatial   (World& w);

void updateWeapons    (World& w, float dt);
void updateProjectiles(World& w, float dt);
void resolveProjectileHits(World& w);
void resolveContactDamage (World& w, float dt);

void updateAura       (World& w, float dt);
void updateOrbit      (World& w, float dt);
void resolveOrbitHits (World& w, float dt);
void updateLightning  (World& w, float dt);

void updateXPMagnet   (World& w, float dt);
void resolveXPPickup  (World& w);
void checkLevelUp     (World& w);

void renderAura       (World& w);
void renderOrbit      (World& w);
void renderLightning  (World& w);
void renderCircles    (entt::registry& r);

void spawnBosses    (World& w, float dt);
void renderBossHP   (World& w);

void updateMagnets   (World& w, float dt);
void resolveMagnetPickup(World& w);
void renderMagnets   (World& w);

void updateHitFlashes  (World& w, float dt);
void updateScreenShake (World& w, float dt);

void addShake (World& w, float intensity, float duration);
void applyHit(World& w, entt::entity enemy);

// Единая точка обработки смерти врага: дроп XP, магнита, возврат в пул.
// Все системы, которые наносят урон, должны вызывать это.
void killEnemy(World& w, entt::entity e);
}