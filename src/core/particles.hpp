#pragma once

#include <cstddef>
#include <vector>
#include <raylib.h>

#include "core/game_state.hpp"   // randRange

namespace vk {

inline Color lerpColor(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t),
        static_cast<unsigned char>(a.a + (b.a - a.a) * t),
    };
}

struct Particle {
    Vector2 position    = {};
    Vector2 velocity    = {};
    float   lifetime    = 1.0f;
    float   maxLifetime = 1.0f;
    float   sizeStart   = 3.0f;
    float   sizeEnd     = 0.0f;
    Color   colorStart  = WHITE;
    Color   colorEnd    = {255, 255, 255, 0};
    float   drag        = 6.0f;
};

class ParticleSystem {
public:
    void reserve(std::size_t capacity) { pool_.resize(capacity); }
    void clear() { active_ = 0; }

    void spawn(const Particle& p) {
        if (active_ >= pool_.size()) return;
        pool_[active_++] = p;
    }

    void spawnBurst(Vector2 pos, int count,
                    float speedMin, float speedMax,
                    float sizeStart, float sizeEnd,
                    Color colorStart, Color colorEnd,
                    float lifetime, float drag,
                    uint64_t& rng) {
        for (int i = 0; i < count; ++i) {
            const float angle = randRange(rng, 0.0f, 6.2831853f);
            const float speed = randRange(rng, speedMin, speedMax);
            Particle p;
            p.position    = pos;
            p.velocity    = { std::cos(angle) * speed, std::sin(angle) * speed };
            p.lifetime    = lifetime;
            p.maxLifetime = lifetime;
            p.sizeStart   = sizeStart;
            p.sizeEnd     = sizeEnd;
            p.colorStart  = colorStart;
            p.colorEnd    = colorEnd;
            p.drag        = drag;
            spawn(p);
        }
    }

    void update(float dt) {
        for (std::size_t i = 0; i < active_; ) {
            Particle& p = pool_[i];
            p.lifetime -= dt;
            if (p.lifetime <= 0.0f) {
                pool_[i] = pool_[--active_];   // swap-remove
                continue;
            }
            const float damp = 1.0f / (1.0f + p.drag * dt);
            p.velocity.x *= damp;
            p.velocity.y *= damp;
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;
            ++i;
        }
    }

    void render() const {
        for (std::size_t i = 0; i < active_; ++i) {
            const Particle& p = pool_[i];
            const float t = (p.maxLifetime > 0.0f) ? (1.0f - p.lifetime / p.maxLifetime) : 1.0f;
            const float size = p.sizeStart + (p.sizeEnd - p.sizeStart) * t;
            if (size <= 0.0f) continue;
            DrawCircleV(p.position, size, lerpColor(p.colorStart, p.colorEnd, t));
        }
    }

    std::size_t activeCount() const { return active_; }
    std::size_t capacity()    const { return pool_.size(); }

private:
    std::vector<Particle> pool_;
    std::size_t active_ = 0;
};

// ---------------------------------------------------------------- damage numbers

struct DamageNumber {
    Vector2 position    = {};
    Vector2 velocity    = {};
    float   remaining   = 1.0f;
    float   maxLifetime = 0.7f;
    float   value       = 0.0f;
    Color   color       = WHITE;
};

class DamageNumberSystem {
public:
    void reserve(std::size_t capacity) { pool_.resize(capacity); }
    void clear() { active_ = 0; }

    void spawn(Vector2 pos, float value, Color color, float lifetime) {
        if (active_ >= pool_.size()) return;
        DamageNumber d;
        d.position    = pos;
        d.velocity    = { 0.0f, -60.0f };
        d.remaining   = lifetime;
        d.maxLifetime = lifetime;
        d.value       = value;
        d.color       = color;
        pool_[active_++] = d;
    }

    void update(float dt) {
        for (std::size_t i = 0; i < active_; ) {
            DamageNumber& d = pool_[i];
            d.remaining -= dt;
            if (d.remaining <= 0.0f) {
                pool_[i] = pool_[--active_];
                continue;
            }
            d.velocity.y *= 1.0f / (1.0f + 3.0f * dt);
            d.position.x += d.velocity.x * dt;
            d.position.y += d.velocity.y * dt;
            ++i;
        }
    }

    void render() const {
        for (std::size_t i = 0; i < active_; ++i) {
            const DamageNumber& d = pool_[i];
            const float t = (d.maxLifetime > 0.0f) ? (d.remaining / d.maxLifetime) : 0.0f;
            const unsigned char alpha = static_cast<unsigned char>(255.0f * t);
            const Color c = { d.color.r, d.color.g, d.color.b, alpha };
            const int size = (d.value >= 20.0f) ? 22 : 18;
            const char* text = TextFormat("%.0f", d.value);
            const int w = MeasureText(text, size);
            DrawText(text,
                     static_cast<int>(d.position.x) - w / 2,
                     static_cast<int>(d.position.y),
                     size, c);
        }
    }

    std::size_t activeCount() const { return active_; }
    std::size_t capacity()    const { return pool_.size(); }

private:
    std::vector<DamageNumber> pool_;
    std::size_t active_ = 0;
};

} // namespace vk