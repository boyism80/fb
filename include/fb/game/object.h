#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <fb/game/mmo.h>
#include <fb/game/spell.h>
#include <fb/game/lua.h>
#include <fb/core/stream.h>

namespace fb { namespace game {

class context;
class map;
class session;
class buffs;
class objects;
class items;
class sector;
class sectors;

class object : public lua::luable
{
public:
    interface listener;

public:
    class master;

public:
    struct config
    {
    public:
        uint32_t            id          = 0xFFFFFFFF;
        const point16_t     position    = fb::game::point16_t();
        fb::game::direction direction   = fb::game::direction::BOTTOM;
        fb::game::map*      map         = nullptr;
    };

public:
    enum class types : uint32_t
    { 
        UNKNOWN = 0x00, 
        ITEM    = 0x01, 
        NPC     = 0x02, 
        MOB     = 0x04, 
        SESSION = 0x08, 
        LIFE    = (MOB | SESSION),
        OBJECT  = (ITEM | NPC | MOB) 
    };

public:
    LUA_PROTOTYPE

    friend fb::game::buffs;
    friend fb::game::items;

private:
    fb::game::object::listener*         _listener;
    bool                                _visible    = true;
    fb::game::sector*                   _sector     = nullptr;

protected:
    uint32_t                            _sequence   = 0;
    const fb::game::object::master*     _master;

    point16_t                           _position   = point16_t(0, 0);
    fb::game::direction                 _direction  = fb::game::direction::BOTTOM;
    fb::game::map*                      _map        = nullptr;

public:
    fb::game::context&                  context;
    fb::game::buffs                     buffs;

protected:
    object(fb::game::context& context, const master* master, const fb::game::object::config& config);
    object(const object& right);
public:
    virtual ~object();

private:
    void                                leave();
    static bool                         sight(const point16_t me, const point16_t you, const fb::game::map* map);

public:
    template <typename T>
    typename T::listener*               get_listener() const { return dynamic_cast<typename T::listener*>(this->_listener); }
    virtual void                        destroy();
    virtual void                        send(const fb::ostream& stream, bool encrypt = true, bool wrap = true) { }
    virtual void                        send(const fb::protocol::base::header& response, bool encrypt = true, bool wrap = true) { }

    uint32_t                            sequence() const { return this->_sequence; }
    void                                sequence(uint32_t value) { this->_sequence = value; }

    const master*                       based() const;
    template <typename T>
    const typename T::master*           based() const { return static_cast<const typename T::master*>(this->_master); }
    bool                                is(object::types type) const;

    virtual const std::string&          name() const;
    virtual uint16_t                    look() const;
    virtual uint8_t                     color() const;
    virtual object::types               type() const;


    const point16_t&                    position() const;
    const point16_t                     position_forward() const;
    const point16_t                     position_forward(fb::game::direction direction) const;
    virtual bool                        position(uint16_t x, uint16_t y, bool refresh = false);
    virtual bool                        position(const point16_t position, bool refresh = false);
    bool                                move();
    bool                                move(fb::game::direction direction);

    uint16_t                            x() const;
    bool                                x(uint16_t value);

    uint16_t                            y() const;
    bool                                y(uint16_t value);

    fb::game::direction                 direction() const;
    bool                                direction(fb::game::direction value);

    virtual bool                        map(fb::game::map* map, const point16_t& position);
    bool                                map(fb::game::map* map);
    fb::game::map*                      map() const;

    bool                                sector(fb::game::sector* sector);
    fb::game::sector*                   sector();

    bool                                sight(const point16_t& position) const;
    bool                                sight(const fb::game::object& object) const;

