#pragma once

#include <entt/entt.hpp>

#include "components/components.hpp"
#include "core/config.hpp"

namespace vk {

inline entt::entity createPlayer(entt::registry& registry, const PlayerConfig& cfg) {
    const auto e = registry.create();
    registry.emplace<Position>    (e, cfg.startX, cfg.startY);
    registry.emplace<Velocity>    (e, 0.0f, 0.0f);
    registry.emplace<Speed>       (e, cfg.speed);
    registry.emplace<RenderCircle>(e, cfg.radius, cfg.color);
    registry.emplace<PlayerTag>   (e);
    return e;
}

}