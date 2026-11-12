#include <fb/game/ai/retaliate.h>

#include <fb/game/mob.h>
#include <random.h>

#include <cstdlib>

using namespace fb::game;

async::task<bool> retaliate_ai::execute(mob& mob_obj, const datetime& now)
{
    if (co_await super::execute(mob_obj, now))
        co_return true;

    auto target = super::select_target(mob_obj, now);
    if (target == nullptr)
    {
        co_await mob_obj.move(DIRECTION(random(0, 3)));
        co_return true;
    }

    co_await super::chase(mob_obj, target, now);
    co_return true;
}

MOB_ATTACK_TYPE retaliate_ai::get_type() const
{
    return MOB_ATTACK_TYPE::RETALIATE;
}
