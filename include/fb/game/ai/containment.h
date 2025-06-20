#ifndef __CONTAINMENT_AI_H__
#define __CONTAINMENT_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

namespace fb::game {
/**
 * @brief   Containment type mob AI implementation
 *
 *          This AI actively seeks and attacks targets.
 *          Mobs with this AI will:
 *          - Follow owner if one exists
 *          - Otherwise actively seek targets in sight range
 *          - Move towards and attack targets
 *          - Remember and prioritize recent aggressors
 */
class containment_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

private:
    datetime _target_lock_time;

public:
    /**
     * @brief   Executes the containment AI behavior
     *
     *          First tries to follow owner (from base class).
     *          If no owner or couldn't follow:
     *          1. Find target in sight range
     *          2. Move towards target if out of range
     *          3. Attack target when in range
     *
     * @param[in,out]  mob_obj  The mob to execute AI behavior for
     * @param[in]      now      Current game time for action timing
     * @return     true if owner following was executed, false if AI executed its own behavior
     */
    bool execute(mob& mob_obj, const datetime& now) override;

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  Always returns MOB_ATTACK_TYPE::CONTAINMENT
     */
    MOB_ATTACK_TYPE get_type() const override;
};
} // namespace fb::game

#endif // !__CONTAINMENT_AI_H__