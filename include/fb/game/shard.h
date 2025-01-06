#ifndef __SHARED_H__
#define __SHARED_H__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <fb/game/group.h>
#include <fb/game/clan.h>
#include <fb/locker.h>
#include <shared_mutex>

namespace fb::game {

class shard_params
{
public:
    using group_container          = std::unordered_map<uint32_t, shared_group_lock>;
    using clan_container           = std::unordered_map<uint32_t, shared_clan_lock>;
    using character_name_container = std::unordered_map<std::string, fb::game::character*>;
    using character_id_container   = std::unordered_map<uint32_t, fb::game::character*>;

public:
    fb::locker<group_container>          groups;
    fb::locker<clan_container>           clans;
    fb::locker<character_id_container>   ids;
    fb::locker<character_name_container> names;

public:
    shard_params() = default;
    shard_params(const shard_params&) = delete;
    shard_params(shard_params&&)      = default;
    ~shard_params()                   = default;
};

} // namespace fb::game

#endif