#include <fb/game/server.h>
#include <fb/model/model.h>

#include <cstdint>

using table = fb::model::table;

namespace fb::game {

// Looked up from the current mob table on every call so a table reload is picked up without a rebuild.
const fb::model::mob* server::collection_mob(uint32_t mob_id) const
{
    auto* mob = table::mob->find(mob_id);
    if (mob == nullptr)
        return nullptr;

    if (this->meta.find_item(mob->name) == nullptr)
        return nullptr;

    return mob;
}

const fb::model::mob* server::collection_mob(const fb::meta_dat_collection_item& item) const
{
    return table::mob->name2mob(item.name);
}

} // namespace fb::game
