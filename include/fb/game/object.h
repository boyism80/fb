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

class context;
class map;
class character;
class buffs;
class objects;
class items;
class sector;
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
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  c        { parameter_description }
     */
    object(fb::game::context& context, const fb::model::object& model, const initial_params& c);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    object(const object& right);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~object();

private:
    /**
     * @brief      { function_description }
     */
    void leave();

    /**
     * @brief      { function_description }
     *
     * @param[in]  me    { parameter_description }
     * @param[in]  you   You
     * @param[in]  map   The map
     *
     * @return     { description_of_the_return_value }
     */
    static bool sight(const fb::model::point16_t me, const fb::model::point16_t you, const fb::game::map* map);

public:
    /**
     * @brief      Gets the listener.
     *
     * @tparam     T     { description }
     *
     * @return     The listener.
     */
    template <typename T>
    typename T::listener* get_listener() const
    {
        return dynamic_cast<typename T::listener*>(this->_listener);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @tparam     T     { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T> const T& based() const
    {
        return static_cast<const T&>(this->_model);
    }

public:
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      { function_description }
     *
     * @param[in]  stream   The stream
     * @param[in]  encrypt  The encrypt
     * @param[in]  wrap     The wrap
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     * @param[in]  encrypt   The encrypt
     * @param[in]  wrap      The wrap
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t sequence() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void sequence(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::object& based() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    bool is(OBJECT_TYPE type) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual const std::string& name() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint16_t look() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint8_t color() const;

    /**
     * @brief      { function_description }
     *
     * @return     The object type.
     */
    virtual OBJECT_TYPE what() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  light  The light
     */
    virtual void update_external(bool light);

    /**
     * @brief      { function_description }
     *
     * @param      you    You
     * @param[in]  light  The light
     */
    virtual void update_external(object& you, bool light);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message    The message
     * @param[in]  chat_type  The chat type
     */
    void chat(const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::point16_t& position() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::point16_t position_forward() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::point16_t position_forward(DIRECTION direction) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  x        { parameter_description }
     * @param[in]  y        { parameter_description }
     * @param[in]  refresh  The refresh
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool position(uint16_t x, uint16_t y, bool refresh = false);

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     * @param[in]  refresh   The refresh
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool position(const fb::model::point16_t position, bool refresh = false);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool move();

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     *
     * @return     { description_of_the_return_value }
     */
    bool move(DIRECTION direction);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t x() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    bool x(uint16_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t y() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    bool y(uint16_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    DIRECTION direction() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    bool direction(DIRECTION value);

    /**
     * @brief      { function_description }
     *
     * @param      map           The map
     * @param[in]  position      The position
     * @param[in]  destroy_type  The destroy type
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<bool> map(fb::game::map*              map,
                                  const fb::model::point16_t& position     = fb::model::point16_t{0, 0},
                                  DESTROY_TYPE                destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::map* map() const;

    /**
     * @brief      { function_description }
     */
    void update_sector();

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    bool sight(const fb::model::point16_t& position) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     *
     * @return     { description_of_the_return_value }
     */
    bool sight(const fb::game::object& object) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     * @param[in]  type       The type
     *
     * @return     { description_of_the_return_value }
     */
    object* side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     * @param[in]  type       The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> sides(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    object* forward(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> forwards(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool visible() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void visible(bool value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  right  The right
     *
     * @return     { description_of_the_return_value }
     */
    double distance(const object& right) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  right  The right
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t distance_sqrt(const object& right) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  conditions  The conditions
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool condition(const std::vector<fb::model::dsl>& conditions) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool available() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  destroy_type  The destroy type
     */
    virtual void hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      { function_description }
     *
     * @param      to            { parameter_description }
     * @param[in]  destroy_type  The destroy type
     */
    virtual void hide(fb::game::object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> showings(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  source    The source
     * @param[in]  position  The position
     * @param[in]  type      The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> showings(const std::vector<object*>& source,
                                  const fb::model::point16_t& position,
                                  OBJECT_TYPE                 type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> showns(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  source    The source
     * @param[in]  position  The position
     * @param[in]  type      The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> showns(const std::vector<object*>& source,
                                const fb::model::point16_t& position,
                                OBJECT_TYPE                 type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread() const override;

    /**
     * @brief      { function_description }
     */
    void assert_thread() const override;

    /**
     * @brief      { function_description }
     */
    virtual void update_id();

    /**
     * @brief      { function_description }
     *
     * @param[in]  map   The map
     */
    virtual void update_map(const fb::game::map& map);

    /**
     * @brief      { function_description }
     *
     * @param[in]  bgm     The bgm
     * @param[in]  volume  The volume
     */
    virtual void update_bgm(uint16_t bgm, uint8_t volume);

    /**
     * @brief      { function_description }
     */
    virtual void update_position();

    /**
     * @brief      { function_description }
     *
     * @param[in]  sound  The sound
     */
    void sound(SOUND sound);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void effect(uint8_t value);

public:
    /**
     * @brief      Equality operator.
     *
     * @param[in]  right  The right
     *
     * @return     The result of the equality
     */
    bool operator== (const object& right) const;

    /**
     * @brief      Inequality operator.
     *
     * @param[in]  right  The right
     *
     * @return     The result of the inequality
     */
    bool operator!= (const object& right) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_id(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_eq(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_tostring(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_sound(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_position(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_direction(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_chat(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_message(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buff(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_unbuff(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_isbuff(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_effect(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_map(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mkitem(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_showings(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_showns(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_front(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_is(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_thread(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_ptr(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_near(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct object::listener
{
    /**
     * @brief      Called on chat.
     *
     * @param      me         { parameter_description }
     * @param[in]  message    The message
     * @param[in]  chat_type  The chat type
     */
    virtual void on_chat(fb::game::object& me, const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;

    /**
     * @brief      Called on direction.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_direction(fb::game::object& me) = 0;

    /**
     * @brief      Called on update external.
     *
     * @param      me     { parameter_description }
     * @param[in]  light  The light
     */
    virtual void on_update_external(fb::game::object& me, bool light) = 0;

    /**
     * @brief      Called on update external.
     *
     * @param      me     { parameter_description }
     * @param      you    You
     * @param[in]  light  The light
     */
    virtual void on_update_external(fb::game::object& me, fb::game::object& you, bool light) = 0;

    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param[in]  destroy_type  The destroy type
     */
    virtual void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) = 0;

    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param      you           You
     * @param[in]  destroy_type  The destroy type
     */
    virtual void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type) = 0;

    /**
     * @brief      Called on move.
     *
     * @param      me      { parameter_description }
     * @param[in]  before  The before
     */
    virtual void on_move(fb::game::object& me, const fb::model::point16_t& before) = 0;

    /**
     * @brief      Called on unbuff.
     *
     * @param      me    { parameter_description }
     * @param      buff  The buffer
     */
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff) = 0;

    /**
     * @brief      Called when map changed.
     *
     * @param      me      { parameter_description }
     * @param      before  The before
     * @param      after   The after
     */
    virtual void on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) = 0;

    /**
     * @brief      Called on create.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_create(fb::game::object& me) = 0;

    /**
     * @brief      Called on destroy.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_destroy(fb::game::object& me) = 0;

    /**
     * @brief      Called on sound.
     *
     * @param      ch     { parameter_description }
     * @param[in]  sound  The sound
     */
    virtual void on_sound(fb::game::object& ch, SOUND sound) = 0;

    /**
     * @brief      Called on effect.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     */
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
