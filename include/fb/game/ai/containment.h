#ifndef __CONTAINMENT_AI_H__
#define __CONTAINMENT_AI_H__

#include <fb/game/ai.h>
#include <fb/game/mob.h>
#include <fb/game/map.h>

namespace fb::game {

class containment_ai : public ai
{
private:
    using super    = ai;
    using datetime = fb::model::datetime;

private:
    datetime _target_lock_time;

public:
    async::task<bool> execute(mob& mob_obj, const datetime& now) override;
    MOB_ATTACK_TYPE   get_type() const override;
};
} // namespace fb::game

#endif // !__CONTAINMENT_AI_H__