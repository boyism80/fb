#ifndef __MOB_H__
#define __MOB_H__

#include <fb/game/life.h>
#include <fb/game/item.h>

using namespace std::chrono_literals;

namespace fb::game {

/**
 * @brief      This class describes a character.
 */
class character;

/**
 * @brief      This class describes a rezen.
 */
class rezen
{
private:
    fb::game::context&                 _context;
    const fb::model::mob_spawn&        _model;
    uint16_t                           _count = 0;
    std::optional<fb::model::datetime> _respawn_time;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    rezen(fb::game::context& context, const fb::model::mob_spawn& model);

    /**
     * @brief      Destroys the object.
     */
    ~rezen() = default;

    /**
     * @brief      { function_description }
     */
    void decrease();

    /**
     * @brief      { function_description }
     *
     * @param[in]  thread_id  The thread identifier
     */
    void spawn(std::thread::id thread_id);
};

/**
 * @brief      This class describes a mob.
 */
class mob : public life
{
public:
    using model_type = fb::model::mob;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;
    struct builtin;

public:
    /**
     * @brief      { struct_description }
     */
    struct initial_params : fb::game::life::initial_params
    {
    public:
        const bool             alive = false;
        fb::game::rezen* const rezen = nullptr;
        fb::game::character*   owner = nullptr;
    };

private:
    fb::model::datetime _action_time;
    rezen*              _rezen         = nullptr;
    life*               _target        = nullptr;
    life*               _oblivion      = nullptr;
    lua::context*       _attack_thread = nullptr;
    uint32_t            _buff_hp       = 0;
    uint32_t            _buff_mp       = 0;
    uint8_t             _buff_str      = 0;
    uint8_t             _buff_dex      = 0;
    uint8_t             _buff_int      = 0;
    int8_t              _buff_phydef   = 0;
    int8_t              _buff_magdef   = 0;
    uint8_t             _buff_dam      = 0;
    uint8_t             _buff_hit      = 0;
    std::vector<item*>  _items;
    bool                _hidden = false;
    character*          _owner  = nullptr;

public:
    character* const owner = nullptr;

public:
    mob::listener_t& listener;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  params   The parameters
     * @param[in]  config  The configuration
     */
    mob(fb::game::context& context, const fb::model::mob& model, const initial_params& params);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    mob(const mob& right);

    /**
     * @brief      Destroys the object.
     */
    ~mob();

private:
    /**
     * @brief      Finds a target.
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::life* find_target();

    /**
     * @brief      { function_description }
     *
     * @param[in]  target  The target
     * @param      out     The out
     *
     * @return     { description_of_the_return_value }
     */
    bool near_target(const fb::game::life& target, DIRECTION& out) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    bool move_step(const fb::model::point16_t& position);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t base_hp() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t buff_hp() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_hp(uint32_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t base_mp() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t buff_mp() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_mp(uint32_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t base_str() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_str() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_str(uint8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t base_dex() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_dex() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_dex(uint8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t base_int() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_int() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_int(uint8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    int8_t base_phydef() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    int8_t buff_phydef() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_phydef(int8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    int8_t base_magdef() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    int8_t buff_magdef() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_magdef(int8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t base_dam() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_dam() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_dam(uint8_t value) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t base_hit() const override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_hit() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_hit(uint8_t value) override final;

public:
    /**
     * @brief      { function_description }
     */
    void action();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::datetime& action_time() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  dt    { parameter_description }
     */
    void action_time(const fb::model::datetime& dt);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::life* target() const;

    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     */
    void target(fb::game::life* value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::life* oblivion() const;

    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     */
    void oblivion(fb::game::life* value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::life* repair_target();

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     */
    void AI(const fb::model::datetime& now);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool available() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  size  The size
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t auto_attack_damage(MOB_SIZE size) const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value     The value
     * @param      from      The from
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t damage(uint32_t value, fb::game::object* from = nullptr, bool critical = false) override final;

    /**
     * @brief      { function_description }
     *
     * @param      from          The from
     * @param[in]  destroy_type  The destroy type
     */
    void kill(fb::game::object* from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      { function_description }
     */
    void drop_items();

    /**
     * @brief      { function_description }
     */
    void assert_thread() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     *
     * @return     { description_of_the_return_value }
     */
    bool move(DIRECTION direction) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::vector<item*>& items() const;

    /**
     * @brief      Pushes an item.
     *
     * @param      i     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool push_item(item& i);

    /**
     * @brief      { function_description }
     *
     * @param[in]  target  The target
     *
     * @return     { description_of_the_return_value }
     */
    bool hidden(const object& target) const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  enabled  Indicates if enabled
     */
    void hidden(bool enabled);
};

/**
 * @brief      { struct_description }
 */
struct mob::listener_t : public virtual fb::game::life::listener_t
{ };

/**
 * @brief      { struct_description }
 */
struct mob::builtin
{
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_target(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_oblivion(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_owner(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_items(lua_State* L);
};

} // namespace fb::game

#endif // !__MOB_H__
