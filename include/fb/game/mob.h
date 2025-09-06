#ifndef __MOB_H__
#define __MOB_H__

#include <fb/game/life.h>
#include <fb/game/item.h>
#include <async/task.h>

using namespace std::chrono_literals;

namespace fb::game {

class character;
class ai;

class rezen
{
private:
    fb::game::server&                  _server;
    uint16_t                           _count = 0;
    std::optional<fb::model::datetime> _respawn_time;

public:
    const fb::model::mob_spawn& model;

public:
    rezen(fb::game::server& server, const fb::model::mob_spawn& model);
    ~rezen() = default;

    /**
     * @brief      Decreases the active mob count for this spawn point.
     *
     *             Called when a mob from this spawn point is killed or removed,
     *             allowing for potential respawn based on the spawn configuration.
     */
    void decrease();

    /**
     * @brief      Spawns mobs at this spawn point if conditions are met.
     *
     *             Checks spawn conditions (count limits, timing, etc.) and spawns
     *             new mobs if appropriate. This method is called periodically by
     *             the specified thread.
     *
     * @param[in]  thread_id  The thread identifier responsible for this spawn point.
     *
     * @return     Async task that completes when the spawn operation is finished.
     */
    [[nodiscard]] async::task<void> spawn(std::thread::id thread_id);

    /**
     * @brief      Forces a mob spawn regardless of normal spawn conditions.
     *
     *             This method bypasses the normal spawn restrictions and immediately
     *             spawns mobs at this spawn point. Useful for special events,
     *             GM commands, or scripted scenarios that need to override normal
     *             spawn logic.
     *
     * @param[in]  thread_id  The thread identifier responsible for this spawn point.
     */
    void force_spawn(std::thread::id thread_id);
};

class mob : public life
{
public:
    using model_type = fb::model::mob;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;

public:
    struct initial_params : fb::game::life::initial_params
    {
    public:
        const bool             alive = false;   ///< Whether the mob should spawn alive
        fb::game::rezen* const rezen = nullptr; ///< The respawn manager for this mob
        character*             owner = nullptr; ///< The character that owns this mob (for summons)
    };

private:
    fb::model::datetime                          _action_time;
    std::weak_ptr<life>                          _target;
    std::weak_ptr<life>                          _oblivion;
    rezen*                                       _rezen         = nullptr;
    lua::context*                                _attack_thread = nullptr;
    uint32_t                                     _buff_hp       = 0;
    uint32_t                                     _buff_mp       = 0;
    uint8_t                                      _buff_str      = 0;
    uint8_t                                      _buff_dex      = 0;
    uint8_t                                      _buff_int      = 0;
    int8_t                                       _buff_phydef   = 0;
    int8_t                                       _buff_magdef   = 0;
    uint8_t                                      _buff_dam      = 0;
    uint8_t                                      _buff_hit      = 0;
    std::vector<std::shared_ptr<fb::game::item>> _items;
    bool                                         _hidden = false;
    std::unique_ptr<ai>                          _ai_strategy;

public:
    const std::weak_ptr<character> owner;

public:
    mob::listener_t& listener;

public:
    mob(fb::game::server& server, const fb::model::mob& model, const initial_params& params);
    mob(const mob& right);
    ~mob();

private:
    /**
     * @brief      Finds a suitable target for AI behavior.
     *
     *             Searches nearby areas for potential targets based on the mob's
     *             AI configuration, aggression settings, and targeting preferences.
     *
     * @return     Pointer to the selected target, or nullptr if no target found.
     */
    std::weak_ptr<fb::game::life> find_target();

    /**
     * @brief      Executes the mob's Lua AI script asynchronously.
     *
     *             Calls the mob's custom AI script if available, allowing for
     *             complex scripted behaviors and decision-making processes.
     *
     * @return     Async task that completes when script execution finishes.
     */
    [[nodiscard]] async::task<bool> call_script();

    /**
     * @brief      Executes the mob's AI behavior for the current time step.
     *
     *             Processes AI logic including target acquisition, movement,
     *             combat decisions, and state transitions based on current conditions.
     *
     * @param[in]  now   The current game time for AI processing.
     */
    void AI(const fb::model::datetime& now);

public:
    /**
     * @brief      Checks if the mob is adjacent to the specified target.
     *
     *             Determines if the mob is close enough to the target for melee
     *             attacks and calculates the direction to face the target.
     *
     * @param[in]  target  The target to check proximity to.
     * @param      out     Output parameter for the direction to the target.
     *
     * @return     True if the mob is adjacent to the target, false otherwise.
     */
    bool near_target(const std::shared_ptr<fb::game::life>& target, DIRECTION& out) const;

    /**
     * @brief      Attempts to move the mob one step toward the specified position.
     *
     *             Performs pathfinding and collision detection to move the mob
     *             closer to the target position, handling obstacles and boundaries.
     *
     * @param[in]  position  The target position to move toward.
     *
     * @return     True if the movement was successful, false if blocked.
     */
    bool move_step(const fb::model::point16_t& position);

