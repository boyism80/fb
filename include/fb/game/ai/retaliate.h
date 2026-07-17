#ifndef FB_GAME_AI_RETALIATE_H_
#define FB_GAME_AI_RETALIATE_H_

#include <fb/game/ai.h>

namespace fb::game {

class retaliate_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    // clang-format off
    bool            execute(mob& mob_obj, const datetime& now) override final;
    MOB_ATTACK_TYPE get_type() const override final;
    // clang-format on
};

} // namespace fb::game

#endif // !FB_GAME_AI_RETALIATE_H_
