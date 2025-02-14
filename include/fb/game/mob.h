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
    /**
     * @brief      { struct_description }
     */
    struct listener;

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
    listener*           _listener = nullptr;
    fb::model::datetime _action_time;
    fb::game::rezen*    _rezen         = nullptr;
    fb::game::life*     _target        = nullptr;
    fb::game::life*     _oblivion      = nullptr;
    lua::context*       _attack_thread = nullptr;

public:
    fb::game::character* owner = nullptr;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  config   The configuration
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
     * @param      out   The out
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
    bool action();

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

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_target(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_oblivion(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct mob::listener : public virtual fb::game::life::listener
{ };

} // namespace fb::game

#endif // !__MOB_H__
