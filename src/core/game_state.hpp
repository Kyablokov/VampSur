#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace vk {

inline uint64_t xorshift64(uint64_t& s) {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return s;
}

inline float rand01(uint64_t& s) {
    return static_cast<float>(xorshift64(s) >> 11) * (1.0f / 9007199254740992.0f);
}

inline float randRange(uint64_t& s, float lo, float hi) {
    return lo + (hi - lo) * rand01(s);
}

inline int randInt(uint64_t& s, int lo, int hi) {
    if (hi <= lo) return lo;
    return lo + static_cast<int>(xorshift64(s) % static_cast<uint64_t>(hi - lo));
}

enum class GameMode {
    Playing,
    Upgrading,
    Paused,
    GameOver,
};

struct RunStats {
    int   kills       = 0;
    float damageTaken = 0.0f;
};

struct GameState {
    float    timeSeconds = 0.0f;
    float    spawnTimer  = 0.0f;
    GameMode mode        = GameMode::Playing;
    uint64_t rngState    = 0x9E3779B97F4A7C15ull;

    RunStats stats;

    // Флаг: результат текущего прогона уже записан в save.json
    bool runSaved = false;

    std::array<int, 3> upgradeOffer = { -1, -1, -1 };
    std::unordered_map<std::string, int> takenUpgrades;
};

}