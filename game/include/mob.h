#ifndef __MOB_H__
#define __MOB_H__

#include <life.h>
#include <item.h>

using namespace std::chrono_literals;

namespace fb { namespace game {

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
    fb::game::context&          _context;
    const fb::model::mob_spawn& _model;
    uint16_t                    _count = 0;
    std::optional<datetime>     _respawn_time;

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
    };

private:
    listener*        _listener = nullptr;
    datetime         _action_time;
    fb::game::rezen* _rezen         = nullptr;
    fb::game::life*  _target        = nullptr;
    lua::context*    _attack_thread = nullptr;

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
    bool near_target(DIRECTION& out) const;

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
    const datetime& action_time() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  dt    { parameter_description }
     */
    void action_time(const datetime& dt);
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
    fb::game::life* fix();
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     */
    void AI(const datetime& now);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool available() const;

protected:
    /**
     * @brief      Called on calculate damage.
     *
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t on_calculate_damage(bool critical) const final;
    /**
     * @brief      Called when damaged.
     *
     * @param      from      The from
     * @param[in]  damage    The damage
     * @param[in]  critical  The critical
     */
    void on_damaged(fb::game::object* from, uint32_t damage, bool critical) final;
    /**
     * @brief      Called on die.
     *
     * @param      from  The from
     */
    void on_die(fb::game::object* from) final;

public:
    /**
     * @brief      Called on exponent.
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t on_exp() const final;
};

/**
 * @brief      { struct_description }
 */
struct mob::listener : public virtual fb::game::life::listener
{ };

}} // namespace fb::game

#endif // !__MOB_H__