    /**
     * @brief      Gets the mob's base hit points from the model.
     *
     *             Returns the base HP value defined in the mob's model data,
     *             without any temporary buffs or modifications applied.
     *
     * @return     The base hit points value from the mob model.
     */
    uint32_t base_hp() const override final;

    /**
     * @brief      Gets the current HP buff modifier.
     *
     *             Returns the temporary HP bonus or penalty currently applied
     *             to this mob through spells, items, or other effects.
     *
     * @return     The current HP buff value (can be positive or negative).
     */
    uint32_t buff_hp() const override final;

    /**
     * @brief      Sets the HP buff modifier.
     *
     *             Applies a temporary HP bonus or penalty to the mob, typically
     *             from spells, items, or other temporary effects.
     *
     * @param[in]  value  The HP buff value to apply.
     */
    void buff_hp(uint32_t value) override final;

    /**
     * @brief      Gets the mob's base mana points from the model.
     *
     *             Returns the base MP value defined in the mob's model data,
     *             without any temporary buffs or modifications applied.
     *
     * @return     The base mana points value from the mob model.
     */
    uint32_t base_mp() const override final;

    /**
     * @brief      Gets the current MP buff modifier.
     *
     *             Returns the temporary MP bonus or penalty currently applied
     *             to this mob through spells, items, or other effects.
     *
     * @return     The current MP buff value (can be positive or negative).
     */
    uint32_t buff_mp() const override final;

    /**
     * @brief      Sets the MP buff modifier.
     *
     *             Applies a temporary MP bonus or penalty to the mob, typically
     *             from spells, items, or other temporary effects.
     *
     * @param[in]  value  The MP buff value to apply.
     */
    void buff_mp(uint32_t value) override final;

    /**
     * @brief      Gets the mob's base strength from the model.
     *
     * @return     The base strength value from the mob model
     */
    uint8_t base_str() const override final;

    /**
     * @brief      Gets the current strength buff modifier.
     *
     * @return     The current strength buff value
     */
    uint8_t buff_str() const override final;

    /**
     * @brief      Sets the strength buff modifier.
     *
     * @param[in]  value  The strength buff value to apply
     */
    void buff_str(uint8_t value) override final;

    /**
     * @brief      Gets the mob's base dexterity from the model.
     *
     * @return     The base dexterity value from the mob model
     */
    uint8_t base_dex() const override final;

    /**
     * @brief      Gets the current dexterity buff modifier.
     *
     * @return     The current dexterity buff value
     */
    uint8_t buff_dex() const override final;

    /**
     * @brief      Sets the dexterity buff modifier.
     *
     * @param[in]  value  The dexterity buff value to apply
     */
    void buff_dex(uint8_t value) override final;

    /**
     * @brief      Gets the mob's base intelligence from the model.
     *
     * @return     The base intelligence value from the mob model
     */
    uint8_t base_int() const override final;

    /**
     * @brief      Gets the current intelligence buff modifier.
     *
     * @return     The current intelligence buff value
     */
    uint8_t buff_int() const override final;

    /**
     * @brief      Sets the intelligence buff modifier.
     *
     * @param[in]  value  The intelligence buff value to apply
     */
    void buff_int(uint8_t value) override final;

    /**
     * @brief      Gets the mob's base physical defense from the model.
     *
     * @return     The base physical defense value from the mob model
     */
    int8_t base_phydef() const override final;

    /**
     * @brief      Gets the current physical defense buff modifier.
     *
     * @return     The current physical defense buff value
     */
    int8_t buff_phydef() const override final;

    /**
     * @brief      Sets the physical defense buff modifier.
     *
     * @param[in]  value  The physical defense buff value to apply
     */
    void buff_phydef(int8_t value) override final;

    /**
     * @brief      Gets the mob's base magical defense from the model.
     *
     * @return     The base magical defense value from the mob model
     */
    int8_t base_magdef() const override final;

    /**
     * @brief      Gets the current magical defense buff modifier.
     *
     * @return     The current magical defense buff value
     */
    int8_t buff_magdef() const override final;

    /**
     * @brief      Sets the magical defense buff modifier.
     *
     * @param[in]  value  The magical defense buff value to apply
     */
    void buff_magdef(int8_t value) override final;

    /**
     * @brief      Gets the mob's base damage from the model.
     *
     * @return     The base damage value from the mob model
     */
    uint8_t base_dam() const override final;

    /**
     * @brief      Gets the current damage buff modifier.
     *
     * @return     The current damage buff value
     */
    uint8_t buff_dam() const override final;

    /**
     * @brief      Sets the damage buff modifier.
     *
     * @param[in]  value  The damage buff value to apply
     */
    void buff_dam(uint8_t value) override final;

    /**
     * @brief      Gets the mob's base hit rate from the model.
     *
     * @return     The base hit rate value from the mob model
     */
    uint8_t base_hit() const override final;

    /**
     * @brief      Gets the current hit rate buff modifier.
     *
     * @return     The current hit rate buff value
     */
    uint8_t buff_hit() const override final;

