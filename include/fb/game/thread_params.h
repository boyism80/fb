#ifndef __THREAD_PARAMS_H__
#define __THREAD_PARAMS_H__

#include <fb/game/group.h>
#include <fb/game/map.h>
#include <fb/game/character.h>
#include <unordered_map>
#include <async/task.h>

namespace fb::game {

class server;

class thread_params
{
public:
    using group_container     = std::unordered_map<uint32_t, std::unique_ptr<group>>;
    using map_container       = std::unordered_map<uint32_t, std::shared_ptr<map>>;
    using rezen_container     = std::vector<std::unique_ptr<rezen>>;
    using character_container = character::container;
    using map_snapshot        = std::vector<std::shared_ptr<map>>;

public:
    group_container                     groups;
    map_container                       maps;
    rezen_container                     rezens;
    character_container                 characters;
    std::shared_ptr<const map_snapshot> map_view = std::make_shared<map_snapshot>();

public:
    thread_params(server& server);

    void add_map(const std::shared_ptr<map>& map);
    void remove_map(uint32_t id);

private:
    void rebuild_map_view();
};

} // namespace fb::game

#endif