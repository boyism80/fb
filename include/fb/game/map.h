#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <memory>
#include <fb/game/door.h>
#include <fb/game/sector.h>
#include <fb/core/stream.h>
#include <fb/core/config.h>
#include <fb/core/container.h>

namespace fb { namespace game {

class object;
class map;
class mob;
class session;
class context;

namespace wm {

struct destination
{
public:
    point<uint16_t>         position = point<uint16_t>(0, 0);
    fb::game::map*          map      = nullptr;

public:
    destination() : 
        map(nullptr)
    { }

    destination(const point<uint16_t>& position, fb::game::map* map) : 
        position(position), map(map)
    { }
};

struct offset
{
public:
    const std::string       id;
    const std::string       name;
    const point<uint16_t>   position = point<uint16_t>(0, 0);
    const destination       dst;

public:
    offset(const std::string& id, const std::string& name, const point<uint16_t>& position, const destination& dst) : 
        id(id), name(name), position(position), dst(dst)
    { }

    offset(const struct offset& right) : 
        offset(right.id, right.name, right.position, right.dst)
    { }

};

class group : private std::vector<std::unique_ptr<offset>>
{
public:
    using std::vector<std::unique_ptr<offset>>::begin;
    using std::vector<std::unique_ptr<offset>>::end;
    using std::vector<std::unique_ptr<offset>>::cbegin;
    using std::vector<std::unique_ptr<offset>>::cend;
    using std::vector<std::unique_ptr<offset>>::size;
    using std::vector<std::unique_ptr<offset>>::operator[];

public:
    group() = default;
    group(const group&) = delete;
    group(group&&) = delete;
    ~group() = default;

public:
    group& operator = (group&) = delete;
    group& operator = (group&&) = delete;

public:
    void                                push(offset* offset);
    bool                                contains(const offset& offset) const;
};

class world : private std::vector<std::unique_ptr<group>>
{
private:
    std::vector<offset*>                _offsets;

public:
    const std::string                   name;

public:
    using std::vector<std::unique_ptr<group>>::begin;
    using std::vector<std::unique_ptr<group>>::end;
    using std::vector<std::unique_ptr<group>>::cbegin;
    using std::vector<std::unique_ptr<group>>::cend;
    using std::vector<std::unique_ptr<group>>::size;
    using std::vector<std::unique_ptr<group>>::operator[];

public:
    world(const std::string& name);
    world(const world&) = delete;
    world(world&&) = delete;
    ~world() = default;

public:
    world& operator = (world&) = delete;
    world& operator = (world&&) = delete;

public:
    void                                push(group* group);
    const std::vector<offset*>&         offsets() const;
    const fb::game::wm::group*          find(const std::string& name) const;
    const fb::game::wm::group*          find(const fb::game::map& map) const;
};

}

class objects : private std::map<uint32_t, fb::game::object*>
{
    friend fb::game::object;

private:
    fb::game::map*                  _owner    = nullptr;
    uint16_t                        _sequence = 1;

public:
    using std::map<uint32_t, fb::game::object*>::begin;
    using std::map<uint32_t, fb::game::object*>::end;
    using std::map<uint32_t, fb::game::object*>::cbegin;
    using std::map<uint32_t, fb::game::object*>::cend;
    using std::map<uint32_t, fb::game::object*>::rbegin;
    using std::map<uint32_t, fb::game::object*>::rend;
    using std::map<uint32_t, fb::game::object*>::crbegin;
    using std::map<uint32_t, fb::game::object*>::crend;
    using std::map<uint32_t, fb::game::object*>::size;
    using std::map<uint32_t, fb::game::object*>::at;

public:
    objects(fb::game::map* owner);
    ~objects();

private:
    uint16_t                        empty_seq();
    uint16_t                        add(fb::game::object& object);
    uint16_t                        add(fb::game::object& object, const point<uint16_t>& position);
    bool                            remove(fb::game::object& object);

public:
    std::vector<object*>            filter(OBJECT_TYPE type) const;
    fb::game::object*               exists(point<uint16_t> position) const;

public:
    fb::game::object*               operator [] (uint32_t seq) const;
};

class map : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    static constexpr uint32_t       MAX_SCREEN_WIDTH   = 17;
    static constexpr uint32_t       HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static constexpr uint32_t       MAX_SCREEN_HEIGHT  = 15;
    static constexpr uint32_t       HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
    struct tile;

public:
    using unique_tiles              = std::unique_ptr<tile[]>;
    using unique_sector             = std::unique_ptr<fb::game::sectors>;

private:
    size<uint16_t>                  _size     = fb::model::size<uint16_t>(0, 0);
    unique_tiles                    _tiles    = nullptr;
    unique_sector                   _sectors;

public:
    const fb::game::context&        context;
    const fb::model::map&           model;
    fb::game::objects               objects = fb::game::objects(this);
    fb::game::doors                 doors;
    const bool                      active;

public:
    map(const fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size);
    map(const map&) = delete;
    ~map();

public:
    uint64_t                        index(const fb::model::point<uint16_t>& p) const;
    fb::model::point<uint16_t>      point(uint64_t i) const;

    bool                            blocked(uint16_t x, uint16_t y) const;
    bool                            block(uint16_t x, uint16_t y, bool option);
    uint16_t                        width() const;
    uint16_t                        height() const;
    size<uint16_t>                  size() const;
    bool                            loaded() const;

    bool                            existable(const fb::model::point<uint16_t> position) const;
    bool                            movable(const fb::model::point<uint16_t> position) const;
    bool                            movable(const fb::game::object& object, DIRECTION direction) const;
    bool                            movable_forward(const fb::game::object& object, uint16_t step = 1) const;

    void                            push_warp(fb::game::map* map, const fb::model::point<uint16_t>& before, const fb::model::point<uint16_t>& after);
    void                            push_warp(fb::game::wm::offset* offset, const fb::model::point<uint16_t>& before);
    const fb::model::warp*          warpable(const fb::model::point<uint16_t>& position) const;

    bool                            update(fb::game::object& object);
    bool                            activated() const;
    
    std::vector<object*>            nears(const fb::model::point<uint16_t>& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>            belows(const fb::model::point<uint16_t>& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>            activateds(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN);

public:
    void                            on_timer(uint64_t elapsed_milliseconds);

public:
    tile*                           operator () (uint16_t x, uint16_t y) const;

public:
    static int                      builtin_width(lua_State* lua);
    static int                      builtin_height(lua_State* lua);
    static int                      builtin_area(lua_State* lua);
    static int                      builtin_objects(lua_State* lua);
    static int                      builtin_movable(lua_State* lua);
    static int                      builtin_door(lua_State* lua);
    static int                      builtin_doors(lua_State* lua);
};


struct map::tile
{
    uint16_t                        id;
    uint16_t                        object;
    bool                            blocked;
};

class maps : public fb::kv_container<uint32_t, fb::game::map>
{
private:
    std::mutex              _mutex;

public:
    const fb::game::context& context;
    const uint32_t host;

public:
    maps(const fb::game::context& context, uint32_t host);
    ~maps();

private:
    static bool             load_data(uint32_t id, std::vector<char>& buffer);
    static bool             load_block(uint32_t id, Json::Value& buffer);

public:
    void                    load(const fb::model::map& moel);
    fb::game::map*          name2map(const std::string& name) const;
};

} }

#endif // !__MAP_H__
