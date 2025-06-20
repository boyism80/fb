#ifndef FB_GAME_AI_COUNTER_H_
#define FB_GAME_AI_COUNTER_H_

#include <fb/game/ai.h>

namespace fb::game {
/**
 * @brief   Counter AI implementation for defensive mobs
 *
 *          This AI type focuses on counter-attacking when the mob is attacked.
 *          The mob will:
 *          - Follow owner if one exists (from base class)
 *          - Otherwise stay in place until attacked
 *          - When attacked, move towards and attack the target
 *          - Return to random movement when target is lost
 */
class counter_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    /**
     * @brief   Executes the counter AI behavior
     *
     *          First tries to follow owner (from base class).
     *          If no owner or couldn't follow:
     *          - If no target, move randomly
     *          - If has target, move towards and attack them
     *
     * @param[in,out] mob_obj The mob to execute AI behavior for
     * @param[in]     now     Current timestamp for timing-based decisions
     * @return     true if owner following or random movement was executed
     */
    bool execute(mob& mob_obj, const datetime& now) override final;

    /**
     * @brief   Gets the AI type identifier
     *
     * @return  Always returns MOB_ATTACK_TYPE::COUNTER
     */
    MOB_ATTACK_TYPE get_type() const override final;
};
} // namespace fb::game

#endif // !FB_GAME_AI_COUNTER_H_