    object*                             side(fb::game::direction direction, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>                sides(fb::game::direction direction, fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    object*                             forward(fb::game::object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>                forwards(fb::game::object::types type = fb::game::object::types::UNKNOWN) const;

    // 내 시야에서 보여지는 오브젝트들
    std::vector<object*>                showings(object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>                showings(const std::vector<object*>& source, const point16_t& position, object::types type = fb::game::object::types::UNKNOWN) const;

    // 자기 시야에 내가 있는 오브젝트들
    std::vector<object*>                showns(object::types type = fb::game::object::types::UNKNOWN) const;
    std::vector<object*>                showns(const std::vector<object*>& source, const point16_t& position, object::types type = fb::game::object::types::UNKNOWN) const;

    bool                                visible() const;
    void                                visible(bool value);

    double                              distance(const object& right) const;
    uint32_t                            distance_sqrt(const object& right) const;

    // 유효한 오브젝트인지
    virtual bool                        available() const { return true; }

public:
    virtual void                        on_timer(uint64_t elapsed_milliseconds) { }
    virtual void                        on_kill(fb::game::life& you) { }

protected:
    virtual void                        on_hold() { }

public:
    bool                                operator == (const object& right) const;
    bool                                operator != (const object& right) const;

public:
    static int                          builtin_core(lua_State* lua);
    static int                          builtin_id(lua_State* lua);
    static int                          builtin_eq(lua_State* lua);
    static int                          builtin_tostring(lua_State* lua);
    static int                          builtin_name(lua_State* lua);
    static int                          builtin_dialog(lua_State* lua);
    static int                          builtin_sound(lua_State* lua);
    static int                          builtin_position(lua_State* lua);
    static int                          builtin_direction(lua_State* lua);
    static int                          builtin_chat(lua_State* lua);
    static int                          builtin_message(lua_State* lua);
    static int                          builtin_buff(lua_State* lua);
    static int                          builtin_unbuff(lua_State* lua);
    static int                          builtin_isbuff(lua_State* lua);
    static int                          builtin_effect(lua_State* lua);
    static int                          builtin_map(lua_State* lua);
    static int                          builtin_mkitem(lua_State* lua);
    static int                          builtin_showings(lua_State* lua);
    static int                          builtin_showns(lua_State* lua);
    static int                          builtin_front(lua_State* lua);
    static int                          builtin_is(lua_State* lua);
};


interface object::listener
{
    virtual void                        on_direction(fb::game::object& me) = 0;
    virtual void                        on_show(fb::game::object& me, bool light) = 0;
    virtual void                        on_show(fb::game::object& me, fb::game::object& you, bool light) = 0;
    virtual void                        on_hide(fb::game::object& me) = 0;
    virtual void                        on_hide(fb::game::object& me, fb::game::object& you) = 0;
    virtual void                        on_move(fb::game::object& me, const point16_t& before) = 0;
    virtual void                        on_unbuff(fb::game::object& me, fb::game::buff& buff) = 0;
    virtual void                        on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) = 0;
    virtual void                        on_create(fb::game::object& me) = 0;
    virtual void                        on_destroy(fb::game::object& me) = 0;
};

class object::master : public lua::luable
{
public:
    struct config
    {
    public:
        std::string     name;
        uint16_t        look  = 0;
        uint8_t         color = 0;
    };

public:
    friend class fb::game::object;

public:
    LUA_PROTOTYPE

public:
    const std::string                   name;
    const uint16_t                      look  = 0;
    const uint8_t                       color = 0;

public:
    template <typename T, typename... Args>
    T* make(fb::game::context& context, Args... args) const
    {
        return new T(context, static_cast<const typename T::master*>(this), args...);
    }

public:
    master(const fb::game::object::master::config& config);
    virtual ~master();

protected:
    uint8_t                             dialog_look_type() const;

public:
    virtual object::types               type() const;

public:
    static int                          builtin_name(lua_State* lua);
    static int                          builtin_look(lua_State* lua);
    static int                          builtin_color(lua_State* lua);
    static int                          builtin_dialog(lua_State* lua);
};

} }

#endif // !__OBJECT_H__
