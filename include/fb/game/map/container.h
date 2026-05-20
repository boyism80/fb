#ifndef __MAP_CONTAINER_H__
#define __MAP_CONTAINER_H__

#include <fb/container.h>
#include <fb/config.h>
#include <fb/game/map.h>

namespace fb::model {
class npc_spawn;
}

namespace fb::game {

class map_container : public fb::kv_container<uint32_t, map>
{
private:
    std::mutex _mutex;

public:
    fb::game::server& server;
    const uint32_t    host;

public:
    map_container(fb::game::server& server, uint32_t host);
    ~map_container();

private:
    static bool load_data(uint32_t id, std::vector<char>& buffer);
    static bool load_block(uint32_t id, std::vector<fb::model::point16_t>& buffer);
    void        spawn_npcs(const std::shared_ptr<map>& map);
    void        spawn_npc(const fb::model::npc_spawn& spawn, const std::shared_ptr<map>& map);

public:
    void                 load(const fb::model::map& model);
    bool                 ensure_loaded(const std::shared_ptr<map>& map);
    void                 spawn_npc(const fb::model::npc_spawn& spawn);
    std::shared_ptr<map> name2map(std::string_view name) const;
};

} // namespace fb::game

#endif