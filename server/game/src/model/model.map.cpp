#include <fb/model/model.h>
#include <fb/game/server.h>
#include <random.h>
#include <unordered_map>
#include <shared_mutex>
#include <optional>

using namespace fb::model::enum_value;

std::optional<fb::model::point16_t> fb::model::map::spawn_position() const
{
    if (this->teleport.empty())
        return std::nullopt;

    auto& dsl = this->teleport.at(random<uint32_t>(0, this->teleport.size() - 1));
    switch (dsl.header)
    {
    case DSL::area:
    {
        auto params = fb::model::dsl::area(dsl.params);
        auto x      = random<uint16_t>(params.left, params.right);
        auto y      = random<uint16_t>(params.top, params.bottom);
        return fb::model::point16_t{x, y};
    }

    case DSL::point:
    {
        auto params = fb::model::dsl::point(dsl.params);
        return fb::model::point16_t{params.x, params.y};
    }

    default:
        return std::nullopt;
    }
}

fb::model::map* fb::model::__map::name2map(std::string_view name) const
{
    static const __map* cache_owner = nullptr;
    static auto         cache       = std::unordered_map<std::string, fb::model::map*>{};
    static auto         cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        if (cache_owner == this)
        {
            auto it = cache.find(name_str);
            if (it != cache.end())
                return it->second;
        }
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name)
        {
            auto lock = std::lock_guard(cache_mutex);
            if (cache_owner != this)
            {
                cache.clear();
                cache_owner = this;
            }
            cache[name_str] = &v;
            return &v;
        }
    }

    return nullptr;
}