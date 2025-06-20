#ifndef __NO_MOVE_AI_H__
#define __NO_MOVE_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

namespace fb::game {
/**
 * @brief   No Move type mob AI implementation
 *
 *          This AI stays in place and only attacks when targets are in range.
 *          Mobs with this AI will:
 *          - Follow owner if one exists (from base class)
 *          - Otherwise never move from their position
 *          - Attack targets that come within range
 *          - Remember and prioritize recent aggressors
 */
class no_move_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

private:
    datetime _target_lock_time;

public:
    /**
     * @brief   Executes the no move AI behavior
     *
     *          First tries to follow owner (from base class).
     *          If no owner or couldn't follow:
     *          - Stay in current position
     *          - Attack targets that come within range
     *          - Never move to chase targets
     *
     * @param[in,out]  mob_obj  The mob to execute AI behavior for
     * @param[in]      now      Current game time for action timing
     * @return     true if owner following was executed, false if AI executed its own behavior
     */
    bool execute(mob& mob_obj, const datetime& now) override;

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  Always returns MOB_ATTACK_TYPE::NO_MOVE
     */
    MOB_ATTACK_TYPE get_type() const override;
};
} // namespace fb::game

#endif // !__NO_MOVE_AI_H__