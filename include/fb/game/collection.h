#ifndef FB_GAME_COLLECTION_H
#define FB_GAME_COLLECTION_H

#include <async/task.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace fb::game {

class character;

class character_collections
{
private:
    std::weak_ptr<character>     _owner;
    std::unordered_set<uint32_t> _unlocked_mob_ids;

    character* owner_character() const;

public:
    // clang-format off
    void                                                  owner(std::weak_ptr<character> owner);
    bool                                                  contains(uint32_t mob_id) const;
    bool                                                  unlock(uint32_t mob_id);
    bool                                                  remove(uint32_t mob_id);
    void                                                  load(const std::vector<fb::protocol::internal::CollectionUnlock>& rows);
    void                                                  sync();
    std::vector<fb::protocol::internal::CollectionUnlock> to_protocol(uint32_t user) const;
    async::task<void>                                     try_unlock(uint32_t mob_id);
    async::task<void>                                     set(uint8_t group_id, uint8_t slot, bool onoff);
    async::task<uint32_t>                                 unlock_all();
    uint8_t                                               unlocked_count(uint8_t group_id) const;
    std::vector<uint8_t>                                  bitmask(uint8_t group_id) const;
    // clang-format on
};

} // namespace fb::game

#endif
