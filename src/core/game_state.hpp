#pragma once

#include <cstdint>

namespace vk {

inline uint64_t xorshift64(uint64_t& s) {
    s ^= s << 13;
    s ^= s >> 7;
    s ^= s << 17;
    return s;
}

inline float rand01(uint64_t& s) {
    // 2^53 = 9007199254740992.0f — берём старшие 53 бита для равномерного [0,1)
    return static_cast<float>(xorshift64(s) >> 11) * (1.0f / 9007199254740992.0f);
}

inline float randRange(uint64_t& s, float lo, float hi) {
    return lo + (hi - lo) * rand01(s);
}

struct GameState {
    float    timeSeconds = 0.0f;
    float    spawnTimer  = 0.0f;
    bool     gameOver    = false;
    uint64_t rngState    = 0x9E3779B97F4A7C15ull;
};

}