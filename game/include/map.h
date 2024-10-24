#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <memory>
#include <door.h>
#include <sector.h>
#include <fb/stream.h>
#include <fb/config.h>
#include <fb/container.h>

namespace fb { namespace game {

class object;
class map;
class mob;
class character;
class context;

class objects
{
public:
    using ptrs           = std::unordered_map<uint32_t, std::unique_ptr<object>>;
    using refs           = std::unordered_map<uint32_t, object&>;
    using iterator       = refs::iterator;
    using const_iterator = refs::const_iterator;
    using filter_func    = std::function<bool(object&)>;

private:
    ptrs     _ptrs;
    refs     _refs;
    uint32_t _sequence = 1;

public:
    fb::game::map& owner;

public:
    objects(fb::game::map& map);
    ~objects() = default;

private:
    uint32_t empty_seq();

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    uint32_t       size() const;
    object&        at(uint32_t i);
    void           push(object& obj);
    object&        pop(uint32_t fd);
    object&        pop(object& obj);
    object*        try_pop(uint32_t fd);
    object*        try_pop(object& obj);
    void foreach (OBJECT_TYPE type, const filter_func& fn);
    bool contains(const object& obj) const;
    bool contains(uint32_t fd) const;

public:
    object* operator[] (uint32_t fd);
};

class map : public lua::luable
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

public:
    using unique_tiles  = std::unique_ptr<tile[]>;
    using unique_sector = std::unique_ptr<fb::game::sectors>;

private:
    size16_t      _size  = size16_t(0, 0);
    unique_tiles  _tiles = nullptr;
    unique_sector _sectors;

public:
    const fb::game::context& context;
    const fb::model::map&    model;
    fb::game::objects        objects = fb::game::objects(*this);
    fb::game::doors          doors;
    const bool               active;

public:
    map(const fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size);
    map(const map&) = delete;
    ~map();

public:
    uint64_t               index(const point16_t& p) const;
    point16_t              point(uint64_t i) const;

    bool                   blocked(uint16_t x, uint16_t y) const;
    bool                   block(uint16_t x, uint16_t y, bool option);
    uint16_t               width() const;
    uint16_t               height() const;
    size16_t               size() const;
    bool                   loaded() const;

    bool                   existable(const point16_t position) const;
    bool                   movable(const point16_t position) const;
    bool                   movable(const fb::game::object& object, DIRECTION direction) const;
    bool                   movable_forward(const fb::game::object& object, uint16_t step = 1) const;
    const fb::model::warp* warpable(const point16_t& position) const;

    bool                   update(fb::game::object& object);
    bool                   activated() const;

    std::vector<object*>   nears(const point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>   belows(const point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>   activateds(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN);

public:
    void on_timer(uint64_t elapsed_milliseconds);

public:
    tile* operator() (uint16_t x, uint16_t y) const;

public:
    static int builtin_width(lua_State* lua);
    static int builtin_height(lua_State* lua);
    static int builtin_area(lua_State* lua);
    static int builtin_objects(lua_State* lua);
    static int builtin_movable(lua_State* lua);
    static int builtin_door(lua_State* lua);
    static int builtin_doors(lua_State* lua);
    static int builtin_contains(lua_State* lua);
};

struct map::tile
{
    uint16_t id;
    uint16_t object;
    bool     blocked;
};

class maps : public fb::kv_container<uint32_t, fb::game::map>
{
private:
    std::mutex _mutex;

public:
    const fb::game::context& context;
    const uint32_t           host;

public:
    maps(const fb::game::context& context, uint32_t host);
    ~maps();

private:
    static bool load_data(uint32_t id, std::vector<char>& buffer);
    static bool load_block(uint32_t id, Json::Value& buffer);

public:
    void           load(const fb::model::map& moel);
    fb::game::map* name2map(const std::string& name) const;
};

}} // namespace fb::game

#endif // !__MAP_H__
