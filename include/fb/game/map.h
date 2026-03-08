#ifndef __MAP_H__
#define __MAP_H__

#include <fb/game/door.h>
#include <fb/game/object.h>
#include <fb/game/sector.h>

namespace fb::game {

class map : public fb::thread_switchable
{
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
    fb::model::size16_t      _size  = fb::model::size16_t(0, 0);
    unique_tiles             _tiles = nullptr;
    std::shared_ptr<sectors> _sectors;

public:
    const fb::game::server& server;
    const fb::model::map&   model;
    object::container       objects = object::container(*this);
    door::container         doors   = door::container(*this);
    const bool              active;

public:
    map(fb::game::server& server, const fb::model::map& model, bool active, const void* data, size_t size);
    map(const fb::game::map&) = delete;
    ~map();

private:
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
    std::shared_ptr<sector>         sector_at(const fb::model::point16_t& position);
    std::vector<object_ptr>         nears(const fb::model::point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    std::vector<object_ptr>         belows(const fb::model::point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    void                            bulk_update(const std::vector<uint32_t>& oids);
    fb::thread*                     thread() const override;
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

} // namespace fb::game

#endif // !__MAP_H__
