#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/logger.h>

using table = fb::model::table;

namespace fb::game {

void server::init_collection_mobs()
{
    this->_collection_mobs.clear();
    for (const auto& group : this->meta.groups())
    {
        for (const auto& item : group.items)
        {
            if (item.name.empty())
                continue;

            auto* mob = table::mob->name2mob(item.name);
            if (mob == nullptr)
            {
                fb::logger::warn("collection mob skipped (not in mob.json): {}", item.name);
                continue;
            }

            auto [it, inserted] = this->_collection_mobs.emplace(mob->id, mob);
            if (inserted == false)
                fb::logger::warn("duplicate collection mob id {} ({})", mob->id, item.name);
        }
    }
}

const std::unordered_map<uint32_t, fb::model::mob*>& server::collection_mobs() const
{
    return this->_collection_mobs;
}

fb::model::mob* server::collection_mob(uint32_t mob_id) const
{
    auto it = this->_collection_mobs.find(mob_id);
    if (it == this->_collection_mobs.end())
        return nullptr;
    return it->second;
}

fb::model::mob* server::collection_mob(const fb::meta_dat_collection_item& item) const
{
    auto* mob = table::mob->name2mob(item.name);
    if (mob == nullptr)
        return nullptr;

    return this->collection_mob(mob->id);
}

} // namespace fb::game
