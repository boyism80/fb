#ifndef __AI_H__
#define __AI_H__

#include <fb/model/model.h>
#include <unordered_map>

using namespace std::chrono_literals;
using namespace fb::model::enum_value;

namespace fb::game {

// Forward declarations
class mob;
class life;
class character;

/**
 * @brief   Base class for mob AI implementations
 *
 *          Provides common functionality for different AI types:
 *          - Owner following behavior (common to all AIs)
 *          - Damage memory tracking
 *          - Target selection and validation
 *          - Common utility functions
 */
class ai
{
public:
    static constexpr auto DAMAGE_MEMORY_DURATION = std::chrono::seconds(10); ///< How long to remember recent damage
    static constexpr auto TARGET_LOCK_DURATION   = std::chrono::seconds(5);  ///< How long to stick to current target

private:
    using datetime      = fb::model::datetime;
    using damage_record = std::pair<uint32_t, datetime>; ///< Pair of (damage, time)
    using damage_map    = std::unordered_map<std::shared_ptr<life>, damage_record>;

private:
    damage_map _recent_damage;    ///< Tracks recent damage from each attacker
    datetime   _target_lock_time; ///< When the current target was locked in

public:
    /**
     * @brief   Virtual destructor for proper cleanup
     */
    virtual ~ai() = default;

    /**
     * @brief   Executes the AI behavior
     *
     *          First checks and handles owner following behavior.
     *          If owner following is not applicable, executes type-specific behavior.
     *
     * @param[in,out]  mob_obj  The mob to execute AI behavior for
     * @param[in]      now      Current game time for action timing
     * @return     true if owner following was executed, false if type-specific behavior should be executed
     */
    virtual bool execute(mob& mob_obj, const datetime& now);

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  The MOB_ATTACK_TYPE for this AI
     */
    virtual MOB_ATTACK_TYPE get_type() const = 0;

    /**
     * @brief   Handles when the mob takes damage
     *
     * @param[in]  mob_obj    The mob that took damage
     * @param[in]  attacker   The life object that dealt the damage
     * @param[in]  now        Current game time
     */
    virtual void on_damage(mob& mob_obj, std::shared_ptr<life> attacker, const datetime& now);

    /**
     * @brief   Creates appropriate AI strategy for given attack type
     *
     * @param[in]  attack_type  The type of attack behavior to create strategy for
     * @return     Unique pointer to the created strategy, nullptr if type is invalid
     */
    static std::unique_ptr<ai> create(MOB_ATTACK_TYPE attack_type);

protected:
    /**
     * @brief   Finds a target for the mob within its sight range
     *
     *          Searches for potential targets with the following criteria:
     *          - Must be a CHARACTER type object
     *          - Must be alive
     *          - Must not be in oblivion state
     *          - Must be within sight range
     *          - Prioritizes recent attackers
     *
     * @param[in]  mob_obj  The mob searching for a target
     * @param[in]  now      Current game time
     * @return     Pointer to the best target, nullptr if none found
     */
    std::shared_ptr<life> find_target_in_sight(mob& mob_obj, const datetime& now);

    /**
     * @brief   Finds a target for the mob within its attack range
     *
     *          Similar to find_target_in_sight but requires targets to be
     *          within attack range. Used by stationary AIs.
     *
     * @param[in]  mob_obj  The mob searching for a target
     * @param[in]  now      Current game time
     * @return     Pointer to the best target, nullptr if none found
     */
    std::shared_ptr<life> find_target_in_range(mob& mob_obj, const datetime& now);

    /**
     * @brief   Cleans up expired damage records
     *
     * @param[in]  now  Current game time
     */
    void cleanup_expired_damage(const datetime& now);

    /**
     * @brief   Checks if the attacker should be ignored based on rules
     *
     * @param[in]  mob_obj   The mob being attacked
     * @param[in]  attacker  The attacking character
     * @return     true if the attacker should be ignored
     */
    bool should_ignore_attacker(const mob& mob_obj, std::shared_ptr<life> attacker) const;

    /**
     * @brief   Checks if the current target should be maintained
     *
     * @param[in]  mob_obj  The mob with the target
     * @param[in]  now      Current game time
     * @return     true if should keep current target
     */
    bool should_maintain_target(const mob& mob_obj, const datetime& now) const;

    /**
     * @brief   Updates damage tracking for an attacker
     *
     * @param[in]  attacker  The life object that dealt damage
     * @param[in]  now       Current game time
     */
    void record_damage(std::shared_ptr<life> attacker, const datetime& now);

    /**
     * @brief   Makes the mob run away from its target
     *
     * @param[in,out]  mob_obj  The mob that should run away
     * @param[in]      target   The target to run from
     */
    void run_from_target(mob& mob_obj, std::shared_ptr<life> target);
};

} // namespace fb::game

#endif // !__AI_H__