#pragma once

#include <cstddef>
#include <utility>
#include <vector>
#include <entt/entt.hpp>

#include "components/components.hpp"

namespace vk {

template <typename Tag>
class EntityPool {
public:
    template <typename PrewarmFn>
    EntityPool(entt::registry& registry, std::size_t capacity, PrewarmFn&& prewarm)
        : registry_(registry), capacity_(capacity)
    {
        freeList_.reserve(capacity);
        for (std::size_t i = 0; i < capacity; ++i) {
            const auto e = registry_.create();
            std::forward<PrewarmFn>(prewarm)(registry_, e);
            registry_.emplace<Tag>(e);
            registry_.emplace<Inactive>(e);
            freeList_.push_back(e);
        }
    }

    entt::entity acquire() {
        if (freeList_.empty()) return entt::null;
        const auto e = freeList_.back();
        freeList_.pop_back();
        registry_.remove<Inactive>(e);
        ++active_;
        return e;
    }

    void release(entt::entity e) {
        if (!registry_.valid(e)) return;
        if (registry_.all_of<Inactive>(e)) return;
        registry_.emplace<Inactive>(e);
        freeList_.push_back(e);
        --active_;
    }

    std::size_t active()   const { return active_; }
    std::size_t capacity() const { return capacity_; }

private:
    entt::registry& registry_;
    std::size_t     capacity_;
    std::size_t     active_ = 0;
    std::vector<entt::entity> freeList_;
};

} // namespace vk