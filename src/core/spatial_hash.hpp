#pragma once

#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <entt/entt.hpp>

namespace vk {

// Uniform grid для быстрого поиска соседей.
// Требует пересборки каждый кадр (или через кадр) — дёшево при сотнях сущностей.
class SpatialHash {
public:
    explicit SpatialHash(float cellSize = 64.0f) : cellSize_(cellSize) {}

    void clear() {
        // Не удаляем сами векторы — переиспользуем их capacity между кадрами.
        for (auto& [_, v] : cells_) v.clear();
    }

    void insert(entt::entity e, float x, float y) {
        cells_[key(cellX(x), cellY(y))].push_back(e);
    }

    // Возвращает все сущности в клетках, пересекающих AABB [x-r, x+r] × [y-r, y+r].
    // Точная проверка расстояния — на стороне вызывающего.
    void query(float x, float y, float radius, std::vector<entt::entity>& out) const {
        const int minCX = cellX(x - radius);
        const int maxCX = cellX(x + radius);
        const int minCY = cellY(y - radius);
        const int maxCY = cellY(y + radius);
        for (int cx = minCX; cx <= maxCX; ++cx) {
            for (int cy = minCY; cy <= maxCY; ++cy) {
                auto it = cells_.find(key(cx, cy));
                if (it != cells_.end()) {
                    out.insert(out.end(), it->second.begin(), it->second.end());
                }
            }
        }
    }

private:
    int cellX(float x) const { return static_cast<int>(std::floor(x / cellSize_)); }
    int cellY(float y) const { return static_cast<int>(std::floor(y / cellSize_)); }

    static int64_t key(int cx, int cy) {
        // Упаковка двух int32 в int64. Подходит для |координат| < ~2^31 клеток.
        return (static_cast<int64_t>(cx) << 32) ^ (static_cast<uint32_t>(cy));
    }

    float cellSize_;
    std::unordered_map<int64_t, std::vector<entt::entity>> cells_;
};

} // namespace vk