#include <fb/model/model.h>

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

fb::model::spell* fb::model::___spell::name2spell(std::string_view name) const
{
    static auto cache_owner = std::weak_ptr<const ___spell>{};
    static auto cache       = std::unordered_map<std::string, fb::model::spell*>{};
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
            auto published = fb::model::table::spell.get();
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
