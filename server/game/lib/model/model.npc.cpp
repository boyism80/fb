#include <fb/model/model.h>
#include <fb/game/server.h>
#include <unordered_map>
#include <shared_mutex>

fb::model::npc* fb::model::__npc::name2npc(const std::string& name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::npc*>{};
    static auto cache_mutex = std::shared_mutex{};

    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name);
        if (it != cache.end())
            return it->second;
    }

    for (auto& npc : *this)
    {
        if (npc.second.name == name)
        {
            auto lock = std::lock_guard(cache_mutex);
            cache[name] = &npc.second;
            return &npc.second;
        }
    }

    return nullptr;
}