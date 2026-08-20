#include <fb/game/collection.h>
#include <fb/game/character.h>
#include <fb/game/server.h>

namespace fb::game {

character* character_collections::owner_character() const
{
    if (auto owner = this->_owner.lock())
        return owner.get();
    return nullptr;
}

void character_collections::owner(std::weak_ptr<character> owner)
{
    this->_owner = std::move(owner);
}

bool character_collections::contains(uint32_t mob_id) const
{
    return this->_unlocked_mob_ids.contains(mob_id);
}

bool character_collections::unlock(uint32_t mob_id)
{
    return this->_unlocked_mob_ids.insert(mob_id).second;
}

bool character_collections::remove(uint32_t mob_id)
{
    return this->_unlocked_mob_ids.erase(mob_id) > 0;
}

void character_collections::load(const std::vector<fb::protocol::internal::CollectionUnlock>& rows)
{
    this->_unlocked_mob_ids.clear();
    for (const auto& row : rows)
        this->_unlocked_mob_ids.insert(row.mob_id);
}

void character_collections::sync()
{
    auto* ch = this->owner_character();
    if (ch == nullptr)
        return;

    if (ch->client_version != fb::protocol::CLIENT_VERSION::v651)
        return;

    // LIST (type 0) forces sheet_select_tab(5); push flags only.
    for (auto mob_id : this->_unlocked_mob_ids)
    {
        auto* mob = ch->server.collection_mob(mob_id);
        if (mob == nullptr)
            continue;

        auto* item = ch->server.meta.find_item(mob->name);
        if (item == nullptr)
            continue;

        ch->listener.on_collection_flag(*ch, item->group_id, item->slot, true);
    }
}

std::vector<fb::protocol::internal::CollectionUnlock> character_collections::to_protocol(uint32_t user) const
{
    auto rows = std::vector<fb::protocol::internal::CollectionUnlock>{};
    rows.reserve(this->_unlocked_mob_ids.size());
    for (auto mob_id : this->_unlocked_mob_ids)
        rows.emplace_back(user, mob_id);
    return rows;
}

async::task<void> character_collections::try_unlock(uint32_t mob_id)
{
    auto* ch = this->owner_character();
    if (ch == nullptr)
        co_return;

    if (ch->client_version != fb::protocol::CLIENT_VERSION::v651)
        co_return;

    if (ch->server.collection_mob(mob_id) == nullptr)
        co_return;

    if (this->contains(mob_id))
        co_return;

    if (this->unlock(mob_id) == false)
        co_return;

    auto* mob = ch->server.collection_mob(mob_id);
    if (mob == nullptr)
        co_return;

    auto* item = ch->server.meta.find_item(mob->name);
    if (item == nullptr)
        co_return;

    ch->listener.on_collection_flag(*ch, item->group_id, item->slot, true);
    co_return;
}

async::task<void> character_collections::set(uint8_t group_id, uint8_t slot, bool onoff)
{
    auto* ch = this->owner_character();
    if (ch == nullptr)
        co_return;

    auto* item = ch->server.meta.item(group_id, slot);
    if (item == nullptr)
        co_return;

    auto* mob = ch->server.collection_mob(*item);
    if (mob == nullptr)
        co_return;

    if (onoff)
    {
        if (this->contains(mob->id))
            co_return;
        this->unlock(mob->id);
    }
    else
    {
        if (this->contains(mob->id) == false)
            co_return;
        this->remove(mob->id);
    }

    if (ch->client_version != fb::protocol::CLIENT_VERSION::v651)
        co_return;

    ch->listener.on_collection_flag(*ch, group_id, slot, onoff);
    co_return;
}

uint8_t character_collections::unlocked_count(uint8_t group_id) const
{
    auto* ch = this->owner_character();
    if (ch == nullptr)
        return 0;

    auto count = uint8_t{0};
    for (auto mob_id : this->_unlocked_mob_ids)
    {
        auto* mob = ch->server.collection_mob(mob_id);
        if (mob == nullptr)
            continue;

        auto* item = ch->server.meta.find_item(mob->name);
        if (item == nullptr || item->group_id != group_id)
            continue;

        ++count;
    }
    return count;
}

std::vector<uint8_t> character_collections::bitmask(uint8_t group_id) const
{
    auto* ch = this->owner_character();
    if (ch == nullptr)
        return {};

    auto* group = ch->server.meta.group(group_id);
    if (group == nullptr)
        return {};

    auto bytes = std::vector<uint8_t>((group->items.size() + 7) / 8, 0);
    for (const auto& item : group->items)
    {
        if (item.name.empty())
            continue;

        auto* mob = ch->server.collection_mob(item);
        if (mob == nullptr || this->contains(mob->id) == false)
            continue;

        auto slot = static_cast<size_t>(item.slot);
        if (slot >= group->items.size())
            continue;

        bytes[slot / 8] |= static_cast<uint8_t>(1u << (slot % 8));
    }
    return bytes;
}

} // namespace fb::game
