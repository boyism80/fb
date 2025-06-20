#ifndef __RUN_AWAY_AI_H__
#define __RUN_AWAY_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

namespace fb::game {
/**
 * @brief   Run Away type mob AI implementation
 *
 *          This AI behaves like containment AI but runs away when HP is low.
 *          Mobs with this AI will:
 *          - Follow owner if one exists (from base class)
 *          - Otherwise act like containment AI while HP > 20%
 *          - Run away from targets when HP <= 20%
 *          - Remember and prioritize recent aggressors
 */
class run_away_ai : public ai
{
public:
    static constexpr auto LOW_HP_THRESHOLD = 0.2f; ///< HP ratio threshold to trigger running away

private:
    using super    = ai;
    using datetime = fb::model::datetime;

private:
    datetime _target_lock_time;

public:
    /**
     * @brief   Executes the run away AI behavior
     *
     *          First tries to follow owner (from base class).
     *          If no owner or couldn't follow:
     *          - If HP > 20%: behaves like containment AI
     *          - If HP <= 20%: runs away from nearest target
     *
     * @param[in,out]  mob_obj  The mob to execute AI behavior for
     * @param[in]      now      Current game time for action timing
     * @return     true if owner following was executed, false if AI executed its own behavior
     */
    bool execute(mob& mob_obj, const datetime& now) override;

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  Always returns MOB_ATTACK_TYPE::RUN_AWAY
     */
    MOB_ATTACK_TYPE get_type() const override;
};
} // namespace fb::game

#endif // !__RUN_AWAY_AI_H__