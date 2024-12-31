#ifndef __SHARED_H__
#define __SHARED_H__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <fb/game/group.h>
#include <fb/game/clan.h>
#include <fb/locker.h>

namespace fb::game {

class shard_params
{
public:
    using group_container     = std::unordered_map<uint32_t, shared_group_lock>;
    using clan_container      = std::unordered_map<uint32_t, shared_clan_lock>;
    using character_container = std::unordered_map<std::string, fb::game::character*>;

public:
    fb::locker<group_container>     groups;
    fb::locker<clan_container>      clans;
    fb::locker<character_container> characters;

public:
    shard_params();
    shard_params(const shard_params&) = delete;
    shard_params(shard_params&&)      = default;
    ~shard_params()                   = default;
};

class shard
{
public:
    inline static const uint32_t SHARD_MODULAR_SIZE = 12;

private:
    std::unordered_map<uint32_t, std::unique_ptr<shard_params>> _data;

public:
    shard();
    shard(const shard&) = delete;
    shard(shard&&)      = default;
    ~shard();

public:
    uint32_t mod(uint32_t hash) const;
    uint32_t mod(const std::string& val) const;

public:
    shard_params* operator[] (uint32_t hash) const;
    shard_params* operator[] (const std::string& val) const;
};

} // namespace fb::game

#endif