#ifndef FB_GAME_AI_COUNTER_H_
#define FB_GAME_AI_COUNTER_H_

#include <fb/game/ai.h>

namespace fb::game {

class counter_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

public:
    async::task<bool> execute(mob& mob_obj, const datetime& now) override final;
    MOB_ATTACK_TYPE   get_type() const override final;
};
} // namespace fb::game

#endif // !FB_GAME_AI_COUNTER_H_