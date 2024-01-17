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

namespace fb { namespace game {

class object;
class map;
class mob;
class session;

namespace wm {

struct destination
{
public:
    point16_t               position = point16_t(0, 0);
    fb::game::map*          map      = nullptr;

public:
    destination() : 
        map(nullptr)
    { }

    destination(const point16_t& position, fb::game::map* map) : 
        position(position), map(map)
    { }
};

struct offset
{
public:
    const std::string       id;
    const std::string       name;
    const point16_t         position = point16_t(0, 0);
    const destination       dst;

public:
    offset(const std::string& id, const std::string& name, const point16_t& position, const destination& dst) : 
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
    uint16_t                        add(fb::game::object& object, const point16_t& position);
    bool                            remove(fb::game::object& object);

public:
    std::vector<object*>            filter(fb::game::object::types type) const;
    fb::game::object*               exists(point16_t position) const;

public:
    fb::game::object*               operator [] (uint32_t seq) const;
};

class map : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    enum class OPTION : uint8_t 
    { 
        NO_OPTION                   = 0x00,
        BUILD_IN                    = 0x01,
        DISABLE_TALK                = 0x02,
        DISABLE_WHISPER             = 0x04,
        DISABLE_MAGIC               = 0x08,
        HUNTING_GROUND              = 0x10,
        ENABLE_PK                   = 0x20,
        DISABLE_DIE_PENALTY         = 0x30 
    };
    enum class EFFECT : uint8_t 
    { 
        NONE                        = 0x00,
        FIRE                        = 0x01,
        WATER                       = 0x02 
    };

    static constexpr uint32_t       MAX_SCREEN_WIDTH   = 17;
    static constexpr uint32_t       HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static constexpr uint32_t       MAX_SCREEN_HEIGHT  = 15;
    static constexpr uint32_t       HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
    struct tile;
    struct warp;

public:
    using unique_tiles              = std::unique_ptr<tile[]>;
    using unique_warps              = std::vector<std::unique_ptr<warp>>;
    using unique_sector             = std::unique_ptr<fb::game::sectors>;

private:
    uint16_t                        _id       = 0;
    uint16_t                        _parent   = 0;
    size16_t                        _size     = size16_t(0, 0);
    unique_tiles                    _tiles    = nullptr;
    std::string                     _name;
    OPTION                          _option   = OPTION::NO_OPTION;
    EFFECT                          _effect   = EFFECT::NONE;
    uint8_t                         _bgm      = 0;
    unique_warps                    _warps;
    unique_sector                   _sectors;

public:
    fb::game::objects               objects = fb::game::objects(this);
    fb::game::doors                 doors;
    const uint32_t                  group;
    const bool                      active;

public:
    map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, OPTION option, EFFECT effect, uint32_t group, const void* data, size_t size);
    ~map();

public:
    uint16_t                        id() const;
    uint16_t                        parent() const;
    const std::string&              name() const;
    bool                            blocked(uint16_t x, uint16_t y) const;
    bool                            block(uint16_t x, uint16_t y, bool option);
    EFFECT                          effect() const;
    OPTION                          option() const;
    uint16_t                        width() const;
    uint16_t                        height() const;
    size16_t                        size() const;
    uint8_t                         bgm() const;
    bool                            loaded() const;

    bool                            existable(const point16_t position) const;
    bool                            movable(const point16_t position) const;
    bool                            movable(const fb::game::object& object, fb::game::DIRECTION_TYPE direction) const;
    bool                            movable_forward(const fb::game::object& object, uint16_t step = 1) const;

    void                            push_warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t& condition);
    void                            push_warp(fb::game::wm::offset* offset, const point16_t& before);
    const fb::game::map::warp*      warpable(const point16_t& position) const;

    bool                            update(fb::game::object& object);
    bool                            activated() const;
    
    std::vector<object*>            nears(const point16_t& pivot, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>            belows(const point16_t& pivot, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>            activateds(fb::game::object::types type = fb::game::object::types::UNKNOWN);

public:
    void                            on_timer(uint64_t elapsed_milliseconds);

public:
    tile*                           operator () (uint16_t x, uint16_t y) const;

public:
    static int                      builtin_name(lua_State* lua);
    static int                      builtin_objects(lua_State* lua);
    static int                      builtin_width(lua_State* lua);
    static int                      builtin_height(lua_State* lua);
    static int                      builtin_area(lua_State* lua);
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

struct map::warp
{
public:
    // default warp
    fb::game::map*                  map;
    const point16_t                 before, after;
    const range8_t                  condition;

    // world warp
    const fb::game::wm::offset*     offset;

public:
    warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t condition) : 
        map(map), before(before), after(after), condition(condition), offset(nullptr) { }
    warp(const fb::game::wm::offset* offset, const point16_t& before) : 
        map(nullptr), before(before), after(0, 0), condition(0, 0), offset(offset)
    { }
    warp(const warp& right) : 
        map(right.map), before(right.before), after(right.after), condition(right.condition), offset(right.offset)
    { }
    ~warp() { }
};

} }

#endif // !__MAP_H__
