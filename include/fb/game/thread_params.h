#ifndef __THREAD_PARAMS_H__
#define __THREAD_PARAMS_H__

#include <fb/game/group.h>
#include <fb/game/map.h>
#include <fb/game/character.h>
#include <unordered_map>
#include <async/task.h>

namespace fb::game {

class thread_params
{
public:
    using group_container     = std::unordered_map<uint32_t, std::unique_ptr<group>>;
    using map_container       = std::unordered_map<uint32_t, map*>;
    using rezen_container     = std::vector<rezen>;
    using character_container = std::unordered_map<uint32_t, character*>;

public:
    group_container     groups;
    map_container       maps;
    rezen_container     rezens;
    character_container characters;
};

} // namespace fb::game

#endif