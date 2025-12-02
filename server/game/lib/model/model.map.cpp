#include <fb/model/model.h>
#include <fb/game/server.h>
#include <unordered_map>
#include <shared_mutex>

fb::model::map* fb::model::__map::name2map(const std::string& name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::map*>{};
    static auto cache_mutex = std::shared_mutex{};

    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name);
        if (it != cache.end())
            return it->second;
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name)
        {
            auto lock   = std::lock_guard(cache_mutex);
            cache[name] = &v;
            return &v;
        }
    }

    return nullptr;
}