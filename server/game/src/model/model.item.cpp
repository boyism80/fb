#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/model/model.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <shared_mutex>
#include <mutex>

using namespace fb::model::enum_value;

ITEM_ATTRIBUTE fb::model::item::attr() const
{
    auto attr = ITEM_ATTRIBUTE::NONE;
    if (this->capacity > 1)
        attr = ITEM_ATTRIBUTE((uint32_t)attr | (uint32_t)ITEM_ATTRIBUTE::BUNDLE);
    return attr;
}

bool fb::model::item::attr(ITEM_ATTRIBUTE flag) const
{
    return ((uint32_t)this->attr() & (uint32_t)flag) == (uint32_t)flag;
}

std::optional<fb::model::datetime> fb::model::item::expire_time(const fb::model::datetime& now) const
{
    if (this->duration.has_value() == false)
        return std::nullopt;

    return now + this->duration.value();
}

fb::model::item* fb::model::___item::name2item(std::string_view name) const
{
    static const ___item* cache_owner = nullptr;
    static auto           cache       = std::unordered_map<std::string, fb::model::item*>{};
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

std::vector<fb::model::item*> fb::model::___item::name2item_prefix(std::string_view prefix) const
{
    static const ___item* cache_owner  = nullptr;
    static auto           sorted_items = std::map<std::string, fb::model::item*>{};
    static auto           cache_mutex  = std::shared_mutex{};

    {
        auto lock = std::unique_lock(cache_mutex);
        if (cache_owner != this)
        {
            sorted_items.clear();
            for (auto& [k, v] : *this)
                sorted_items[v.name] = &v;
            cache_owner = this;
        }
    }

    auto result     = std::vector<fb::model::item*>{};
    auto prefix_str = std::string(prefix);

    {
        auto lock = std::shared_lock(cache_mutex);
        if (prefix.empty())
        {
            for (auto& [name, item] : sorted_items)
                result.push_back(item);
            return result;
        }

        auto it = sorted_items.lower_bound(prefix_str);
        while (it != sorted_items.end())
        {
            if (it->first.size() < prefix.size())
            {
                ++it;
                continue;
            }

            if (it->first.substr(0, prefix.size()) != prefix)
                break;

            result.push_back(it->second);
            ++it;
        }
    }

    return result;
}

std::shared_ptr<fb::game::item> fb::model::item::make(fb::game::server&                  server,
                                                      uint16_t                           count,
                                                      std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::item>(*this,
                                       fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::cash::make(fb::game::server&                  server,
                                                      uint16_t                           count,
                                                      std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::cash>(count);
}

std::shared_ptr<fb::game::item> fb::model::consume::make(fb::game::server&                  server,
                                                         uint16_t                           count,
                                                         std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::consume>(*this,
                                          fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::pack::make(fb::game::server&                  server,
                                                      uint16_t                           count,
                                                      std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::pack>(*this,
                                       fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::weapon::make(fb::game::server&                  server,
                                                        uint16_t                           count,
                                                        std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::weapon>(*this,
                                         fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::armor::make(fb::game::server&                  server,
                                                       uint16_t                           count,
                                                       std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::armor>(*this,
                                        fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::helmet::make(fb::game::server&                  server,
                                                        uint16_t                           count,
                                                        std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::helmet>(*this,
                                         fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::shield::make(fb::game::server&                  server,
                                                        uint16_t                           count,
                                                        std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::shield>(*this,
                                         fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::ring::make(fb::game::server&                  server,
                                                      uint16_t                           count,
                                                      std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::ring>(*this,
                                       fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}

std::shared_ptr<fb::game::item> fb::model::auxiliary::make(fb::game::server&                  server,
                                                           uint16_t                           count,
                                                           std::optional<fb::model::datetime> expire_time) const
{
    return server.make<fb::game::auxiliary>(*this,
                                            fb::game::item::initial_params{.count = count, .expire_time = expire_time});
}