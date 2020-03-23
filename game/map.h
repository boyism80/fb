#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include "stream.h"
#include "zlib/zlib.h"
#include "object.h"

namespace fb { namespace game {

class session;

typedef struct _door_element
{
    uint16_t                    open;
    uint16_t                    close;

    _door_element(uint16_t open, uint16_t close) : open(open), close(close) {}
    ~_door_element() {}
} door_element;

class door : private std::vector<door_element>
{
public:
    using std::vector<door_element>::begin;
    using std::vector<door_element>::end;
    using std::vector<door_element>::cbegin;
    using std::vector<door_element>::cend;
    using std::vector<door_element>::rbegin;
    using std::vector<door_element>::rend;
    using std::vector<door_element>::crbegin;
    using std::vector<door_element>::crend;
    using std::vector<door_element>::size;
    using std::vector<door_element>::push_back;
    using std::vector<door_element>::operator[];
};

class objects : private std::vector<fb::game::object*>
{
private:
    uint16_t                    _sequence;

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
    using std::vector<fb::game::object*>::operator[];

public:
    objects();
    ~objects();

#pragma region private method
private:
    uint16_t                    empty_seq();
#pragma endregion


#pragma region public method
public:
    std::vector<session*>       sessions();
    fb::game::object*           at(uint16_t id);
    uint16_t                    add(fb::game::object& object);
    uint16_t                    add(fb::game::object& object, const point16_t position);
    bool                        remove(fb::game::object& object);
    fb::game::object*           exists(point16_t position) const;
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
        NO_OPTION = 0x00,
        BUILD_IN = 0x01,
        DISABLE_TALK = 0x02,
        DISABLE_WHISPER = 0x04,
        DISABLE_MAGIC = 0x08,
        HUNTING_GROUND = 0x10,
        ENABLE_PK = 0x20,
        DISABLE_DIE_PENALTY = 0x30 
    };
    enum effects : uint8_t 
    { 
        NO_EFFECT = 0x00,
        HOT_WAVE = 0x01,
        SHIPWRECT = 0x02 
    };
#pragma endregion


#pragma region static const field
    static const uint32_t       MAX_SCREEN_WIDTH   = 17;
    static const uint32_t       HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static const uint32_t       MAX_SCREEN_HEIGHT  = 15;
    static const uint32_t       HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);
#pragma endregion


#pragma region structure
public:
    typedef struct _tile
    {
        uint16_t id;
        uint16_t object;
        bool blocked;
    } tile;

    typedef struct _warp
    {
    public:
        map* map;
        const point16_t before, after;
        const range8_t limit;

    public:
        _warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t limit) : 
            map(map), before(before), after(after), limit(limit) {}
        _warp(const _warp& right) : 
            map(right.map), before(right.before), after(right.after), limit(right.limit)
        {}
        ~_warp() {}
    } warp;
#pragma endregion


#pragma region class
class door
{
private:
    game::map*                  _owner;
    game::door&                 _core;
    bool                        _opened;

public:
    const point16_t             position;

public:
    door(fb::game::map* owner, fb::game::door& core, const point16_t position, bool opened);
    ~door();

public:
    const fb::game::door&       core() const;
    bool                        toggle();
    bool                        opened() const;
};
#pragma endregion


#pragma region private field
private:
    uint16_t                    _id;
    uint16_t                    _parent;
    size16_t                    _size;
    tile*                       _tiles;
    std::string                 _name;
    options                     _option;
    effects                     _effect;
    uint8_t                     _bgm;
    std::vector<warp*>          _warps;
    std::vector<door*>          _doors;
#pragma endregion


#pragma region public field
public:
    objects                     objects;
#pragma endregion


#pragma region constructor / destructor
public:
    map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, options option, effects effect, const void* data, uint32_t size);
    ~map();
#pragma endregion


#pragma region private method
    bool                        matched_door(const fb::game::door& door, const point16_t& position, bool open) const;
    bool                        find_door(const fb::game::door& door, point16_t& position, bool open) const;
#pragma endregion


#pragma region public method
public:
    uint16_t                    id() const;
    uint16_t                    parent() const;
    const std::string&          name() const;
    bool                        blocked(uint16_t x, uint16_t y) const;
    bool                        block(uint16_t x, uint16_t y, bool option);
    effects                     effect() const;
    options                     option() const;
    uint16_t                    width() const;
    uint16_t                    height() const;
    size16_t                    size() const;
    uint8_t                     bgm() const;
    fb::game::map::door*        find_door(const point16_t& position);

    bool                        existable(const point16_t position) const;
    bool                        movable(const point16_t position) const;
    bool                        movable(const fb::game::object& object, fb::game::direction direction) const;
    bool                        movable_forward(const fb::game::object& object, uint16_t step = 1) const;

    void                        warp_add(map* map, const point16_t& before, const point16_t& after, const range8_t& limit);
    const map::warp*            warpable(const point16_t& position) const;
#pragma endregion


#pragma region event method
public:
    void                        handle_timer(uint64_t elapsed_milliseconds);
#pragma endregion


#pragma region operator method
public:
    tile*                       operator () (uint16_t x, uint16_t y) const;
#pragma endregion


#pragma region make stream method
public:
    fb::ostream                 make_config_stream() const;
    fb::ostream                 make_bgm_stream() const;
    fb::ostream                 make_update_stream(uint16_t begin_x, uint16_t begin_y, uint8_t width, uint8_t height, uint16_t crc = 0xFFFF) const;
#pragma endregion


#pragma region built-in method
public:
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_objects(lua_State* lua);
    static int                  builtin_width(lua_State* lua);
    static int                  builtin_height(lua_State* lua);
    static int                  builtin_area(lua_State* lua);
    static int                  builtin_movable(lua_State* lua);
#pragma endregion
};

} }

#endif // !__MAP_H__
