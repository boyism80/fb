#ifndef __NONE_AI_H__
#define __NONE_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>

namespace fb::game {
/**
 * @brief   None type mob AI implementation
 *
 *          This AI runs away from all targets.
 *          Mobs with this AI will:
 *          - Follow owner if one exists (from base class)
 *          - Otherwise run away from any target
 *          - Never attack
 *          - Move randomly when no target exists
 */
class none_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    /**
     * @brief   Executes the none AI behavior
     *
     *          First tries to follow owner (from base class).
     *          If no owner or couldn't follow:
     *          - If has target, run away from it
     *          - If no target, move randomly
     *
     * @param[in,out]  mob_obj  The mob to execute AI behavior for
     * @param[in]      now      Current game time for action timing
     * @return     true if owner following was executed, false if AI executed its own behavior
     */
    bool execute(mob& mob_obj, const datetime& now) override;

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  Always returns MOB_ATTACK_TYPE::NONE
     */
    MOB_ATTACK_TYPE get_type() const override;
};
} // namespace fb::game

#endif // !__NONE_AI_H__