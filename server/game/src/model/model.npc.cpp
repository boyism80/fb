#include <fb/model/model.h>
#include <fb/game/server.h>
#include <unordered_map>
#include <shared_mutex>

fb::model::npc* fb::model::___npc::name2npc(std::string_view name) const
{
    static const ___npc* cache_owner = nullptr;
    static auto          cache       = std::unordered_map<std::string, fb::model::npc*>{};
    static auto          cache_mutex = std::shared_mutex{};

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

    for (auto& npc : *this)
    {
        if (npc.second.name == name)
        {
            auto lock = std::lock_guard(cache_mutex);
            if (cache_owner != this)
            {
                cache.clear();
                cache_owner = this;
            }
            cache[name_str] = &npc.second;
            return &npc.second;
        }
    }

    return nullptr;
}
