#include <fb/model/model.h>
#include <unordered_map>
#include <shared_mutex>

fb::model::spell* fb::model::__spell::name2spell(std::string_view name) const
{
    static const __spell* cache_owner = nullptr;
    static auto           cache       = std::unordered_map<std::string, fb::model::spell*>{};
    static auto           cache_mutex = std::shared_mutex{};

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
