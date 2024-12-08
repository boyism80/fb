#ifndef __THREAD_PARAMS_H__
#define __THREAD_PARAMS_H__

#include <group.h>
#include <map.h>
#include <unordered_map>

namespace fb::game {

class thread_params
{
public:
    using group_container = std::unordered_map<uint32_t, std::unique_ptr<group>>;
    using map_container   = std::unordered_map<uint32_t, fb::game::map*>;

public:
    group_container groups;
    map_container   maps;
};

} // namespace fb::game

#endif