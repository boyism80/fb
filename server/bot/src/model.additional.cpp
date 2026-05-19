#include <fb/model/model.h>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>
#include <shared_mutex>
#include <mutex>

const fb::model::promotion* fb::model::__promotion::operator() (fb::model::enum_value::CLASS cls,
                                                                uint8_t                      promotion) const
{
    try
    {
        auto& model = (*this)[cls][promotion];
        return &model;
    }
    catch (std::exception&)
    {
        return nullptr;
    }
}

bool fb::model::__promotion::name2class(std::string_view              name,
                                        fb::model::enum_value::CLASS& cls,
                                        uint8_t&                      promotion) const
{
    static auto cache       = std::unordered_map<std::string, std::pair<fb::model::enum_value::CLASS, uint8_t>>{};
    static auto cache_mutex = std::shared_mutex{};

    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(std::string(name));
        if (it != cache.end())
        {
            cls       = it->second.first;
            promotion = it->second.second;
            return true;
        }
    }

    auto name_str = std::string(name);
    for (const auto& [k1, v1] : *this)
    {
        for (const auto& [k2, v2] : v1)
        {
            if (v2.name == name_str)
            {
                cls       = k1;
                promotion = v2.step;
                {
                    auto lock       = std::lock_guard(cache_mutex);
                    cache[name_str] = std::make_pair(k1, v2.step);
                }
                return true;
            }
        }
    }

    return false;
}

bool fb::model::__promotion::class2name(fb::model::enum_value::CLASS cls, uint8_t promotion, std::string& name) const
{
    if (this->contains(cls) == false)
        return false;

    if ((*this)[cls].contains(promotion) == false)
        return false;

    name = (*this)[cls][promotion].name;
    return true;
}

fb::model::mob* fb::model::__mob::name2mob(std::string_view name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::mob*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name_str);
        if (it != cache.end())
            return it->second;
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name_str)
        {
            auto lock       = std::lock_guard(cache_mutex);
            cache[name_str] = &v;
            return &v;
        }
    }
    return nullptr;
}

fb::model::item* fb::model::__item::name2item(std::string_view name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::item*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name_str);
        if (it != cache.end())
            return it->second;
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name_str)
        {
            auto lock       = std::lock_guard(cache_mutex);
            cache[name_str] = &v;
            return &v;
        }
    }

    return nullptr;
}

std::vector<fb::model::item*> fb::model::__item::name2item_prefix(std::string_view prefix) const
{
    static auto sorted_items = std::map<std::string, fb::model::item*>{};
    static auto once_flag    = std::once_flag{};
    static auto read_mutex   = std::shared_mutex{};

    std::call_once(once_flag, [this]() {
        auto lock = std::lock_guard(read_mutex);
        for (auto& [k, v] : *this)
        {
            sorted_items[v.name] = &v;
        }
    });

    auto result = std::vector<fb::model::item*>{};

    {
        auto lock = std::shared_lock(read_mutex);
        if (prefix.empty())
        {
            result.reserve(sorted_items.size());
            for (auto& [name, item] : sorted_items)
            {
                result.push_back(item);
            }
            return result;
        }

        auto prefix_str = std::string(prefix);
        auto it         = sorted_items.lower_bound(prefix_str);
        while (it != sorted_items.end())
        {
            if (it->first.size() < prefix_str.size())
            {
                ++it;
                continue;
            }

            if (!it->first.starts_with(prefix_str))
                break;

            result.push_back(it->second);
            ++it;
        }
    }

    return result;
}

fb::model::npc* fb::model::__npc::name2npc(std::string_view name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::npc*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name_str);
        if (it != cache.end())
            return it->second;
    }

    for (auto& npc : *this)
    {
        if (npc.second.name == name_str)
        {
            auto lock       = std::lock_guard(cache_mutex);
            cache[name_str] = &npc.second;
            return &npc.second;
        }
    }

    return nullptr;
}

fb::model::map* fb::model::__map::name2map(std::string_view name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::map*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name_str);
        if (it != cache.end())
            return it->second;
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name_str)
        {
            auto lock       = std::lock_guard(cache_mutex);
            cache[name_str] = &v;
            return &v;
        }
    }

    return nullptr;
}

fb::model::spell* fb::model::__spell::name2spell(std::string_view name) const
{
    static auto cache       = std::unordered_map<std::string, fb::model::spell*>{};
    static auto cache_mutex = std::shared_mutex{};

    auto name_str = std::string(name);
    {
        auto lock = std::shared_lock(cache_mutex);
        auto it   = cache.find(name_str);
        if (it != cache.end())
            return it->second;
    }

    for (auto& [k, v] : *this)
    {
        if (v.name == name_str)
        {
            auto lock       = std::lock_guard(cache_mutex);
            cache[name_str] = &v;
            return &v;
        }
    }
    return nullptr;
}
