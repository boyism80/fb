#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/server.h>
#include <fb/model/model.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

fb::model::mob* fb::model::___mob::name2mob(std::string_view name) const
{
    static auto cache_owner = std::weak_ptr<const ___mob>{};
    static auto cache       = std::unordered_map<std::string, fb::model::mob*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        if (cache_owner.lock().get() == this)
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
            auto published = fb::model::table::mob.get();
            if (published.get() == this)
            {
                auto lock = std::lock_guard(cache_mutex);
                if (cache_owner.lock() != published)
                {
                    cache.clear();
                    cache_owner = published;
                }
                cache[name_str] = &v;
            }
            return &v;
        }
    }
    return nullptr;
}
