#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <module/stream/stream.h>
#include <model/mmo/mmo.h>
#include <model/spell/spell.h>
#include <model/lua/lua.h>

namespace fb { namespace game {

class map;
class session;
class buffs;
class objects;
class items;
class listener;

class object : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region friend
    friend fb::game::objects;
    friend fb::game::buffs;
    friend fb::game::items;
#pragma endregion

#pragma region enum of object
public:
    enum types 
    { 
        UNKNOWN = 0x00, 
        ITEM    = 0x01, 
        NPC     = 0x02, 
        MOB     = 0x04, 
        SESSION = 0x08, 
        LIFE    = (MOB | SESSION),
        OBJECT  = (ITEM | NPC | MOB) 
    };
#pragma endregion

#pragma region master class
public:
class master : public lua::luable
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region protected field
protected:
    std::string                 _name;
    uint16_t                    _look;
    uint8_t                     _color;
#pragma endregion


#pragma region friend
public:
    friend class fb::game::object;
#pragma endregion


#pragma region constructor / destructor
public:
    master(const std::string& name = "", uint16_t look = 0, uint8_t color = 0);
    virtual ~master();
#pragma endregion


#pragma region protected method
protected:
    uint8_t                     dialog_look_type() const;
    //virtual void              handle_lua_field(lua_State* lua) const;
#pragma endregion


#pragma region public method
public:
    virtual object::types       type() const;

    const std::string&          name() const;
    void                        name(const std::string& value);

    uint16_t                    look() const;
    void                        look(uint16_t value);

    uint8_t                     color() const;
    void                        color(uint8_t value);
#pragma endregion
        

#pragma region make stream method
public:
    fb::ostream                 make_dialog_stream(const std::string& message, bool button_prev, bool button_next, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::NORMAL) const;
#pragma endregion


#pragma region build-in method
public:
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_look(lua_State* lua);
    static int                  builtin_color(lua_State* lua);
    static int                  builtin_dialog(lua_State* lua);
#pragma endregion
};
#pragma endregion

#pragma region private field
private:
    listener*                   _listener;
    bool                        _visible;
#pragma endregion

#pragma region protected field
protected:
    const master*               _master;
    point16_t                   _before;

    point16_t                   _position;
    fb::game::direction         _direction;
    fb::game::map*              _map;
#pragma endregion

#pragma region public field
public:
    buffs                       buffs;
#pragma endregion

#pragma region constructor / destructor
protected:
    object(const master* master, listener* listener, uint32_t id = 0xFFFFFFFF, const point16_t position = fb::game::point16_t(), fb::game::direction direction = fb::game::direction::BOTTOM, fb::game::map* map = nullptr);
    object(const object& right);
public:
    virtual ~object();
#pragma endregion

#pragma region private method
private:
    static bool                 sight(const point16_t me, const point16_t you, const fb::game::map* map);
    void                        map(fb::game::map* map, const point16_t& position);
    void                        map(fb::game::map* map);
#pragma endregion

#pragma region public method
public:
    const master*               based() const;
    template <typename T>
    const T*                    based() const { return static_cast<const T*>(this->_master); }
    bool                        is(object::types type);

    virtual const std::string&  name() const;
    virtual uint16_t            look() const;
    virtual uint8_t             color() const;
    virtual object::types       type() const;


    point16_t                   position() const;
    point16_t                   position_forward() const;
    virtual bool                position(uint16_t x, uint16_t y);
    virtual bool                position(const point16_t position);
    bool                        move();
    bool                        move(fb::game::direction direction);

    uint16_t                    x() const;
    bool                        x(uint16_t value);

    uint16_t                    y() const;
    bool                        y(uint16_t value);

    direction                   direction() const;
    bool                        direction(fb::game::direction value);

    fb::game::map*              map() const;

    bool                        sight(const point16_t& position, bool before = false) const;
    bool                        sight(const fb::game::object& object, bool before_me = false, bool before_you = false) const;

    session*                    near_session(fb::game::direction direction) const;
    std::vector<session*>       near_sessions(fb::game::direction direction) const;
    session*                    forward_session() const;
    std::vector<session*>       forward_sessions() const;

    object*                     near_object(fb::game::direction direction, fb::game::object::types type) const;
    std::vector<object*>        near_objects(fb::game::direction direction, fb::game::object::types type) const;
    object*                     forward_object(fb::game::object::types type) const;
    std::vector<object*>        forward_objects(fb::game::object::types type) const;

    std::vector<object*>        showings(object::types type = object::types::UNKNOWN) const;        // 내가 볼 수 있는
    std::vector<object*>        showns(object::types type = object::types::UNKNOWN) const;          // 날 보고 있는

    bool                        visible() const;
    void                        visible(bool value);

    double                      distance(const object& right) const;
    uint32_t                    distance_sqrt(const object& right) const;
#pragma endregion

#pragma region make stream method
public:
    fb::ostream                 make_direction_stream() const;
    static fb::ostream          make_show_stream(const std::vector<fb::game::object*>& objects);
    virtual fb::ostream         make_show_stream(bool light = false) const;
    fb::ostream                 make_hide_stream() const;
    fb::ostream                 make_chat_stream(const std::string& message, chat::type type = chat::NORMAL) const;
    fb::ostream                 make_sound_stream(fb::game::sound::type sound) const;
    fb::ostream                 make_effect_stream(uint8_t effect) const;
    fb::ostream                 make_move_stream(bool from_before = true) const;
    fb::ostream                 make_move_stream(fb::game::direction direction, bool from_before = true) const;
    fb::ostream                 make_dialog_stream(const std::string& message, bool button_prev, bool button_next) const;
#pragma endregion

#pragma region handler method
public:
    virtual void                handle_timer(uint64_t elapsed_milliseconds) {}
#pragma endregion

#pragma region operator
    bool                        operator == (const object& right) const;
    bool                        operator != (const object& right) const;
#pragma endregion

#pragma region build-in method
public:
    static int                  builtin_core(lua_State* lua);
    static int                  builtin_id(lua_State* lua);
    static int                  builtin_eq(lua_State* lua);
    static int                  builtin_tostring(lua_State* lua);
    static int                  builtin_name(lua_State* lua);
    static int                  builtin_dialog(lua_State* lua);
    static int                  builtin_sound(lua_State* lua);
    static int                  builtin_position(lua_State* lua);
    static int                  builtin_direction(lua_State* lua);
    static int                  builtin_chat(lua_State* lua);
    static int                  builtin_message(lua_State* lua);
    static int                  builtin_buff(lua_State* lua);
    static int                  builtin_unbuff(lua_State* lua);
    static int                  builtin_isbuff(lua_State* lua);
    static int                  builtin_effect(lua_State* lua);
    static int                  builtin_map(lua_State* lua);
    static int                  builtin_mkitem(lua_State* lua);
    static int                  builtin_showings(lua_State* lua);
    static int                  builtin_showns(lua_State* lua);
    static int                  builtin_front(lua_State* lua);
    static int                  builtin_is(lua_State* lua);
#pragma endregion
};

} }

#endif // !__OBJECT_H__