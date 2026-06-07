#ifndef __FB_GAME_SERVICE_MAP_H__
#define __FB_GAME_SERVICE_MAP_H__

#include <fb/container.h>
#include <fb/config.h>
#include <fb/game/map.h>
#include <fb/shard_container.h>
#include <shared_mutex>

namespace fb::model {
class npc_spawn;
}

namespace fb::game {
class character;
class server;
} // namespace fb::game

namespace fb::game::service {

// Registry is populated once during startup via load(); no further insertions.
class map : private fb::kv_container<uint32_t, fb::game::map>
{
private:
    using registry = fb::kv_container<uint32_t, fb::game::map>;

    std::mutex _mutex;

    fb::sharded_container<fb::game::map::cache_bytes, 1024, uint64_t> _update_cache;
    mutable std::shared_mutex                                         _update_cache_mutex;

public:
    using registry::contains;
    using registry::operator[];
    using registry::begin;
    using registry::end;

    fb::game::server& server;
    const uint32_t    host;

public:
    map(fb::game::server& server, uint32_t host);
    ~map();

private:
    static bool load_data(uint32_t id, std::vector<char>& buffer);
    static bool load_block(uint32_t id, std::vector<fb::model::point16_t>& buffer);
    void        spawn_npcs(const std::shared_ptr<fb::game::map>& map);
    void        spawn_npc(const fb::model::npc_spawn& spawn, const std::shared_ptr<fb::game::map>& map);

public:
    void                           load(const fb::model::map& model);
    bool                           ensure_loaded(const std::shared_ptr<fb::game::map>& map);
    void                           spawn_npc(const fb::model::npc_spawn& spawn);
    std::shared_ptr<fb::game::map> name2map(std::string_view name) const;

    void rezen_force();
    void erase_map_cache(uint32_t map_id, const fb::model::point16_t& point);
    void send_map_cache(character&                  ch,
                        const fb::game::map&        map,
                        const fb::model::point16_t& position,
                        const fb::model::size8_t&   size,
                        uint16_t                    crc);
    void update_map_cache(uint32_t map_id, const fb::model::area<uint16_t>& area);
};

} // namespace fb::game::service

#endif
