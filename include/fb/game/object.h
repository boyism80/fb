#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <fb/lua.h>
#include <fb/stream.h>
#include <fb/protocol/header.h>
#include <fb/thread_switchable.h>
#include <fb/model/model.h>
#include <fb/game/spell.h>
#include <async/task.h>

using namespace fb::model::enum_value;

namespace fb::game {

/**
 * @brief      This class describes a context.
 */
class context;
/**
 * @brief      This class describes a map of .
 */
class map;
/**
 * @brief      This class describes a character.
 */
class character;
/**
 * @brief      This class describes buffs.
 */
class buffs;
/**
 * @brief      This class describes objects.
 */
class objects;
/**
 * @brief      This class describes items.
 */
class items;
/**
 * @brief      This class describes a sector.
 */
class sector;
/**
 * @brief      This class describes sectors.
 */
class sectors;

/**
 * @brief      This class describes an object.
 */
class object : public lua::luable, public fb::thread_switchable
{
public:
    struct listener;

public:
    struct initial_params;

public:
    LUA_PROTOTYPE

    friend fb::game::buffs;
    friend fb::game::items;

private:
    fb::game::object::listener* _listener;
    bool                        _visible = true;
    fb::game::sector*           _sector  = nullptr;

protected:
    uint32_t                 _sequence = 0;
    const fb::model::object& _model;
    bool                     _map_lock  = false;
    fb::model::point16_t     _position  = fb::model::point16_t(0, 0);
    DIRECTION                _direction = DIRECTION::BOTTOM;
    fb::game::map*           _map       = nullptr;

public:
    fb::game::context& context;
    fb::game::buffs    buffs;

protected:
    object(fb::game::context& context, const fb::model::object& model, const initial_params& c);
    object(const object& right);

public:
    virtual ~object();

private:
    void        leave();
    static bool sight(const fb::model::point16_t me, const fb::model::point16_t you, const fb::game::map* map);

public:
    template <typename T>
    typename T::listener* get_listener() const
    {
        return dynamic_cast<typename T::listener*>(this->_listener);
    }

public:
    template <typename T> const T& based() const
    {
        return static_cast<const T&>(this->_model);
    }

public:
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual async::task<size_t>             send(const fb::stream& stream, bool encrypt = true, bool wrap = true);
    virtual async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);
    uint32_t                    sequence() const;
    void                        sequence(uint32_t value);
    const fb::model::object&    based() const;
    bool                        is(OBJECT_TYPE type) const;
    virtual const std::string&  name() const;
    virtual uint16_t            look() const;
    virtual uint8_t             color() const;
    virtual OBJECT_TYPE         what() const;

    virtual void update_external(bool light);
    virtual void update_external(object& you, bool light);

    void                        chat(const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL);
    const fb::model::point16_t& position() const;
    const fb::model::point16_t  position_forward() const;
    const fb::model::point16_t  position_forward(DIRECTION direction) const;
    virtual bool                position(uint16_t x, uint16_t y, bool refresh = false);
    virtual bool                position(const fb::model::point16_t position, bool refresh = false);
    bool                        move();
    bool                        move(DIRECTION direction);
    uint16_t                    x() const;
    bool                        x(uint16_t value);
    uint16_t                    y() const;
    bool                        y(uint16_t value);
    DIRECTION                   direction() const;
    bool                        direction(DIRECTION value);
    virtual async::task<bool>   map(fb::game::map*              map,
                                    const fb::model::point16_t& position     = fb::model::point16_t{0, 0},
                                    DESTROY_TYPE                destroy_type = DESTROY_TYPE::DEFAULT);
    fb::game::map*              map() const;
    bool                        sector(fb::game::sector* sector);
    fb::game::sector*           sector();
    bool                        sight(const fb::model::point16_t& position) const;
    bool                        sight(const fb::game::object& object) const;
    object*                     side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>        sides(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    object*                     forward(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*>        forwards(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    bool                        visible() const;
    void                        visible(bool value);
    double                      distance(const object& right) const;
    uint32_t                    distance_sqrt(const object& right) const;
    virtual bool                condition(const std::vector<fb::model::dsl>& conditions) const;
    virtual bool                available() const;

    virtual void hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    virtual void         hide(fb::game::object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    std::vector<object*> showings(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*> showings(const std::vector<object*>& source,
                                  const fb::model::point16_t& position,
                                  OBJECT_TYPE                 type = OBJECT_TYPE::UNKNOWN) const;

    std::vector<object*> showns(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<object*> showns(const std::vector<object*>& source,
                                const fb::model::point16_t& position,
                                OBJECT_TYPE                 type = OBJECT_TYPE::UNKNOWN) const;

    fb::thread* thread() const override;

    void assert_thread() const override;

    virtual void update_id();
    virtual void update_map(const fb::game::map& map);
    virtual void update_bgm(uint16_t bgm, uint8_t volume);
    virtual void update_position();

    void sound(SOUND sound);
    void effect(uint8_t value);

public:
    bool operator== (const object& right) const;
    bool operator!= (const object& right) const;

public:
    static int builtin_model(lua_State* lua);
    static int builtin_id(lua_State* lua);
    static int builtin_eq(lua_State* lua);
    static int builtin_tostring(lua_State* lua);
    static int builtin_name(lua_State* lua);
    static int builtin_sound(lua_State* lua);
    static int builtin_position(lua_State* lua);
    static int builtin_direction(lua_State* lua);
    static int builtin_chat(lua_State* lua);
    static int builtin_message(lua_State* lua);
    static int builtin_buff(lua_State* lua);
    static int builtin_unbuff(lua_State* lua);
    static int builtin_isbuff(lua_State* lua);
    static int builtin_effect(lua_State* lua);
    static int builtin_map(lua_State* lua);
    static int builtin_mkitem(lua_State* lua);
    static int builtin_showings(lua_State* lua);
    static int builtin_showns(lua_State* lua);
    static int builtin_front(lua_State* lua);
    static int builtin_is(lua_State* lua);

    static int builtin_thread(lua_State* lua);

    static int builtin_ptr(lua_State* lua);

    static int builtin_near(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct object::listener
{
    virtual void on_chat(fb::game::object& me, const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;
    virtual void on_direction(fb::game::object& me)                                                                 = 0;
    virtual void on_update_external(fb::game::object& me, bool light)                                                          = 0;
    virtual void on_update_external(fb::game::object& me, fb::game::object& you, bool light)                                   = 0;
    virtual void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)                   = 0;
    virtual void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type)                    = 0;
    virtual void on_move(fb::game::object& me, const fb::model::point16_t& before)                                  = 0;
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff)                                              = 0;
    virtual void on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after)                  = 0;
    virtual void on_create(fb::game::object& me)                                                                    = 0;
    virtual void on_destroy(fb::game::object& me)                                                                   = 0;

    virtual void on_sound(fb::game::object& ch, SOUND sound)    = 0;
    virtual void on_effect(fb::game::object& ch, uint8_t value) = 0;
};

/**
 * @brief      { struct_description }
 */
struct object::initial_params
{
public:
    uint32_t                   id        = 0xFFFFFFFF;
    const fb::model::point16_t position  = fb::model::point16_t();
    DIRECTION                  direction = DIRECTION::BOTTOM;
    fb::game::map*             map       = nullptr;
};

} // namespace fb::game

#endif // !__OBJECT_H__
