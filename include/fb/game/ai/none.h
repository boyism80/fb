#ifndef __NONE_AI_H__
#define __NONE_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>

namespace fb::game {

class none_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    // clang-format off
    bool            execute(mob& mob_obj, const datetime& now) override;
    MOB_ATTACK_TYPE get_type() const override;
    // clang-format on
};
} // namespace fb::game

#endif // !__NONE_AI_H__
