#ifndef __MAP_H__
#define __MAP_H__

#include <fb/container.h>
#include <fb/config.h>
#include <fb/game/door.h>
#include <fb/game/object.h>
#include <fb/game/sector.h>
#include <fb/shard_container.h>
#include <fb/synchronized.h>
#include <atomic>
#include <optional>
#include <queue>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace fb::model {
class npc_spawn;
}

namespace fb::game {

class character;
class server;

class map : public fb::thread_switchable
{
public:
    class container;

private:
    friend class container;

public:
    LUA_PROTOTYPE

public:
    static constexpr uint32_t MAX_SCREEN_WIDTH   = 17;
    static constexpr uint32_t HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static constexpr uint32_t MAX_SCREEN_HEIGHT  = 15;
    static constexpr uint32_t HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
    struct tile;
    struct cache_bytes;

public:
    using unique_tiles = std::unique_ptr<tile[]>;
    using object_ptr   = std::shared_ptr<object>;

private:
    fb::model::size16_t _size  = fb::model::size16_t(0, 0);
    unique_tiles        _tiles = nullptr;
    std::atomic<bool>   _init_script_invoked{false};

public:
    const uint32_t          id;
    const fb::game::server& server;
    const fb::model::map&   model;
    object::container       objects = object::container(*this);
    door::container         doors   = door::container(*this);
    sector::container       sectors = sector::container(*this);
    const bool              active;

public:
    map(fb::game::server& server, uint32_t id, const fb::model::map& model, bool active, const void* data, size_t size);
    map(const fb::game::map&) = delete;
    virtual ~map();

private:
    void load_tiles(const void* data, size_t size);
    void update_door();

public:
    // clang-format off
    uint64_t                        index(const fb::model::point16_t& p) const;
    fb::model::point16_t            point(uint64_t i) const;
    bool                            blocked(uint16_t x, uint16_t y) const;
    bool                            block(uint16_t x, uint16_t y, bool option);
    uint16_t                        width() const;
    uint16_t                        height() const;
    fb::model::size16_t             size() const;
    bool                            loaded() const;
    bool                            in_ground(const fb::model::point16_t position) const;
    bool                            movable(const fb::model::point16_t& position, const std::function<bool(const object&)>& predicate) const;
    bool                            movable(const object& object, const fb::model::point16_t position) const;
    bool                            movable(const object& object, DIRECTION direction) const;
    bool                            movable_forward(const object& object, uint16_t step = 1) const;
    const fb::model::warp*          warpable(const fb::model::point16_t& position) const;
    bool                            is_active() const;
    std::vector<object_ptr>         nears(const fb::model::point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    std::vector<object_ptr>         belows(const fb::model::point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    void                            bulk_update(const std::vector<uint32_t>& oids);
    void                            rezen_force() const;
    fb::thread*                     thread() const override;
    virtual bool                    is_instance() const;
    virtual std::shared_ptr<map>    source() const;
    virtual uint32_t                slot() const;
    virtual bool                    closing() const;
    virtual void                    on_character_enter();
    virtual void                    on_character_leave();
    virtual bool                    begin_destroy();
    // clang-format on

public:
    tile* operator() (uint16_t x, uint16_t y) const;
};

struct map::tile
{
    uint16_t id;
    uint16_t object;
    bool     blocked;
};

struct map::cache_bytes
{
    uint64_t             hash = 0;
    uint16_t             crc  = 0;
    std::vector<uint8_t> bytes;
};

class map::container
{
public:
    using registry   = fb::kv_container<uint32_t, fb::game::map>;
    using snapshot_t = std::vector<std::shared_ptr<fb::game::map>>;

private:
    struct slot_pool
    {
        uint32_t                                                     next = 1;
        std::set<uint32_t>                                           available;
        std::unordered_map<uint32_t, std::shared_ptr<fb::game::map>> by_slot;
    };

    fb::synchronized<registry>                             _maps;
    uint32_t                                               _sequence = 1;
    std::queue<uint32_t>                                   _available_seq;
    std::unordered_map<uint32_t, slot_pool>                _slot_pools;
    std::mutex                                             _load_mutex;
    mutable std::atomic<std::shared_ptr<const snapshot_t>> _snapshot{std::make_shared<snapshot_t>()};

    fb::sharded_container<fb::game::map::cache_bytes, 1024, uint64_t> _update_cache;
    mutable std::shared_mutex                                         _update_cache_mutex;

public:
    fb::game::server& server;
    const uint32_t    host;

public:
    container(fb::game::server& server, uint32_t host);
    ~container();

private:
    static bool       load_data(uint32_t id, std::vector<char>& buffer);
    static bool       load_block(uint32_t id, std::vector<fb::model::point16_t>& buffer);
    void              spawn_npcs(const std::shared_ptr<fb::game::map>& map);
    void              spawn_npc(const fb::model::npc_spawn& spawn, const std::shared_ptr<fb::game::map>& map);
    bool              try_mark_init_script(const std::shared_ptr<fb::game::map>& map);
    async::task<void> run_init_script(const std::shared_ptr<fb::game::map>& map);
    void              append_snapshot(const std::shared_ptr<fb::game::map>& map);
    void              remove_snapshot(uint32_t id);
    uint32_t          allocate_id(registry& registry);
    uint32_t          allocate_slot(slot_pool& pool);
    void              release_slot(uint32_t model_id, uint32_t slot, const std::shared_ptr<fb::game::map>& map);
    void              register_slot(uint32_t model_id, uint32_t slot, const std::shared_ptr<fb::game::map>& map);
    std::shared_ptr<fb::game::map> create_instance(const std::shared_ptr<fb::game::map>&    source,
                                                   uint32_t                                 slot,
                                                   const std::vector<char>&                 binary,
                                                   const std::vector<fb::model::point16_t>& blocks);

public:
    bool                              contains(uint32_t id) const;
    std::shared_ptr<fb::game::map>    find(uint32_t id) const;
    std::shared_ptr<fb::game::map>    operator[] (uint32_t id) const;
    std::shared_ptr<const snapshot_t> snapshot() const;
    void                              insert(const std::shared_ptr<fb::game::map>& map);
    void                              erase(uint32_t id);
    void                              load(const fb::model::map& model);
    bool                              ensure_loaded(const std::shared_ptr<fb::game::map>& map);
    void                              invoke_init_script(const std::shared_ptr<fb::game::map>& map);
    async::task<void>                 invoke_init_script_wait(const std::shared_ptr<fb::game::map>& map);
    void                              spawn_npc(const fb::model::npc_spawn& spawn);
    async::task<void>                 cleanup();
    std::shared_ptr<fb::game::map>    name2map(std::string_view name) const;
    std::shared_ptr<fb::game::map>    clone(const std::shared_ptr<fb::game::map>& source);
    std::shared_ptr<fb::game::map>    ensure_instance(const std::shared_ptr<fb::game::map>& source, uint32_t slot);
    async::task<void>                 destroy(const std::shared_ptr<fb::game::map>& map);
    void                              rezen_force();
    void                              erase_map_cache(uint32_t map_id, const fb::model::point16_t& point);
    std::optional<fb::stream>         map_update_stream(character&                  ch,
                                                        const fb::game::map&        map,
                                                        const fb::model::point16_t& position,
                                                        const fb::model::size8_t&   size,
                                                        uint16_t                    crc);
    void                              update_map_cache(uint32_t map_id, const fb::model::area<uint16_t>& area);
};

} // namespace fb::game

#endif // !__MAP_H__
