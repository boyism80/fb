#ifndef __RUN_AWAY_AI_H__
#define __RUN_AWAY_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

namespace fb::game {

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
    // clang-format off
    bool            execute(mob& mob_obj, const datetime& now) override;
    MOB_ATTACK_TYPE get_type() const override;
    // clang-format on
};
} // namespace fb::game

#endif // !__RUN_AWAY_AI_H__
