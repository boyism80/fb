#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include "model/door/door.h"
#include "model/map/sector.h"
#include "module/stream/stream.h"
#include "module/config/config.h"

namespace fb { namespace game {

class object;
class map;
class mob;
class session;

namespace wm {

typedef struct __destination_tag
{
public:
    point16_t               position;
    fb::game::map*          map;

public:
    __destination_tag() : 
        map(nullptr)
    {}

    __destination_tag(const point16_t& position, fb::game::map* map) : 
        position(position), map(map)
    {}
} destination;

typedef struct __offset_tag
{
public:
    const std::string       id;
    const std::string       name;
    const point16_t         position;
    const destination       dest;

public:
    __offset_tag(const std::string& id, const std::string& name, const point16_t& position, const destination& dest) : 
        id(id), name(name), position(position), dest(dest)
    {}

    __offset_tag(const struct __offset_tag& right) : 
        __offset_tag(right.id, right.name, right.position, right.dest)
    {}

} offset;

class group : private std::vector<offset*>
{
public:
    using std::vector<offset*>::begin;
    using std::vector<offset*>::end;
    using std::vector<offset*>::cbegin;
    using std::vector<offset*>::cend;
    using std::vector<offset*>::size;
    using std::vector<offset*>::operator[];

public:
    group();
    ~group();

public:
    void                                push(offset* offset);
    bool                                contains(const offset& offset) const;
};

class world : private std::vector<group*>
{
private:
    std::vector<offset*>                _offsets;

public:
    const std::string                   name;

public:
    using std::vector<group*>::begin;
    using std::vector<group*>::end;
    using std::vector<group*>::cbegin;
    using std::vector<group*>::cend;
    using std::vector<group*>::size;
    using std::vector<group*>::operator[];

public:
    world(const std::string& name);
    ~world();

public:
    void                                push(group* group);
    const std::vector<offset*>&         offsets() const;
    const fb::game::wm::group*          find(const std::string& name) const;
    const fb::game::wm::group*          find(const fb::game::map& map) const;
};

}

class objects : private std::vector<fb::game::object*>
{
#pragma region friend
    friend fb::game::object;
#pragma endregion

private:
    map*                            _owner;
    uint16_t                        _sequence;

public:
    using std::vector<fb::game::object*>::begin;
    using std::vector<fb::game::object*>::end;
    using std::vector<fb::game::object*>::cbegin;
    using std::vector<fb::game::object*>::cend;
    using std::vector<fb::game::object*>::rbegin;
    using std::vector<fb::game::object*>::rend;
    using std::vector<fb::game::object*>::crbegin;
    using std::vector<fb::game::object*>::crend;
    using std::vector<fb::game::object*>::size;
    using std::vector<fb::game::object*>::at;
    using std::vector<fb::game::object*>::operator[];

public:
    objects(fb::game::map* owner);
    ~objects();

#pragma region private method
private:
    uint16_t                        empty_seq();
    uint16_t                        add(fb::game::object& object);
    uint16_t                        add(fb::game::object& object, const point16_t& position);
    bool                            remove(fb::game::object& object);
#pragma endregion


#pragma region public method
public:
    std::vector<object*>            filter(fb::game::object::types type) const;
    fb::game::object*               find(uint16_t sequence);
    fb::game::object*               exists(point16_t position) const;
#pragma endregion
};

class map : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region enum
public:
    enum options : uint8_t 
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
    enum effects : uint8_t 
    { 
        NO_EFFECT = 0x00,
        FIRE = 0x01,
        WATER = 0x02 
    };
#pragma endregion


#pragma region static const field
    static const uint32_t           MAX_SCREEN_WIDTH   = 17;
    static const uint32_t           HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static const uint32_t           MAX_SCREEN_HEIGHT  = 15;
    static const uint32_t           HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);
#pragma endregion


#pragma region structure
public:
    typedef struct _tile
    {
        uint16_t                    id;
        uint16_t                    object;
        bool                        blocked;
    } tile;

    typedef struct _warp
    {
    public:
        // default warp
        fb::game::map*              map;
        const point16_t             before, after;
        const range8_t              limit;

        // world warp
        const fb::game::wm::offset* offset;

    public:
        _warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t limit) : 
            map(map), before(before), after(after), limit(limit), offset(nullptr) {}
        _warp(const fb::game::wm::offset* offset, const point16_t& before) : 
            map(nullptr), before(before), after(0, 0), limit(0, 0), offset(offset)
        {}
        _warp(const _warp& right) : 
            map(right.map), before(right.before), after(right.after), limit(right.limit), offset(right.offset)
        {}
        ~_warp() {}
    } warp;
#pragma endregion


#pragma region private field
private:
    uint16_t                        _id;
    uint16_t                        _parent;
    size16_t                        _size;
    tile*                           _tiles;
    std::string                     _name;
    options                         _option;
    effects                         _effect;
    uint8_t                         _bgm;
    std::vector<warp*>              _warps;
    fb::game::sectors*              _sectors;
#pragma endregion


#pragma region public field
public:
    fb::game::objects               objects;
    fb::game::doors                 doors;
    const std::string               host;
#pragma endregion


#pragma region constructor / destructor
public:
    map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, options option, effects effect, const std::string& host, const void* data, size_t size);
    ~map();
#pragma endregion


#pragma region public method
public:
    uint16_t                        id() const;
    uint16_t                        parent() const;
    const std::string&              name() const;
    bool                            blocked(uint16_t x, uint16_t y) const;
    bool                            block(uint16_t x, uint16_t y, bool option);
    effects                         effect() const;
    options                         option() const;
    uint16_t                        width() const;
    uint16_t                        height() const;
    size16_t                        size() const;
    uint8_t                         bgm() const;
    bool                            loaded() const;

    bool                            existable(const point16_t position) const;
    bool                            movable(const point16_t position) const;
    bool                            movable(const fb::game::object& object, fb::game::direction direction) const;
    bool                            movable_forward(const fb::game::object& object, uint16_t step = 1) const;

    void                            push_warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t& limit);
    void                            push_warp(fb::game::wm::offset* offset, const point16_t& before);
    const fb::game::map::warp*      warpable(const point16_t& position) const;

    bool                            update(fb::game::object& object);
    
    std::vector<object*>            nears(const point16_t& pivot, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>            belows(const point16_t& pivot, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>            activateds(fb::game::object::types type = fb::game::object::types::UNKNOWN);
#pragma endregion


#pragma region event method
public:
    void                            handle_timer(uint64_t elapsed_milliseconds);
#pragma endregion


#pragma region operator method
public:
    tile*                           operator () (uint16_t x, uint16_t y) const;
#pragma endregion


#pragma region built-in method
public:
    static int                      builtin_name(lua_State* lua);
    static int                      builtin_objects(lua_State* lua);
    static int                      builtin_width(lua_State* lua);
    static int                      builtin_height(lua_State* lua);
    static int                      builtin_area(lua_State* lua);
    static int                      builtin_movable(lua_State* lua);
    static int                      builtin_door(lua_State* lua);
    static int                      builtin_doors(lua_State* lua);
#pragma endregion
};

} }

#endif // !__MAP_H__