    /**
     * @brief      Sets the hit rate buff modifier.
     *
     * @param[in]  value  The hit rate buff value to apply
     */
    void buff_hit(uint8_t value) override final;

public:
    /**
     * @brief      Executes the mob's action for the current time step.
     *
     *             Processes the mob's AI behavior, movement, combat actions, and
     *             other time-based activities for the specified game time.
     *
     * @param[in]  now   The current game time for action processing.
     *
     * @return     Async task that completes when the action is finished.
     */
    [[nodiscard]] async::task<void> action(fb::model::datetime now);

    /**
     * @brief      Gets the time of the mob's last action.
     *
     *             Returns the timestamp when this mob last performed an action,
     *             used for timing calculations and AI scheduling.
     *
     * @return     Reference to the last action timestamp.
     */
    const fb::model::datetime& action_time() const;

    /**
     * @brief      Sets the time of the mob's last action.
     *
     *             Updates the timestamp when this mob last performed an action,
     *             used for timing calculations and AI scheduling.
     *
     * @param[in]  dt    The new action timestamp to set.
     */
    void action_time(const fb::model::datetime& dt);

    /**
     * @brief   Gets the current target of this mob
     *
     *          Validates the target before returning:
     *          - Must not be null
     *          - Must be alive in server
     *          - Must be alive in game
     *          - Must be within sight
     *          - Must not be hidden from this mob
     *
     * @return  Valid target pointer, nullptr if target is invalid or non-existent
     */
    std::shared_ptr<fb::game::life> target() const;

    /**
     * @brief   Sets the current target for this mob
     *
     * @param[in]  value  New target to set, can be nullptr to clear target
     */
    void target(std::shared_ptr<fb::game::life> value);

    /**
     * @brief      Gets the mob's oblivion target (last attacker).
     *
     * @return     Pointer to the life entity that last attacked this mob
     */
    std::shared_ptr<fb::game::life> oblivion() const;

    /**
     * @brief      Sets the mob's oblivion target (last attacker).
     *
     * @param      value  The life entity that attacked this mob
     */
    void oblivion(std::shared_ptr<fb::game::life> value);

    /**
     * @brief      Updates and returns the mob's current target.
     *
     * @return     Pointer to the updated target, or nullptr if no valid target
     */
    std::shared_ptr<fb::game::life> update_target();

    /**
     * @brief      Checks if the mob is available for actions.
     *
     * @return     True if the mob can perform actions, false otherwise
     */
    virtual bool available() const;

    /**
     * @brief      Calculates auto-attack damage based on target size.
     *
     * @param[in]  size  The target mob size (affects damage calculation)
     *
     * @return     The calculated damage value for auto-attack
     */
    uint32_t auto_attack_damage(MOB_SIZE size) const override final;

    /**
     * @brief      Applies damage to the mob.
     *
     *             Processes incoming damage to the mob, handling defense calculations,
     *             critical hits, and death conditions. Updates HP and triggers
     *             appropriate responses based on the damage source.
     *
     * @param[in]  value     The base damage value to apply.
     * @param      from      The object causing the damage (optional).
     * @param[in]  critical  Whether this is a critical hit.
     *
     * @return     The actual damage dealt after defense calculations.
     */
    uint32_t damage(uint32_t value, std::shared_ptr<fb::game::object> from = nullptr, bool critical = false) override final;

    /**
     * @brief      Kills the mob and handles death processing.
     *
     *             Processes the mob's death, including item drops, experience
     *             distribution, respawn management, and cleanup operations.
     *
     * @param      from          The object that caused the death (optional).
     * @param[in]  destroy_type  The type of destruction (normal, admin, etc.).
     */
    void kill(std::shared_ptr<fb::game::object> from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      Drops items when the mob dies.
     *
     *             Handles the item dropping logic when the mob is killed,
     *             including loot table processing and item placement.
     *
     * @return     An async task that completes when the items are dropped
     */
    async::task<void> drop_items();

    /**
     * @brief      Asserts that the current thread is the correct thread for this mob.
     */
    void assert_thread() const override final;

    /**
     * @brief      Moves the mob in the specified direction.
     *
     * @param[in]  direction  The direction to move
     *
     * @return     True if the move was successful, false otherwise
     */
    bool move(DIRECTION direction) override final;

    /**
     * @brief      Gets the mob's item inventory.
     *
     * @return     Reference to the vector of items carried by this mob
     */
    const std::vector<std::shared_ptr<fb::game::item>>& items() const;

    /**
     * @brief      Adds an item to the mob's inventory.
     *
     * @param      item     The item to add to the mob's inventory
     *
     * @return     True if the item was successfully added, false otherwise
     */
    bool push_item(std::shared_ptr<fb::game::item> item);

    /**
     * @brief      Checks if this mob is hidden from the target.
     *
     * @param[in]  target  The target object to check visibility against
     *
     * @return     True if this mob is hidden from the target, false otherwise
     */
    bool hidden(const fb::game::object& target) const override final;

    /**
     * @brief      Sets the mob's hidden state.
     *
     * @param[in]  enabled  True to hide the mob, false to make it visible
     */
    void hidden(bool enabled);
};

struct mob::listener_t : public virtual fb::game::life::listener_t
{ };

} // namespace fb::game

#endif // !__MOB_H__
