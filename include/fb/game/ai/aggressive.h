#ifndef FB_GAME_AI_AGGRESSIVE_H_
#define FB_GAME_AI_AGGRESSIVE_H_

#include <fb/game/ai.h>

namespace fb::game {

class aggressive_ai : public ai
{
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

#endif // !FB_GAME_AI_AGGRESSIVE_H_
